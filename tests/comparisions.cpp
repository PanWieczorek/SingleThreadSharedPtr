#include <catch2/catch_test_macros.hpp>

#include <single_thread_shared_ptr/single_thread_shared_ptr.hpp>

namespace {
struct A {
  virtual ~A() {}
};

struct B : A {};

struct T {};
} // namespace

namespace std {
template <> struct less<A *> {
  static int count;
  bool operator()(A *l, A *r) {
    ++count;
    return l < r;
  }
};
int less<A *>::count = 0;
} // namespace std

TEST_CASE("single_thread_shared_ptr comparision operators works") {
  SECTION("operator == and != will compile") {
    single_thread_shared_ptr<T> ptr(new T);
    single_thread_shared_ptr<int> ptr1, ptr2;
    if (ptr == 0) {
    }
    if (0 == ptr) {
    }
    if (ptr != 0) {
    }
    if (0 != ptr) {
    }
    if (ptr1 == ptr2) {
    }
    if (ptr1 == nullptr) {
    }
    if (nullptr == ptr1) {
    }
    if (ptr1 != ptr2) {
    }
    if (ptr1 != nullptr) {
    }
    if (nullptr != ptr1) {
    }
    if (ptr1 < ptr2) {
    }
    if (ptr1 < nullptr) {
    }
    if (nullptr < ptr1) {
    }
    if (ptr1 <= ptr2) {
    }
    if (ptr1 <= nullptr) {
    }
    if (nullptr <= ptr1) {
    }
    if (ptr1 > ptr2) {
    }
    if (ptr1 > nullptr) {
    }
    if (nullptr > ptr1) {
    }
    if (ptr1 >= ptr2) {
    }
    if (ptr1 >= nullptr) {
    }
    if (nullptr >= ptr1) {
    }
  }

  SECTION("Empty ptr compares") {
    single_thread_shared_ptr<A> p1;
    single_thread_shared_ptr<B> p2;
    REQUIRE(p1 == p2);
    REQUIRE(!(p1 != p2));
    REQUIRE((!(p1 < p2) && !(p2 < p1)));
  }

  SECTION("Construction from pointer") {
    single_thread_shared_ptr<A> A_default;

    single_thread_shared_ptr<A> A_from_A(new A);
    REQUIRE(A_default != A_from_A);
    REQUIRE(!(A_default == A_from_A));
    REQUIRE(((A_default < A_from_A) || (A_from_A < A_default)));

    single_thread_shared_ptr<B> B_from_B(new B);
    REQUIRE(B_from_B != A_from_A);
    REQUIRE(!(B_from_B == A_from_A));
    REQUIRE(((B_from_B < A_from_A) || (A_from_A < B_from_B)));

    A_from_A.reset();
    REQUIRE(A_default == A_from_A);
    REQUIRE(!(A_default != A_from_A));
    REQUIRE((!(A_default < A_from_A) && !(A_from_A < A_default)));

    B_from_B.reset();
    REQUIRE(B_from_B == A_from_A);
    REQUIRE(!(B_from_B != A_from_A));
    REQUIRE((!(B_from_B < A_from_A) && !(A_from_A < B_from_B)));
  }
  SECTION("less works") {
    std::less<single_thread_shared_ptr<A>> less;
    // test empty shared_ptrs compare equivalent
    single_thread_shared_ptr<A> p1;
    single_thread_shared_ptr<A> p2;
    REQUIRE((!less(p1, p2) && !less(p2, p1)));
    //#ifndef __cpp_lib_three_way_comparison
    //    // In C++20 std::less<single_thread_shared_ptr<A>> uses the operator<
    //    // synthesized from operator<=>, which uses std::compare_three_way not
    //    // std::less<A*>.
    //    REQUIRE(std::less<A *>::count == 2);
    //#endif
  }

  SECTION("construction from pointer works with less") {
    std::less<single_thread_shared_ptr<A>> less;

    single_thread_shared_ptr<A> empty;
    single_thread_shared_ptr<A> p1(new A);
    single_thread_shared_ptr<A> p2(new A);

    REQUIRE((less(p1, p2) || less(p2, p1)));
    REQUIRE((!(less(p1, p2) && less(p2, p1))));

    p1.reset();
    REQUIRE((!less(p1, empty) && !less(empty, p1)));

    p2.reset();
    REQUIRE((!less(p1, p2) && !less(p2, p1)));
  }

  SECTION("Aliasing ctor") {
    std::less<single_thread_shared_ptr<A>> less;

    A a;
    single_thread_shared_ptr<A> p1(new A);
    single_thread_shared_ptr<A> p2(p1, &a);
    REQUIRE((less(p1, p2) || less(p2, p1)));
  }
}
