#pragma once

// kengine functions
#include "kengine/base_function.hpp"

namespace kengine::functions {
	using on_click_signature = void(int button);
	struct on_click : base_function<on_click_signature> {};
}

#define refltype kengine::functions::on_click
kengine_function_reflection_info;
#undef refltype