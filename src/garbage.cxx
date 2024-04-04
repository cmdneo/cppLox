#include <memory>
#include <utility>
#include <variant>

#include "garbage.hxx"
#include "environment.hxx"
#include "object/object.hxx"
#include "object/lox_function.hxx"
#include "object/lox_instance.hxx"

void GarbageCollector::collect_impl()
{
	// Follow the chain from directly-reachable environments
	// and mark all which are reachable
	for (auto &env : directly_reachable)
		mark_reachable(env);

	auto swap_remove = [](auto &vec, unsigned remove_at) {
		using std::swap;
		swap(vec[remove_at], vec[vec.size() - 1]);
		vec.pop_back();
	};

	// Remove unreachable environments, using swap_remove.
	int length = environments.size();
	for (int i = 0; i < length;) {
		if (environments[i].expired()) {
			length -= 1;
			swap_remove(environments, i);
		} else if (auto locked = environments[i].lock(); !locked->reachable) {
			length -= 1;
			locked->values.clear();
			swap_remove(environments, i);
		} else {
			// Only move to next if current one was not removed
			// as when removed it is substituted with the last entry.
			// And we need to visit that substituted entry too.
			++i;
		}
	}

	// Mark all as unreachable for the next round
	for (auto &env : environments)
		env.lock()->reachable = false;
}

void GarbageCollector::mark_reachable(
	const std::weak_ptr<Environment> &environment
)
{
	// The environments containing 'this' and 'super' are not tracked here
	// because they are never added to the garbage collector explicitly.
	// 'super' is referenced by every function stored in a class and is fixed.
	// 'this' is referenced by every instance and bound methods when created.

	// Indirectly reachable environments are also always valid,
	// so no need to check before locking.
	auto env = environment.lock();
	if (env->reachable)
		return;

	env->reachable = true;

	if (env->enclosing != nullptr)
		mark_reachable(env->enclosing);

	for (auto &[name, object] : env->values)
		mark_reachable_from_object(object);
}

void GarbageCollector::mark_reachable_from_object(Object &object)
{
	// Function objects have environments
	if (match_types<LoxCallablePtr>(object)) {
		auto &callable = *std::get<LoxCallablePtr>(object);
		if (typeid(callable) != typeid(LoxFunction))
			return;

		auto env = dynamic_cast<LoxFunction &>(callable).closure;
		mark_reachable(env);
	}

	// FIXME Causes infinite recursion for self referential instances.
	// An instance fields can have function objects which have environments
	else if (match_types<LoxInstancePtr>(object)) {
		for (auto &[name, obj] : std::get<LoxInstancePtr>(object)->fields)
			mark_reachable_from_object(obj);
	}
}