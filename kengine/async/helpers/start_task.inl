#include "start_task.hpp"

// kengine core
#include "kengine/core/helpers/log_helper.hpp"
#include "kengine/core/helpers/profiling_helper.hpp"

// kengine async
#include "kengine/async/data/result.hpp"
#include "kengine/async/data/task.hpp"

namespace kengine::async {
	template<typename T>
	void start_task(entt::registry & r, entt::entity e, const task::string & task_name, std::future<T> && future) noexcept {
		KENGINE_PROFILING_SCOPE;

		kengine_logf(r, log, "async", "Async task '%s' starting", task_name.c_str());
		r.emplace<task>(e, task_name);
		r.emplace<result<T>>(e, std::move(future));
	}
}