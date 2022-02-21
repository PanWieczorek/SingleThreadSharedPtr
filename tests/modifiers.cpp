#include <catch2/catch_test_macros.hpp>

#include <single_thread_shared_ptr/single_thread_shared_ptr.hpp>

namespace {
struct A { };
struct B : A { };
} // namespace

TEST_CASE("single_thread_shared_ptr swap") {
    SECTION("swap works") {
        A * const a1 = new A;
        A * const a2 = new A;
        single_thread_shared_ptr<A> p1(a1);
        single_thread_shared_ptr<A> p2(a2);
        p1.swap(p2);
        REQUIRE( p1.get() == a2 );
        REQUIRE( p2.get() == a1 );
    }
    SECTION("global swap works") {
        A * const a1 = new A;
        A * const a2 = new A;
        single_thread_shared_ptr<A> p1(a1);
        single_thread_shared_ptr<A> p2(a2);
        std::swap(p1, p2);
        REQUIRE( p1.get() == a2 );
        REQUIRE( p2.get() == a1 );
    }
}

TEST_CASE("single_thread_shared_ptr reset"){
    SECTION("reset works") {
        A * const a = new A;
        single_thread_shared_ptr<A> p1(a);
        single_thread_shared_ptr<A> p2(p1);
        p1.reset();
        REQUIRE( p1.get() == nullptr );
        REQUIRE( p2.get() == a );
    }

    SECTION("reset works with different types") {
        A * const a = new A;
        B * const b = new B;
        single_thread_shared_ptr<A> p1(a);
        single_thread_shared_ptr<A> p2(p1);
        p1.reset(b);
        REQUIRE( p1.get() == b );
        REQUIRE( p2.get() == a );
    }
}


template<typename T, typename Args, typename = void>
struct resettable
    : std::false_type
{ };

template<typename... T> struct type_list { };

template<typename T, typename... Args>
using reset_result
    = decltype(single_thread_shared_ptr<T>{}.reset(std::declval<Args>()...));

template<typename T, typename... Args>
struct resettable<T, type_list<Args...>, reset_result<T, Args...>>
    : std::true_type
{ };

template<typename T, typename... Args>
constexpr bool can_reset()
{ return resettable<T, type_list<Args...>>::value; }

template<typename T>
struct Deleter {
    void operator()(T*) const;
};

template<typename T>
using Alloc = std::allocator<T>;

struct Base { };
struct Derived : Base { };

// Positive cases:

static_assert( can_reset<const void, void*>(),
              "void* convertible to const void*");
static_assert( can_reset<const int, int*>(),
              "int* convertible to const int*");
static_assert( can_reset<Base, Derived*>(),
              "Derived* convertible to Base*");
static_assert( can_reset<const Base, Derived*>(),
              "Derived* convertible to const Base*");

// Negative cases:

static_assert( !can_reset<int, void*>(),
              "void* not convertible to int*");

static_assert( !can_reset<int, const int*>(),
              "const int* not convertible to int*");

static_assert( !can_reset<int, long*>(),
              "long* not convertible to int*");

static_assert( !can_reset<Derived, Base*>(),
              "Base* not convertible to Derived*");
