import tls;

#include <vector>
#include <thread>
#include <barrier>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("tls::shard<> specification") {
	SECTION("new instances are default initialized") {
		struct test {
			int x = 4;
		};
		REQUIRE(tls::shard<int>::local() == int{});
		REQUIRE(tls::shard<double>::local() == double{});
		REQUIRE(tls::shard<test>::local().x == test{}.x);
	}

	SECTION("can have custom initial values") {
		REQUIRE(tls::shard<int, 2048>::local() == int{ 2048 });
	}

	SECTION("does not cause data races") {
		using accum = tls::shard<unsigned>;

		unsigned N = std::thread::hardware_concurrency();
		unsigned const sum = N * (N - 1) / 2;
		unsigned result = 0;

		// Use a barrier that sums the values when all threads are done
		std::barrier sync_point(N, [&]() noexcept {
			accum::for_each([&](unsigned i) { result += i; });
			});

		// The runner, which sets the threads local data to a value
		auto runner = [&](unsigned i) {
			accum::local() = i;
			sync_point.arrive_and_wait();
			};

		// Create the threads
		{
			std::vector<std::jthread> threads;
			threads.reserve(N);
			while (N--) {
				threads.emplace_back(runner, N);
			}
		}

		REQUIRE(result == sum);
	}

	SECTION("data does not outlive threads") {
		using accum = tls::shard<unsigned>;

		auto runner = [&](unsigned i) {
			accum::local() = i;
			};

		{
			unsigned num_threads = std::thread::hardware_concurrency();
			std::vector<std::jthread> threads;
			threads.reserve(num_threads);

			while (num_threads--) {
				threads.emplace_back(runner, num_threads);
			}
		}

		unsigned result = 0;
		accum::for_each([&](unsigned i) { result += i; });
		REQUIRE(result == 0u);
	}
}

TEST_CASE("tls::shard_retain<> specification") {
	SECTION("preserves data") {
		using retain = tls::shard_retain<unsigned>;

		auto runner = [&](unsigned i) {
			retain::local() = i;
		};

		{
			unsigned num_threads = std::thread::hardware_concurrency();
			std::vector<std::jthread> threads;
			threads.reserve(num_threads);

			while (num_threads--) {
				threads.emplace_back(runner, num_threads);
			}
		}

		unsigned result = 0;
		retain::for_each([&](unsigned i) { result += i; });
		REQUIRE(result > 0u);
	}
}
