#pragma once

// glm
#include <glm/glm.hpp>

// putils
#include "putils/point.hpp"

namespace kengine::systems::recast_impl {
	struct adjustables {
		float path_optimization_range = 2.f;
	};

	extern adjustables g_adjustables;
}