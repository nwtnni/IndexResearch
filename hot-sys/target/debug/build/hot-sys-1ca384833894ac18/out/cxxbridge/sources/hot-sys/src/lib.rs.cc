#include "hot-sys/include/wrap.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#ifdef __clang__
#pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#endif // __clang__
#endif // __GNUC__

namespace rust {
inline namespace cxxbridge1 {
// #include "rust/cxx.h"

#ifndef CXXBRIDGE1_IS_COMPLETE
#define CXXBRIDGE1_IS_COMPLETE
namespace detail {
namespace {
template <typename T, typename = std::size_t>
struct is_complete : std::false_type {};
template <typename T>
struct is_complete<T, decltype(sizeof(T))> : std::true_type {};
} // namespace
} // namespace detail
#endif // CXXBRIDGE1_IS_COMPLETE

namespace {
template <bool> struct deleter_if {
  template <typename T> void operator()(T *) {}
};
template <> struct deleter_if<true> {
  template <typename T> void operator()(T *ptr) { ptr->~T(); }
};
} // namespace
} // namespace cxxbridge1
} // namespace rust

using HOTTree = ::HOTTree;

extern "C" {
::HOTTree *cxxbridge1$hottree_new() noexcept {
  ::std::unique_ptr<::HOTTree> (*hottree_new$)() = ::hottree_new;
  return hottree_new$().release();
}

bool cxxbridge1$hottree_upsert(::HOTTree *tree, ::std::uint64_t key, ::std::uint32_t value) noexcept {
  bool (*hottree_upsert$)(::HOTTree *, ::std::uint64_t, ::std::uint32_t) = ::hottree_upsert;
  return hottree_upsert$(tree, key, value);
}

bool cxxbridge1$hottree_search(::HOTTree *tree, ::std::uint64_t key, ::std::uint32_t *value) noexcept {
  bool (*hottree_search$)(::HOTTree *, ::std::uint64_t, ::std::uint32_t *) = ::hottree_search;
  return hottree_search$(tree, key, value);
}

static_assert(::rust::detail::is_complete<::std::remove_extent<::HOTTree>::type>::value, "definition of `::HOTTree` is required");
static_assert(sizeof(::std::unique_ptr<::HOTTree>) == sizeof(void *), "");
static_assert(alignof(::std::unique_ptr<::HOTTree>) == alignof(void *), "");
void cxxbridge1$unique_ptr$HOTTree$null(::std::unique_ptr<::HOTTree> *ptr) noexcept {
  ::new (ptr) ::std::unique_ptr<::HOTTree>();
}
void cxxbridge1$unique_ptr$HOTTree$raw(::std::unique_ptr<::HOTTree> *ptr, ::std::unique_ptr<::HOTTree>::pointer raw) noexcept {
  ::new (ptr) ::std::unique_ptr<::HOTTree>(raw);
}
::std::unique_ptr<::HOTTree>::element_type const *cxxbridge1$unique_ptr$HOTTree$get(::std::unique_ptr<::HOTTree> const &ptr) noexcept {
  return ptr.get();
}
::std::unique_ptr<::HOTTree>::pointer cxxbridge1$unique_ptr$HOTTree$release(::std::unique_ptr<::HOTTree> &ptr) noexcept {
  return ptr.release();
}
void cxxbridge1$unique_ptr$HOTTree$drop(::std::unique_ptr<::HOTTree> *ptr) noexcept {
  ::rust::deleter_if<::rust::detail::is_complete<::HOTTree>::value>{}(ptr);
}
} // extern "C"
