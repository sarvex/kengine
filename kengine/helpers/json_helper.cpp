#include "json_helper.hpp"

// entt
#include <entt/entity/handle.hpp>
#include <entt/entity/registry.hpp>

// kengine data
#include "kengine/data/name.hpp"

// kengine meta
#include "kengine/meta/has.hpp"
#include "kengine/meta/load_from_json.hpp"
#include "kengine/meta/save_to_json.hpp"

// kengine helpers
#include "kengine/helpers/sort_helper.hpp"
#include "kengine/helpers/profiling_helper.hpp"

namespace kengine::json_helper {
	void load_entity(const nlohmann::json & entity_json, entt::handle e) noexcept {
		KENGINE_PROFILING_SCOPE;
		kengine_logf(*e.registry(), verbose, "json_helper", "Loading [%u] from JSON", e.entity());
		kengine_logf(*e.registry(), very_verbose, "json_helper", "Input: %s", entity_json.dump(4).c_str());

		for (const auto & [_, loader] : e.registry()->view<const meta::load_from_json>().each())
			loader(entity_json, e);
	}

	nlohmann::json save_entity(entt::const_handle e) noexcept {
		KENGINE_PROFILING_SCOPE;
		kengine_logf(*e.registry(), verbose, "json_helper", "Saving [%u] to JSON", e.entity());

		nlohmann::json ret;

		const auto types = sort_helper::get_name_sorted_entities<const meta::has, const meta::save_to_json>(*e.registry());

		for (const auto & [_, name, has, save] : types) {
			if (!has->call(e))
				continue;

			ret[name->name.c_str()] = save->call(e);
		}

		kengine_logf(*e.registry(), very_verbose, "json_helper", "Output: %s", ret.dump(4).c_str());
		return ret;
	}
}

namespace putils::reflection::detail::json {
	template<>
	void from_to_json(const nlohmann::json & json_object, entt::entity & obj) noexcept {
		auto non_enum_value = entt::id_type(obj);
		from_to_json(json_object, non_enum_value);
		obj = entt::entity(non_enum_value);
	}

	template<>
	inline void from_to_json(nlohmann::json & json_object, const entt::entity & obj) noexcept {
		const auto non_enum_value = entt::id_type(obj);
		from_to_json(json_object, non_enum_value);
	}
}
