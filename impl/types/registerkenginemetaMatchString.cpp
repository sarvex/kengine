#include "helpers/registerTypeHelper.hpp"
#include "meta/MatchString.hpp"
#include "helpers/logHelper.hpp"
#include "helpers/profilingHelper.hpp"

namespace kengine::types{
	void registerkenginemetaMatchString() noexcept {
		KENGINE_PROFILING_SCOPE;
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::meta::MatchString'");
		kengine::registerComponents<kengine::meta::MatchString>();
	}
}