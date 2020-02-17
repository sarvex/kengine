#include "ImGuiPromptSystem.hpp"
#include "EntityManager.hpp"

#include "data/ImGuiComponent.hpp"
#include "data/NameComponent.hpp"
#include "data/ImGuiToolComponent.hpp"

#ifdef KENGINE_LUA
# include "data/LuaStateComponent.hpp"
#endif

#ifdef KENGINE_PYTHON
# include "data/PythonStateComponent.hpp"
#endif

#include "imgui.h"
#include "reflection/imgui_helper.hpp"

namespace kengine {
	static EntityManager * g_em;
	static bool * g_enabled;

	// declarations
	static void draw();
	//
	EntityCreator * ImGuiPromptSystem(EntityManager & em) {
		g_em = &em;

		return [](Entity & sys) {
			auto & tool = sys.attach<ImGuiToolComponent>();
			g_enabled = &tool.enabled;
			sys += NameComponent{ "Prompt" };
			sys += ImGuiComponent( draw );
		};
	}

	enum class Language {
		Lua,
		Python
	};

	static Language g_selectedLanguage = Language::Lua;
	static int g_maxLines = 128;
	static char g_buff[1024];

	struct History {
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
	};

	static History g_history;

	// declarations
	static void history();
	static bool prompt();
	static void eval();
	//
	static void draw() {
		if (!*g_enabled)
			return;

		if (ImGui::Begin("Prompt", g_enabled)) {
			ImGui::Columns(2);
			history();
			ImGui::NextColumn();
			if (prompt()) {
				g_history.addLine(g_buff, false, putils::NormalizedColor{ 0.f }); // cyan
				eval();
				g_buff[0] = 0;
				while (g_history.lines.size() > g_maxLines && !g_history.lines.empty())
					g_history.lines.pop_front();
			}
			ImGui::Columns();
		}
		ImGui::End();
	}

	static bool g_shouldScrollDown = false;
	static void history() {
		int tmp = g_maxLines;
		if (ImGui::InputInt("Max history", &tmp, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
			g_maxLines = tmp;

		ImGui::BeginChild("History");
		for (const auto & line : g_history.lines) {
			if (line.separator)
				ImGui::Separator();

			ImGui::TextColored({ line.color.r, line.color.g, line.color.b, line.color.a }, line.text.c_str());
			if (ImGui::IsItemClicked(1))
				ImGui::SetClipboardText(line.text.c_str());
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Right-click to copy to clipboard");
		}

		if (g_shouldScrollDown) {
			ImGui::SetScrollHere();
			g_shouldScrollDown = false;
		}

		ImGui::EndChild();
	}

	static bool prompt() {
		if (putils::reflection::imguiEnumCombo("##Language", g_selectedLanguage)) {
			g_history.addLine(
				std::string(putils::magic_enum::enum_names<Language>()[(int)g_selectedLanguage]),
				true,
				putils::NormalizedColor{ 1.f, 1.f, 0.f }
			);
			g_shouldScrollDown = true;
		}

		const bool ret = ImGui::InputTextMultiline("##Prompt", g_buff, lengthof(g_buff), { -1.f, -1.f }, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Ctrl+Enter to execute");

		if (ret)
			g_shouldScrollDown = true;
		return ret;
	}

	// declarations
	static void evalLua();
	static void evalPython();
	//
	static void eval() {
		switch (g_selectedLanguage) {
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

	// declarations
	static void setupOutputRedirect(sol::state & state);
	static bool g_bActive = false;
	//
	static void evalLua() {
#ifndef KENGINE_LUA
		addLineToHistory(
			"Please compile with KENGINE_LUA",
			false,
			putils::NormalizedColor{ 1.f, 0.f, 0.f }
		);
#else
		static bool first = true;

		for (const auto & [e, state] : g_em->getEntities<kengine::LuaStateComponent>()) {
			if (first) {
				setupOutputRedirect(*state.state);
				first = false;
			}

			g_bActive = true;
			try {
				state.state->do_string(g_buff);
			}
			catch (const std::exception & e) {
				g_history.addError(e.what());
			}
			g_bActive = false;
		}
#endif
	}

	// declarations
	static int addToHistoryOrPrint(lua_State * L);
	//
	static void setupOutputRedirect(sol::state & state) {
		static const luaL_Reg printlib[] = {
			{ "print", addToHistoryOrPrint },
			{ nullptr, nullptr }
		};

		lua_getglobal(state, "_G");
		luaL_setfuncs(state, printlib, 0);
		lua_pop(state, 1);
	}

	static int addToHistoryOrPrint(lua_State * L) {
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

		if (g_bActive)
			g_history.addLine(std::move(line));
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
			auto sysm = py::module::import("sys");
			_stdout = sysm.attr("stdout");
			_stderr = sysm.attr("stderr");
			auto stringio = py::module::import("io").attr("StringIO");
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
			auto sysm = py::module::import("sys");
			sysm.attr("stdout") = _stdout;
			sysm.attr("stderr") = _stderr;
		}
	};
#endif

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
			py::exec(g_buff);
		}
		catch (const std::exception & e) {
			g_history.addError(e.what());
		}

		auto output = redirect.stdoutString();
		if (!output.empty())
			g_history.addLine(std::move(output));

		auto err = redirect.stderrString();
		if (!err.empty())
			g_history.addError(std::move(err));
#endif
	}
}