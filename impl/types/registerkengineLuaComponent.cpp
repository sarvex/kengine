#include "helpers/registerTypeHelper.hpp"
#include "data/LuaComponent.hpp"
#include "helpers/logHelper.hpp"
#include "helpers/profilingHelper.hpp"

namespace kengine::types{
	void registerkengineLuaComponent() noexcept {
#ifdef KENGINE_LUA
		KENGINE_PROFILING_SCOPE;
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::LuaComponent'");
		kengine::registerComponents<kengine::LuaComponent>();
#else
		kengine_log(Log, "Init/registerTypes", "Not registering 'kengine::LuaComponent' because 'KENGINE_LUA' is not defined");
#endif
	}
}