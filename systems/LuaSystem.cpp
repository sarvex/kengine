#include "LuaSystem.hpp"
#include "helpers/LuaHelper.hpp"
#include "data/LuaComponent.hpp"
#include "functions/Execute.hpp"

namespace kengine {
	// declarations
	static void execute(EntityManager & em, sol::state & state, float deltaTime);
	//
	EntityCreatorFunctor<64> LuaSystem(EntityManager & em) {
		return [&](Entity & e) {
			auto tmp = new sol::state;
			e += LuaStateComponent{ tmp };

			auto & state = *tmp;
			state.open_libraries();
			ScriptLanguageHelper::init(em,
				[&](auto && ... args) {
					LuaHelper::detail::registerFunctionWithState(state, FWD(args)...);
				},
				[&](auto type) {
					using T = putils_wrapped_type(type);
					LuaHelper::detail::registerTypeWithState<T>(em, state);
				}
			);

			e += functions::Execute{ [&](float deltaTime) { execute(em, state, deltaTime); } };
		};
	}

	static void execute(EntityManager & em, sol::state & state, float deltaTime) {
		state["deltaTime"] = deltaTime;
		for (auto & [e, comp] : em.getEntities<LuaComponent>()) {
			state["self"] = &e;
			for (const auto & s : comp.scripts)
				state.script_file(s.c_str());
		}
	}
}