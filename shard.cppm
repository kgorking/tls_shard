module;
#include <shared_mutex>
#include <forward_list>
#include <vector>

export module tls:shard;

namespace tls {
	// Provides thread-local instances of the type T, that can later be iterated.
	export template <typename T, auto InitialValue = T{}>
	class shard final {
		struct instance final {
			instance() { shard::add(this); }
			~instance() { shard::remove(this); }

			T data{ InitialValue };
		};

		// the list of all shards
		inline static std::forward_list<instance*> instances{};

		// Mutex for serializing access when adding/removing/iterating shards
		inline static std::shared_mutex mtx;

		// Adds a new shard
		static void add(instance* i) {
			std::unique_lock sl(mtx);
			instances.emplace_front(i);
		}

		// Removes a shard
		static void remove(instance* i) {
			std::unique_lock sl(mtx);
			instances.remove(i);
		}

	public:
		shard() = delete
#ifdef __cpp_deleted_function
			("Don't create instances of this class; it holds no data")
#endif
			;

		// Get the thread-local variable
		[[nodiscard]]
		static T& local() {
			thread_local instance var{};
			return var.data;
		}

		// Perform an action on all shards data.
		// Non-mutating callbacks take a std::shared_lock, otherwise a std::unique_lock is taken.
		static void for_each(std::invocable<T> auto&& fn) {
			if constexpr (std::invocable<decltype(fn), T const&>) {
				std::shared_lock sl(mtx);
				for (instance* i : instances) {
					fn(std::as_const(i->data));
				}
			}
			else {
				std::unique_lock sl(mtx);
				for (instance* i : instances) {
					fn(i->data);
				}
			}
		}
	};
}
