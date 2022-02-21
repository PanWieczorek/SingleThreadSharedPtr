
#include <catch2/catch_test_macros.hpp>

#include <single_thread_shared_ptr/single_thread_shared_ptr.hpp>

namespace {
struct A {
  A() { ++ctor_count; }
  virtual ~A() { ++dtor_count; }
  static long ctor_count;
  static long dtor_count;
};
long A::ctor_count = 0;
long A::dtor_count = 0;

struct B : A {
  B() { ++ctor_count; }
  virtual ~B() { ++dtor_count; }
  static long ctor_count;
  static long dtor_count;
};
long B::ctor_count = 0;
long B::dtor_count = 0;

struct reset_count_struct {
  ~reset_count_struct() {
    A::ctor_count = 0;
    A::dtor_count = 0;
    B::ctor_count = 0;
    B::dtor_count = 0;
  }
};

} // namespace

TEST_CASE(
    "single_thread_shared_ptr can be assigned from single_thread_shared_ptr") {
  reset_count_struct __attribute__((unused)) reset;

  SECTION("Assign from same type") {
    single_thread_shared_ptr<A> a;

    a = single_thread_shared_ptr<A>(new A);
    REQUIRE(a.get() != 0);
    REQUIRE(A::ctor_count == 1);
    REQUIRE(A::dtor_count == 0);

    a = single_thread_shared_ptr<A>();
    REQUIRE(a.get() == 0);
    REQUIRE(A::ctor_count == 1);
    REQUIRE(A::dtor_count == 1);
  }

  SECTION("Assign from different type") {
    single_thread_shared_ptr<A> a;

    a = single_thread_shared_ptr<A>();
    REQUIRE(a.get() == nullptr);
    REQUIRE(A::ctor_count == 0);
    REQUIRE(A::dtor_count == 0);
    REQUIRE(B::ctor_count == 0);
    REQUIRE(B::dtor_count == 0);

    a = single_thread_shared_ptr<A>(new A);
    REQUIRE(a.get() != nullptr);
    REQUIRE(A::ctor_count == 1);
    REQUIRE(A::dtor_count == 0);
    REQUIRE(B::ctor_count == 0);
    REQUIRE(B::dtor_count == 0);

    a = single_thread_shared_ptr<B>(new B);
    REQUIRE(a.get() != nullptr);
    REQUIRE(A::ctor_count == 2);
    REQUIRE(A::dtor_count == 1);
    REQUIRE(B::ctor_count == 1);
    REQUIRE(B::dtor_count == 0);
  }

  SECTION("Rvalue assignment from single_thread_shared_ptr") {

    single_thread_shared_ptr<A> a1;
    single_thread_shared_ptr<A> a2(new A);

    a1 = std::move(a2);
    REQUIRE(a1.get() != nullptr);
    REQUIRE(a2.get() == nullptr);
    REQUIRE(a1.use_count() == 1);
    REQUIRE(a2.use_count() == 0);
    REQUIRE(A::ctor_count == 1);
    REQUIRE(A::dtor_count == 0);

    a1 = std::move(single_thread_shared_ptr<A>());
    REQUIRE(a1.get() == 0);
    REQUIRE(A::ctor_count == 1);
    REQUIRE(A::dtor_count == 1);
  }

  SECTION("Rvalue assignment from single_thread_shared_ptr<Y>") {
    single_thread_shared_ptr<A> a;
    single_thread_shared_ptr<B> b(new B);

    a = std::move(b);
    REQUIRE(a.get() != nullptr);
    REQUIRE(b.get() == nullptr);
    REQUIRE(a.use_count() == 1);
    REQUIRE(b.use_count() == 0);
    REQUIRE(A::ctor_count == 1);
    REQUIRE(A::dtor_count == 0);
    REQUIRE(B::ctor_count == 1);
    REQUIRE(B::dtor_count == 0);

    a = std::move(single_thread_shared_ptr<A>());
    REQUIRE(a.get() == nullptr);
    REQUIRE(A::ctor_count == 1);
    REQUIRE(A::dtor_count == 1);
    REQUIRE(B::ctor_count == 1);
    REQUIRE(B::dtor_count == 1);
  }

  REQUIRE( A::ctor_count == A::dtor_count );
  REQUIRE( B::dtor_count == B::dtor_count );
}
