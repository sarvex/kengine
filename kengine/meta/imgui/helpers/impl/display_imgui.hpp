#pragma once

// entt
#include <entt/entity/fwd.hpp>

// kengine meta
#include "kengine/meta/imgui/functions/display_imgui.hpp"

// kengine helpers
#include "kengine/meta/helpers/impl/meta_component_implementation.hpp"

namespace kengine::meta {
	template<typename T>
	struct meta_component_implementation<display_imgui, T> : std::true_type {
		static void function(entt::const_handle e) noexcept;
	};
}

#include "display_imgui.inl"