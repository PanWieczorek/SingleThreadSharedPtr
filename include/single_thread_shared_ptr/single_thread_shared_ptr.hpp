#include <cassert>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

// otherwise, Y* shall be convertible to T*.
template <typename _Tp, typename _Yp>
struct __sp_is_constructible : std::is_convertible<_Yp *, _Tp *>::type {};

// A pointer type Y* is said to be compatible with a pointer type T* when
// either Y* is convertible to T* or Y is U[N] and T is U cv [].
template <typename _Yp_ptr, typename _Tp_ptr>
struct __sp_compatible_with : std::false_type {};

template <typename _Yp, typename _Tp>
struct __sp_compatible_with<_Yp *, _Tp *>
    : std::is_convertible<_Yp *, _Tp *>::type {};

class single_thread_shared_ptr_counter {
  union Storage {
    constexpr Storage(unsigned value) : _local{value} {}
    unsigned _local; // optimization for count == 1 and empty counter
    unsigned
        *_global; // stack based counter, needed when there is more then
                  // one single_thread_shared_ptr pointing to the same object
  };

  static constexpr Storage zero() { return 0; }
  static constexpr Storage one() { return 1; }

public:
  constexpr single_thread_shared_ptr_counter(bool) noexcept
      : _storage{zero()} {}
  constexpr single_thread_shared_ptr_counter() noexcept : _storage{one()} {}

  single_thread_shared_ptr_counter(
      const single_thread_shared_ptr_counter &rhs) noexcept
      : _storage{rhs.increment()} {}

  single_thread_shared_ptr_counter(
      single_thread_shared_ptr_counter &&rhs) noexcept
      : _storage{std::exchange(rhs._storage, zero())} {}

  single_thread_shared_ptr_counter &
  operator=(const single_thread_shared_ptr_counter &rhs) {
    globalCounterCleanup();
    _storage = rhs.increment();
    return *this;
  }

  single_thread_shared_ptr_counter &
  operator=(single_thread_shared_ptr_counter &&rhs) noexcept {
    globalCounterCleanup();
    _storage = std::exchange(rhs._storage, zero());
    return *this;
  }

  ~single_thread_shared_ptr_counter() noexcept {
    if (isGlobalCounter() && --(*_storage._global) == 0) {
      delete _storage._global;
    } else
      _storage._local = 0;
  }

  void globalCounterCleanup() noexcept {
    if (isGlobalCounter() && --(*_storage._global) == 0) {
      delete _storage._global;
      _storage._local = 0;
    }
  }

  unsigned count() const noexcept {
    return isGlobalCounter() ? *_storage._global : _storage._local;
  }

  constexpr bool isNone() const noexcept { return _storage._local == 0; }

  bool isLast() const noexcept {
    return _storage._local == 1 ||
           (_storage._local == 0 ? false : (*_storage._global == 1));
  }

  constexpr bool isGlobalCounter() const noexcept {
    return _storage._local > 1;
  }

  Storage increment() const {
    if (_storage._local == 1) {
      _storage._global = new unsigned(2);
    } else
      isNone() ? _storage._local = 1 : ++(*_storage._global);

    return _storage;
  }

  void swap(single_thread_shared_ptr_counter &rhs) {
    std::swap(_storage, rhs._storage);
  }

private:
  mutable Storage _storage;
};

// forward declarations
template <typename _Tp> class single_thread_shared_ptr;

template <typename _Tp, bool = std::is_void_v<_Tp>>
class single_thread_shared_ptr_access {
public:
  using element_type = _Tp;

  element_type &operator*() const noexcept {
    assert(_M_get() != nullptr);
    return *_M_get();
  }

  element_type *operator->() const noexcept {
    return _M_get();
  }

private:
  element_type *_M_get() const noexcept {
    return static_cast<const single_thread_shared_ptr<_Tp> *>(this)->get();
  }
};

// Define operator-> for shared_ptr<cv void>.
template <typename _Tp> class single_thread_shared_ptr_access<_Tp, true> {
public:
  using element_type = _Tp;

  element_type *operator->() const noexcept {
    auto __ptr =
        static_cast<const single_thread_shared_ptr<_Tp> *>(this)->get();
    return __ptr;
  }
};

template <typename T>
class single_thread_shared_ptr : public single_thread_shared_ptr_access<T> {
private:
  // Constraint for taking ownership of a pointer of type _Yp*:
  template <typename _Yp>
  using _SafeConv =
      typename std::enable_if<__sp_is_constructible<T, _Yp>::value>::type;

  // Constraint for construction from shared_ptr and weak_ptr:
  template <typename _Yp, typename _Res = void>
  using _Compatible =
      typename std::enable_if<__sp_compatible_with<_Yp *, T *>::value,
                              _Res>::type;

public:
  using element_type = typename std::remove_extent_t<T>;

  constexpr single_thread_shared_ptr() noexcept
      : _M_ptr{nullptr}, _counter{true} {}

  template <typename _Yp, typename = _SafeConv<_Yp>>
  constexpr single_thread_shared_ptr(_Yp *_M_ptr) noexcept
      : _M_ptr{_M_ptr}, _counter{} {
    static_assert(!std::is_void_v<_Yp>, "incomplete type");
    static_assert(sizeof(_Yp) > 0, "incomplete type");
  }

  constexpr single_thread_shared_ptr(T *_M_ptr) noexcept
      : _M_ptr{_M_ptr}, _counter{} {}

  // aliasing ctor
  template <class Y>
  single_thread_shared_ptr(const single_thread_shared_ptr<Y> &r, T *p) noexcept
      : _M_ptr{p}, _counter{r._counter} {}

  single_thread_shared_ptr(const single_thread_shared_ptr &rhs) noexcept
      : _M_ptr{rhs._M_ptr}, _counter{rhs._counter} {}

  template <typename _Yp, typename = _Compatible<_Yp>>
  constexpr single_thread_shared_ptr(
      single_thread_shared_ptr<_Yp> &&rhs) noexcept
      : _M_ptr{std::exchange(rhs._M_ptr, nullptr)}, _counter{std::move(
                                                        rhs._counter)} {}

  constexpr single_thread_shared_ptr(single_thread_shared_ptr &&rhs) noexcept
      : _M_ptr{std::exchange(rhs._M_ptr, nullptr)}, _counter{std::move(
                                                        rhs._counter)} {}

  single_thread_shared_ptr &operator=(const single_thread_shared_ptr &rhs) {
    if (_counter.isLast()) {
      delete _M_ptr;
    }
    _M_ptr = rhs._M_ptr;
    _counter = rhs._counter;
    return *this;
  }

  single_thread_shared_ptr &operator=(single_thread_shared_ptr &&rhs) noexcept {
    if (_counter.isLast()) {
      delete _M_ptr;
    }
    _M_ptr = std::exchange(rhs._M_ptr, nullptr);
    _counter = std::move(rhs._counter);
    return *this;
  }

  ~single_thread_shared_ptr() noexcept {
    if (!_M_ptr)
      return;
    if (_counter.isLast())
      delete _M_ptr;
  }

  element_type *get() const noexcept { return _M_ptr; }

  long use_count() const noexcept { return _counter.count(); }

  void swap(single_thread_shared_ptr<T> &rhs) noexcept {
    std::swap(_M_ptr, rhs._M_ptr);
    _counter.swap(rhs._counter);
  }

  void reset() noexcept { single_thread_shared_ptr{}.swap(*this); }

  template <typename _Yp>
  _SafeConv<_Yp> reset(_Yp *rhs) // _Yp must be complete.
  {
    // Catch self-reset errors.
    assert(rhs == 0 || rhs != _M_ptr);
    single_thread_shared_ptr(rhs).swap(*this);
  }

  explicit operator bool() const noexcept { return _M_ptr == 0 ? false : true; }

  template <typename _Yp> friend class single_thread_shared_ptr;

private:
  T *_M_ptr;
  single_thread_shared_ptr_counter _counter;
};

/// Return true if the stored pointer is not null.
/// Equality operator for shared_ptr objects, compares the stored pointers
template <typename _Tp, typename _Up>
[[nodiscard]] inline bool
operator==(const single_thread_shared_ptr<_Tp> &__a,
           const single_thread_shared_ptr<_Up> &__b) noexcept {
  return __a.get() == __b.get();
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool operator==(const single_thread_shared_ptr<_Tp> &__a,
                                     std::nullptr_t) noexcept {
  return !__a;
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool
operator==(std::nullptr_t, const single_thread_shared_ptr<_Tp> &__a) noexcept {
  return !__a;
}

/// Inequality operator for shared_ptr objects, compares the stored pointers
template <typename _Tp, typename _Up>
[[nodiscard]] inline bool
operator!=(const single_thread_shared_ptr<_Tp> &__a,
           const single_thread_shared_ptr<_Up> &__b) noexcept {
  return __a.get() != __b.get();
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool operator!=(const single_thread_shared_ptr<_Tp> &__a,
                                     std::nullptr_t) noexcept {
  return (bool)__a;
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool
operator!=(std::nullptr_t, const single_thread_shared_ptr<_Tp> &__a) noexcept {
  return (bool)__a;
}

/// Relational operator for shared_ptr objects, compares the stored pointers
template <typename _Tp, typename _Up>
[[nodiscard]] inline bool
operator<(const single_thread_shared_ptr<_Tp> &__a,
          const single_thread_shared_ptr<_Up> &__b) noexcept {
  using _Tp_elt = typename single_thread_shared_ptr<_Tp>::element_type;
  using _Up_elt = typename single_thread_shared_ptr<_Up>::element_type;
  using _Vp = std::common_type_t<_Tp_elt *, _Up_elt *>;
  return std::less<_Vp>()(__a.get(), __b.get());
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool operator<(const single_thread_shared_ptr<_Tp> &__a,
                                    std::nullptr_t) noexcept {
  using _Tp_elt = typename single_thread_shared_ptr<_Tp>::element_type;
  return std::less<_Tp_elt *>()(__a.get(), nullptr);
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool
operator<(std::nullptr_t, const single_thread_shared_ptr<_Tp> &__a) noexcept {
  using _Tp_elt = typename single_thread_shared_ptr<_Tp>::element_type;
  return std::less<_Tp_elt *>()(nullptr, __a.get());
}

/// Relational operator for shared_ptr objects, compares the stored pointers
template <typename _Tp, typename _Up>
[[nodiscard]] inline bool
operator<=(const single_thread_shared_ptr<_Tp> &__a,
           const single_thread_shared_ptr<_Up> &__b) noexcept {
  return !(__b < __a);
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool operator<=(const single_thread_shared_ptr<_Tp> &__a,
                                     std::nullptr_t) noexcept {
  return !(nullptr < __a);
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool
operator<=(std::nullptr_t, const single_thread_shared_ptr<_Tp> &__a) noexcept {
  return !(__a < nullptr);
}

/// Relational operator for shared_ptr objects, compares the stored pointers
template <typename _Tp, typename _Up>
[[nodiscard]] inline bool
operator>(const single_thread_shared_ptr<_Tp> &__a,
          const single_thread_shared_ptr<_Up> &__b) noexcept {
  return (__b < __a);
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool operator>(const single_thread_shared_ptr<_Tp> &__a,
                                    std::nullptr_t) noexcept {
  return nullptr < __a;
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool
operator>(std::nullptr_t, const single_thread_shared_ptr<_Tp> &__a) noexcept {
  return __a < nullptr;
}

/// Relational operator for shared_ptr objects, compares the stored pointers
template <typename _Tp, typename _Up>
[[nodiscard]] inline bool
operator>=(const single_thread_shared_ptr<_Tp> &__a,
           const single_thread_shared_ptr<_Up> &__b) noexcept {
  return !(__a < __b);
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool operator>=(const single_thread_shared_ptr<_Tp> &__a,
                                     std::nullptr_t) noexcept {
  return !(__a < nullptr);
}

/// shared_ptr comparison with nullptr
template <typename _Tp>
[[nodiscard]] inline bool
operator>=(std::nullptr_t, const single_thread_shared_ptr<_Tp> &__a) noexcept {
  return !(nullptr < __a);
}

namespace std {
template <typename _Tp>
struct hash<single_thread_shared_ptr<_Tp>>
    : public __hash_base<size_t, single_thread_shared_ptr<_Tp>> {
  size_t operator()(const single_thread_shared_ptr<_Tp> &s) const noexcept {
    return std::hash<typename single_thread_shared_ptr<_Tp>::element_type *>()(
        s.get());
  }
};
} // namespace std
