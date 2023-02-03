#pragma once

// entt
#include <entt/entity/fwd.hpp>

// kengine meta
#include "kengine/meta/remove.hpp"

// kengine helpers
#include "kengine/helpers/meta/register_meta_component_implementation.hpp"

namespace kengine {
	template<typename T>
	struct meta_component_implementation<meta::remove, T> : std::true_type {
		static void function(entt::handle e) noexcept;
	};
}

#include "remove.inl"