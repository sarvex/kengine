#include "ImGuiPromptSystem.hpp"
#include "EntityManager.hpp"

#include "data/NameComponent.hpp"
#include "data/ImGuiToolComponent.hpp"
#include "functions/Execute.hpp"

#ifdef KENGINE_LUA
# include "data/LuaStateComponent.hpp"
#endif

#ifdef KENGINE_PYTHON
# include "data/PythonStateComponent.hpp"
#endif

#include "imgui.h"
#include "reflection/imgui_helper.hpp"

enum class Language {
	Lua,
	Python
};

namespace kengine::imgui_prompt {
	struct impl {
		static inline EntityManager * em;
		static inline bool * enabled;

		static inline Language selectedLanguage = Language::Lua;
		static inline int maxLines = 128;
		static inline char buff[1024];

		static inline struct History {
			struct Line {
				std::string text;
				bool separator = false;
				putils::NormalizedColor color;
			};

			std::list<Line> lines;

			template<typename S>
			void addLine(S && s, bool separator = false, const putils::NormalizedColor & color = {}) {
				lines.push_back({ FWD(s), separator, color });
			}

			template<typename S>
			void addError(S && s, bool separator = false) {
				addLine(FWD(s), separator, { 1.f, 0.f, 0.f });
			}
		} history;

		static void init(Entity & e) {
			auto & tool = e.attach<ImGuiToolComponent>();
			enabled = &tool.enabled;
			e += NameComponent{ "Prompt" };
			e += functions::Execute{ draw };
		}

		static void draw(float deltaTime) {
			if (!*enabled)
				return;

			if (ImGui::Begin("Prompt", enabled)) {
				ImGui::Columns(2);
				drawHistory();
				ImGui::NextColumn();
				if (drawPrompt()) {
					history.addLine(buff, false, putils::NormalizedColor{ 0.f }); // cyan
					eval();
					buff[0] = 0;
					while (history.lines.size() > maxLines && !history.lines.empty())
						history.lines.pop_front();
				}
				ImGui::Columns();
			}
			ImGui::End();
		}

		static inline bool shouldScrollDown = false;
		static void drawHistory() {
			int tmp = maxLines;
			if (ImGui::InputInt("Max history", &tmp, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
				maxLines = tmp;

			ImGui::BeginChild("History");
			for (const auto & line : history.lines) {
				if (line.separator)
					ImGui::Separator();

				ImGui::PushTextWrapPos();
				ImGui::TextColored({ line.color.r, line.color.g, line.color.b, line.color.a }, line.text.c_str());
				ImGui::PopTextWrapPos();
				if (ImGui::IsItemClicked(1))
					ImGui::SetClipboardText(line.text.c_str());
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Right-click to copy to clipboard");
			}

			if (shouldScrollDown) {
				ImGui::SetScrollHere();
				shouldScrollDown = false;
			}

			ImGui::EndChild();
		}

		static bool drawPrompt() {
			static bool first = true;
			if (putils::reflection::imguiEnumCombo("##Language", selectedLanguage) || first) {
				history.addLine(
					std::string(putils::magic_enum::enum_names<Language>()[(int)selectedLanguage]),
					true,
					putils::NormalizedColor{ 1.f, 1.f, 0.f }
				);
				shouldScrollDown = true;
				first = false;
			}

			const bool ret = ImGui::InputTextMultiline("##Prompt", buff, putils::lengthof(buff), { -1.f, -1.f }, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Ctrl+Enter to execute");

			if (ret) {
				shouldScrollDown = true;
				ImGui::SetKeyboardFocusHere(-1);
			}
			return ret;
		}

		static void eval() {
			switch (selectedLanguage) {
			case Language::Lua:
				evalLua();
				break;
			case Language::Python:
				evalPython();
				break;
			default:
				static_assert(putils::magic_enum::enum_count<Language>() == 2);
			}
		}

		static inline bool active = false;
		static void evalLua() {
#ifndef KENGINE_LUA
			addLineToHistory(
				"Please compile with KENGINE_LUA",
				false,
				putils::NormalizedColor{ 1.f, 0.f, 0.f }
			);
#else
			static bool first = true;

			for (const auto & [e, state] : em->getEntities<kengine::LuaStateComponent>()) {
				if (first) {
					setupOutputRedirect(*state.state);
					first = false;
				}

				active = true;
				try {
					state.state->script(buff);
				}
				catch (const std::exception & e) {
					history.addError(e.what());
				}
				active = false;
			}
#endif
		}

		static void setupOutputRedirect(sol::state & state) {
			static const luaL_Reg printlib[] = {
				{ "print", [](lua_State * L) { return addToHistoryOrPrint(L, {}); } },
				{ "error", [](lua_State * L) { return addToHistoryOrPrint(L, { 1.f, 0.f, 0.f }); } },
				{ nullptr, nullptr }
			};

			lua_getglobal(state, "_G");
			luaL_setfuncs(state, printlib, 0);
			lua_pop(state, 1);
		}

		static int addToHistoryOrPrint(lua_State * L, const putils::NormalizedColor & color) {
			std::string line;

			// Stolen from luaB_print
			const int nargs = lua_gettop(L);
			lua_getglobal(L, "tostring");
			for (int i = 1; i <= nargs; i++) {
				lua_pushvalue(L, -1);  /* function to be called */
				lua_pushvalue(L, i);   /* value to print */
				lua_call(L, 1, 1);
				const char * s = lua_tolstring(L, -1, nullptr);  /* get result */
				if (s == nullptr)
					return luaL_error(L, "'tostring' must return a string to 'print'");
				if (i > 1)
					line += '\t';
				line += s;
				lua_pop(L, 1);  /* pop result */
			}

			if (active)
				history.addLine(std::move(line), false, color);
			else
				std::cout << line << '\n';
			return 0;
		}

#ifdef KENGINE_PYTHON
		// Stolen from https://github.com/pybind/pybind11/issues/1622
		class PyStdErrOutStreamRedirect {
			py::object _stdout;
			py::object _stderr;
			py::object _stdout_buffer;
			py::object _stderr_buffer;
		public:
			PyStdErrOutStreamRedirect() {
				auto sysm = py::module_::import("sys");
				_stdout = sysm.attr("stdout");
				_stderr = sysm.attr("stderr");
				auto stringio = py::module_::import("io").attr("StringIO");
				_stdout_buffer = stringio();  // Other filelike object can be used here as well, such as objects created by pybind11
				_stderr_buffer = stringio();
				sysm.attr("stdout") = _stdout_buffer;
				sysm.attr("stderr") = _stderr_buffer;
			}
			std::string stdoutString() {
				_stdout_buffer.attr("seek")(0);
				return py::str(_stdout_buffer.attr("read")());
			}
			std::string stderrString() {
				_stderr_buffer.attr("seek")(0);
				return py::str(_stderr_buffer.attr("read")());
			}
			~PyStdErrOutStreamRedirect() {
				auto sysm = py::module_::import("sys");
				sysm.attr("stdout") = _stdout;
				sysm.attr("stderr") = _stderr;
			}
		};
#endif
#pragma endregion PythonRedirectHelper
		static void evalPython() {
#ifndef KENGINE_PYTHON
			addLineToHistory(
				"Please compile with KENGINE_PYTHON",
				false,
				putils::NormalizedColor{ 1.f, 0.f, 0.f }
			);
#else
			PyStdErrOutStreamRedirect redirect;
			try {
				py::exec(buff);
			}
			catch (const std::exception & e) {
				history.addError(e.what());
			}

			auto output = redirect.stdoutString();
			if (!output.empty())
				history.addLine(std::move(output));

			auto err = redirect.stderrString();
			if (!err.empty())
				history.addError(std::move(err));
#endif
		}
	};
}

namespace kengine {
	EntityCreator * ImGuiPromptSystem(EntityManager & em) {
		imgui_prompt::impl::em = &em;
		return [](Entity & e) {
			imgui_prompt::impl::init(e);
		};
	}
}
