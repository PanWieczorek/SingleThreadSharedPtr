#include <catch2/catch_test_macros.hpp>

#include <single_thread_shared_ptr/single_thread_shared_ptr.hpp>

#include <atomic>
using namespace std;

namespace {
class OperatorNewSpy {
public:
  static inline std::atomic<uint32_t> _new_counter{0};
    static inline std::atomic<uint32_t> _delete_counter{0};
  static inline std::atomic<bool> _lock{true};

public:
  OperatorNewSpy() {
    _new_counter = 0;
    _lock = true;
  }

  template <typename Callable> void call(Callable &&c) {
    _lock = false;
    c();
    _lock = true;
  }

  uint32_t countNewCalls() const { return _new_counter.load(); }
  uint32_t countDeleteCalls() const { return _new_counter.load(); }
};
}

void *operator new(size_t size) {
  if (!OperatorNewSpy::_lock)
    OperatorNewSpy::_new_counter++;
  void *p = malloc(size);
  return p;
}

void operator delete(void * p) noexcept
{
    if (!OperatorNewSpy::_lock)
        OperatorNewSpy::_delete_counter++;
    free(p);
}

TEST_CASE("single_thread_shared_ptr_counter construction") {
  OperatorNewSpy spy;

  SECTION("Default construction should not allocate resources") {
    spy.call([]() { [[maybe_unused]] single_thread_shared_ptr_counter c{}; });
    REQUIRE(spy.countNewCalls() == 0);
  }

  SECTION("Default constructed counter is set to one") {
      single_thread_shared_ptr_counter c{};
      REQUIRE(c.count() == 1);
  }

  SECTION("Copy constructor allocates resources") {
      single_thread_shared_ptr_counter c;
      spy.call([&]() { [[maybe_unused]] auto c2{c}; });
      REQUIRE(spy.countNewCalls() == 1);
  }

  SECTION("Copy constructor increments counter"){
      single_thread_shared_ptr_counter c1{};
      auto c2{c1};

      REQUIRE(c1.count() == 2);
      REQUIRE(c2.count() == 2);
  }

  SECTION("Move resets counter to 0"){
      single_thread_shared_ptr_counter c1{};
      auto c2{std::move(c1)};

      REQUIRE(c1.count() == 0);
      REQUIRE(c2.count() == 1);
  }

  SECTION("Destructor decrements the counter"){
      single_thread_shared_ptr_counter c1{};

      {
        [[maybe_unused]] auto c2{c1};
        REQUIRE(c1.count() == 2);
        REQUIRE(c2.count() == 2);
      }

      REQUIRE(c1.count() == 1);
  }

  SECTION("Copy should increment counters"){
      single_thread_shared_ptr_counter c1{};
      auto c2{c1};

      REQUIRE(c1.count() == 2);
      REQUIRE(c2.count() == 2);
  }
}
