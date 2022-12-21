#include "kengine/helpers/register_type_helper.hpp"
#include "kengine/meta/copy.hpp"

// entt
#include <entt/entity/fwd.hpp>

// kengine helpers
#include "kengine/helpers/log_helper.hpp"
#include "kengine/helpers/profiling_helper.hpp"

namespace kengine::types{
	void register_kengine_meta_copy(entt::registry & r) noexcept {
		KENGINE_PROFILING_SCOPE;
		kengine_log(r, log, "init/register_types", "Registering 'kengine::meta::copy'");
		kengine::register_components<kengine::meta::copy>(r);
	}
}