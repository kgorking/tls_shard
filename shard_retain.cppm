module;
#include <shared_mutex>
#include <forward_list>
#include <vector>

export module tls:shard_retain;

namespace tls {
	// Provides thread-local instances of the type T, that can later be iterated.
	export template <typename T, auto InitialValue = T{}>
	class shard_retain final {
		using shard_pair = std::pair<T, bool>;
		using shard_list = std::forward_list<shard_pair>;
		using shard_iterator = shard_list::iterator;

		// This struct manages the instances that access the thread-local data.
		// Its lifetime is marked as thread_local in shard::local(), which means that it can live longer than
		// the shard<> instance that spawned it.
		struct shard_data final {
			shard_data() { shard_retain::add(this); }
			~shard_data() { it->second = true; }

			shard_iterator it;
		};

		// the list of all shards
		inline static shard_list head{};

		// Mutex for serializing access when adding/removing/iterating shards
		inline static std::shared_mutex mtx;

		// Adds a new shard
		static void add(shard_data* t) {
			std::unique_lock sl(mtx);
			head.emplace_front(InitialValue, false);
			t->it = head.begin();
		}

	public:
		shard_retain() = delete
#ifdef __cpp_deleted_function
			("Don't create instances of this class; it holds no data")
#endif
			;

		// Get the thread-local variable
		[[nodiscard]]
		static T& local() {
			thread_local shard_data var{};
			return var.it->first;
		}

		static void remove_dead_data() {
			std::erase_if(head, [](shard_data sd) {
				return sd->it->second;
			});
		}

		// Perform an action on all shards data.
		// Non-mutating callbacks take a std::shared_lock, otherwise a std::unique_lock is taken.
		static void for_each(std::invocable<T> auto&& fn) {
			if constexpr (std::invocable<decltype(fn), T const&>) {
				std::shared_lock sl(mtx);
				for (shard_pair const& data : head) {
					fn(data.first);
				}
			}
			else {
				std::unique_lock sl(mtx);
				for (shard_pair& data : head) {
					fn(data.first);
				}
			}
		}
	};
}
