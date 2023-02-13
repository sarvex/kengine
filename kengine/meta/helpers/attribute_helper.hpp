#pragma once

// entt
#include <entt/entity/fwd.hpp>

// kengine meta
#include "kengine/meta/data/attributes.hpp"

namespace kengine::meta::attribute_helper {
	KENGINE_META_EXPORT const putils::reflection::runtime::attribute_info * find_attribute(entt::handle type_entity, std::string_view path, std::string_view separator = ".") noexcept;
}