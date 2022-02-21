#include <catch2/catch_test_macros.hpp>

#include <single_thread_shared_ptr/single_thread_shared_ptr.hpp>

namespace {
struct T {};
} // namespace

TEST_CASE("single_thread_shared_ptr generates good hash") {
  single_thread_shared_ptr<T> s0(new T);
  std::hash<single_thread_shared_ptr<T>> hs0;
  std::hash<T *> hp0;

  REQUIRE(hs0(s0) == hp0(s0.get()));
}
