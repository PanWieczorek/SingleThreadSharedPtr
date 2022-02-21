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

TEST_CASE("single_thread_shared_ptr can be constructed from raw pointer") {
  reset_count_struct __attribute__((unused)) reset;

  SECTION("Construct empty pointer") {
    { single_thread_shared_ptr<A> a; }
    REQUIRE(A::ctor_count == 0);
    REQUIRE(A::dtor_count == 0);
    REQUIRE(B::ctor_count == 0);
    REQUIRE(B::dtor_count == 0);
  }

  SECTION("Construct from nullptr should increase use count") {
    A *const a = nullptr;
    single_thread_shared_ptr<A> p(a);
    REQUIRE(p.get() == nullptr);
    REQUIRE(p.use_count() == 1);
  }

  SECTION("Construct from a valid object should store a valid pointer") {
    A *const a = new A;
    single_thread_shared_ptr<A> p(a);
    REQUIRE(p.get() == a);
    REQUIRE(p.use_count() == 1);
  }

  SECTION("Construct from derived class works") {
    B *const b = new B;
    single_thread_shared_ptr<A> p(b);
    REQUIRE(p.get() == b);
    REQUIRE(p.use_count() == 1);
  }
}

TEST_CASE("single_thread_shared_ptr can be constructed by move") {
  reset_count_struct __attribute__((unused)) reset;

  SECTION("Rvalue construction from a empty ptr") {
    single_thread_shared_ptr<A> a1;
    single_thread_shared_ptr<A> a2(std::move(a1));
    REQUIRE(a1.use_count() == 0);
    REQUIRE(a2.use_count() == 0);
    REQUIRE(A::ctor_count == 0);
    REQUIRE(A::dtor_count == 0);
    REQUIRE(B::ctor_count == 0);
    REQUIRE(B::dtor_count == 0);
  }

  SECTION("Rvalue construction from another ptr") {
    single_thread_shared_ptr<A> a1(new A);
    single_thread_shared_ptr<A> a2(std::move(a1));
    REQUIRE(a1.use_count() == 0);
    REQUIRE(a2.use_count() == 1);
    REQUIRE(A::ctor_count == 1);
    REQUIRE(A::dtor_count == 0);
  }

  SECTION("Move pointer to different pointer") {
    single_thread_shared_ptr<B> b(new B);
    single_thread_shared_ptr<A> a(std::move(b));
    REQUIRE(b.use_count() == 0);
    REQUIRE(a.use_count() == 1);
    REQUIRE(A::ctor_count == 1);
    REQUIRE(A::dtor_count == 0);
    REQUIRE(B::ctor_count == 1);
    REQUIRE(B::dtor_count == 0);
  }

  SECTION("0") {
    auto data = new int(0);
    single_thread_shared_ptr<int> x{data};
  }

  //    SECTION("Move pointer with user defined destructor"){
  //        single_thread_shared_ptr<B> b(new B, D());
  //        single_thread_shared_ptr<A> a(std::move(b));
  //        REQUIRE( b.use_count() == 0 );
  //        REQUIRE( a.use_count() == 1 );
  //        REQUIRE( A::ctor_count == 1 );
  //        REQUIRE( A::dtor_count == 0 );
  //        REQUIRE( B::ctor_count == 1 );
  //        REQUIRE( B::dtor_count == 0 );

  //        a = std::move(single_thread_shared_ptr<A>());
  //        REQUIRE( D::delete_count == 1 );
  //        REQUIRE( B::dtor_count == 1 );
  //    }

  SECTION("Rvalue construction") {
    single_thread_shared_ptr<A> a(
        std::move(single_thread_shared_ptr<A>(new A)));
    REQUIRE(a.use_count() == 1);
    REQUIRE(A::ctor_count == 1);
    REQUIRE(A::dtor_count == 0);
  }

  SECTION("Aliasing ctor"){
      A a;
      single_thread_shared_ptr<A> p1(new A);
      single_thread_shared_ptr<A> p2(p1, &a);
  }

  REQUIRE(A::ctor_count == A::dtor_count);
  REQUIRE(B::dtor_count == B::dtor_count);
}
