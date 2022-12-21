#include "imgui_helper.hpp"

// entt
#include <entt/entity/handle.hpp>
#include <entt/entity/registry.hpp>

// imgui
#include <imgui.h>

// kengine data
#include "kengine/data/imgui_scale.hpp"
#include "kengine/data/instance.hpp"

// kengine meta
#include "kengine/meta/has.hpp"
#include "kengine/meta/attach_to.hpp"
#include "kengine/meta/detach_from.hpp"
#include "kengine/meta/display_imgui.hpp"
#include "kengine/meta/edit_imgui.hpp"

// kengine helpers
#include "kengine/helpers/type_helper.hpp"
#include "kengine/helpers/sort_helper.hpp"
#include "kengine/helpers/profiling_helper.hpp"

namespace kengine::imgui_helper {
	void display_entity(entt::const_handle e) noexcept {
		KENGINE_PROFILING_SCOPE;

		const auto types = sort_helper::get_name_sorted_entities<const meta::has, const meta::display_imgui>(*e.registry());

		for (const auto & [_, name, has, display] : types)
			if (has->call(e))
				if (ImGui::TreeNode(name->name.c_str())) {
					display->call(e);
					ImGui::TreePop();
				}
	}

	void display_entity_and_model(entt::const_handle e) noexcept {
		KENGINE_PROFILING_SCOPE;

		const auto instance = e.try_get<data::instance>();
		if (!instance || instance->model == entt::null) {
			display_entity(e);
			return;
		}

		if (ImGui::BeginTabBar("##tabs")) {
			if (ImGui::BeginTabItem("object")) {
				display_entity(e);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Model")) {
				display_entity({ *e.registry(), instance->model });
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	void edit_entity(entt::handle e) noexcept {
		KENGINE_PROFILING_SCOPE;

		const auto & r = *e.registry();

		if (ImGui::BeginPopupContextWindow()) {
			const auto types = sort_helper::get_name_sorted_entities<const meta::has, const meta::attach_to>(r);

			for (const auto & [_, name, has, add] : types)
				if (!has->call(e))
					if (ImGui::MenuItem(name->name.c_str()))
						add->call(e);

			ImGui::EndPopup();
		}

		const auto types = sort_helper::get_name_sorted_entities<const meta::has, const meta::edit_imgui>(r);

		for (const auto & [type_entity, name, has, edit] : types) {
			if (!has->call(e))
				continue;
			const auto tree_node_open = ImGui::TreeNode((name->name + "##edit").c_str());

			const auto detach_from = r.try_get<meta::detach_from>(type_entity);
			if (detach_from) {
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Remove"))
						detach_from->call(e);
					ImGui::EndPopup();
				}
			}
			if (tree_node_open) {
				edit->call(e);
				ImGui::TreePop();
			}
		}
	}

	void edit_entity_and_model(entt::handle e) noexcept {
		KENGINE_PROFILING_SCOPE;

		const auto instance = e.try_get<data::instance>();
		if (!instance || instance->model == entt::null) {
			edit_entity(e);
			return;
		}

		if (ImGui::BeginTabBar("##tabs")) {
			ImGui::PushItemWidth(ImGui::GetWindowWidth() / 2.f);

			if (ImGui::BeginTabItem("object")) {
				edit_entity(e);
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Model")) {
				edit_entity({ *e.registry(), instance->model });
				ImGui::EndTabItem();
			}

			ImGui::PopItemWidth();

			ImGui::EndTabBar();
		}
	}

	float get_scale(const entt::registry & r) noexcept {
		KENGINE_PROFILING_SCOPE;

		float scale = 1.f;
		for (const auto & [e, comp] : r.view<data::imgui_scale>().each())
			scale *= comp.scale;
		return scale;
	}
}