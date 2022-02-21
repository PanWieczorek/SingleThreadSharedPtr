#include <catch2/catch_test_macros.hpp>

#include <single_thread_shared_ptr/single_thread_shared_ptr.hpp>
namespace {
struct A {
  A() : i() {}
  int i;
};
} // namespace

TEST_CASE("single_thread_shared_ptr conversion to bool") {
  SECTION("empty pointer reports that is empty") {
    const single_thread_shared_ptr<A> p1;
    REQUIRE(static_cast<bool>(p1) == false);
    const single_thread_shared_ptr<A> p2(p1);
    REQUIRE(static_cast<bool>(p2) == false);
  }

  SECTION("pointer reports empty after reset") {
    single_thread_shared_ptr<A> p1(new A);
    REQUIRE(static_cast<bool>(p1));
    single_thread_shared_ptr<A> p2(p1);
    REQUIRE(static_cast<bool>(p2));
    p1.reset();
    REQUIRE(!static_cast<bool>(p1));
    REQUIRE(static_cast<bool>(p2));
  }

  SECTION("pointer reports empty after reinit") {
    single_thread_shared_ptr<A> p1(new A);
    single_thread_shared_ptr<A> p2(p1);
    p2.reset(new A);
    REQUIRE(static_cast<bool>(p1));
    REQUIRE(static_cast<bool>(p2));
  }
}

TEST_CASE("single_thread_shared_ptr get") {
  SECTION("") {
    A *const a = new A;
    const single_thread_shared_ptr<A> p(a);
    REQUIRE(p.get() == a);
    static_assert(noexcept(p.get()), "non-throwing");
  }

  SECTION("") {
    A *const a = new A;
    const single_thread_shared_ptr<A> p(a);
    REQUIRE(&*p == a);
    static_assert(noexcept(*p), "non-throwing");
  }

  SECTION("") {
    A *const a = new A;
    const single_thread_shared_ptr<A> p(a);
    REQUIRE(&p->i == &a->i);
    static_assert(noexcept(p->i), "non-throwing");
  }

  SECTION("") {
#if !(defined _GLIBCXX_DEBUG && defined _GLIBCXX_DEBUG_PEDANTIC)
    single_thread_shared_ptr<int> p;
    auto np = p.operator->();
    REQUIRE(np == nullptr);
#endif
  }
}
