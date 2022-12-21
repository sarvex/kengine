#pragma once

// putils
#include "putils/point.hpp"

// kengine functions
#include "kengine/base_function.hpp"

namespace kengine::functions {
    struct get_entity_in_pixel : base_function<
        entt::entity(entt::entity window, const putils::point2ui & pixel)
    > {};
}

#define refltype kengine::functions::get_entity_in_pixel
putils_reflection_info{
    putils_reflection_class_name;
	putils_reflection_parents(
		putils_reflection_type(refltype::base)
	);
};
#undef refltype