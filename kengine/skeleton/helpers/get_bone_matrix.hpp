#pragma once

// entt
#include <entt/entity/fwd.hpp>

// glm
#include <glm/glm.hpp>

// kengine
#include "kengine/skeleton/data/skeleton.hpp"
#include "kengine/skeleton/data/model_skeleton.hpp"

namespace kengine::skeleton {
	KENGINE_SKELETON_EXPORT ::glm::mat4 get_bone_matrix(const entt::registry & r, const char * bone, const skeleton & skeleton, const model_skeleton & model) noexcept;
}