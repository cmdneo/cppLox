#ifndef GARBAGE_HXX_INCLUDED
#define GARBAGE_HXX_INCLUDED

#include <cassert>
#include <chrono>
#include <initializer_list>
#include <map>
#include <memory>
#include <vector>
#include <variant>
#include <utility>

#include "object.hxx"
#include "lox_function.hxx"
#include "lox_instance.hxx"
#include "environment.hxx"

static constexpr std::chrono::milliseconds GC_RUN_INTERVAL{100};

class GarbageCollector
{
public:
	GarbageCollector(std::initializer_list<EnvironmentPtr> environs)
	{
		for (auto &env : environs) {
			environments.push_back(env);
			directly_reachable.push_back(env);
		}
	}

	void push_environment(const EnvironmentPtr &environment)
	{
		environments.push_back(environment);
		directly_reachable.push_back(environment);
	}

	void pop_environment() { directly_reachable.pop_back(); }

	void collect()
	{
		// The timer was removed, because it caused a lot of page faults,
		// IDK why :|
		collect_impl();
	}

	//~GarbageCollector() { collect_impl(); }

private:
	void collect_impl()
	{
		// Follow the chain from directly-reachable environments
		// and mark all which are reachable
		for (auto &env : directly_reachable)
			mark_reachable(env);

		auto swap_remove = [](auto &vec, unsigned remove_at,
							  unsigned swap_with) {
			using std::swap;
			swap(vec[remove_at], vec[swap_with]);
			vec.pop_back();
		};

		// Clear out unreachable environments
		int length = environments.size();
		for (int i = 0; i < length;) {
			if (environments[i].expired()) {
				length -= 1;
				swap_remove(environments, i, length);
			} else if (auto locked = environments[i].lock();
					   !locked->reachable) {
				length -= 1;
				locked->values.clear();
				swap_remove(environments, i, length);
			} else {
				// Only move to next if current one was not removed
				// as when removed it is substituted with the last entry.
				// So we would like to visit that substituted entry too.
				++i;
			}
		}

		// Mark all as unreachable for the next round
		for (auto &env : environments)
			env.lock()->reachable = false;
	}

	void mark_reachable(const std::weak_ptr<Environment> &environment)
	{
		// The environments containing 'this' and 'super' are not tracked here
		// Indirectly reachable environments are also always valid,
		// so no need to check before locking
		auto env = environment.lock();
		env->reachable = true;

		if (env->enclosing != nullptr)
			mark_reachable(env->enclosing);

		for (auto &[name, object] : env->values)
			mark_reachable_from_object(object);
	}

	void mark_reachable_from_object(Object &object)
	{
		// Function objects have environments
		if (match_types<LoxCallablePtr>(object)) {
			auto &callable = *std::get<LoxCallablePtr>(object);
			if (typeid(callable) != typeid(LoxFunction))
				return;

			auto env = dynamic_cast<LoxFunction &>(callable).closure;

			// If already visited, then return
			if (env->reachable)
				return;

			mark_reachable(env);
		}
		// An instance fields can have function objects which have environments
		else if (match_types<LoxInstancePtr>(object)) {
			for (auto &[name, obj] : std::get<LoxInstancePtr>(object)->fields)
				mark_reachable_from_object(obj);
		}
	}

	// Maps environment to whether are they reachable or not
	std::vector<std::weak_ptr<Environment>> environments;
	// Environments that enclose the currently active environment,
	// they are direclty reachable because they can be found via
	// traversing the environment chain upwards and they are always valid.
	std::vector<std::weak_ptr<Environment>> directly_reachable;

	using clock = std::chrono::steady_clock;
	// Last time the GC was run
	clock::time_point last_run_at = clock::now();
};

#endif
