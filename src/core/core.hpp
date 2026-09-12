#pragma once
#include <cstddef>
#include <stdint.h>
#include <climits>

// New Keywords
#define restrict			__restrict__
#define static_inl			static inline
#define offsetof(t, d)		__builtin_offsetof(t, d)

// Types
typedef char				i8;
typedef unsigned char		u8;
typedef int16_t				i16;
typedef uint16_t			u16;
typedef int32_t				i32;
typedef uint32_t			u32;
typedef int64_t				i64;
typedef uint64_t			u64;
typedef float				f32;
typedef double				f64;
typedef __int128			i128;
typedef unsigned __int128	u128;
typedef size_t				usize;
typedef ptrdiff_t			isize;
typedef intptr_t			iptr;
typedef uintptr_t			uptr;
typedef unsigned char		uchar;	// For completeness, to mirror platform's type
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

// Wrapped keywords
#if defined(__cplusplus) && __cplusplus >= 201103L
	#define STATIC_ASSERT(expr) static_assert((expr), #expr)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
	#define STATIC_ASSERT(expr) _Static_assert((expr), #expr)
#else
	#define STATIC_ASSERT(expr) typedef char JOIN_MACROS(static_assert_failed_, __LINE__)[(expr) ? 1 : -1]
#endif

#define ARRAY_SIZE(arr)		(sizeof(arr) / sizeof((arr)[0]))
#define ARRAY_END(arr)		(&(arr)[ARRAY_SIZE(arr)])
#define ATTR(kind, ...) ATTR_##kind __attribute__((__VA_ARGS__))
#define ATTR_inl inline __attribute__((always_inline))
#define ATTR_static static
#define ATTR_static_inl static inline __attribute__((always_inline))

// always_inline, noinline, packed, aligned(n), cold, hot
// const: Function depends only on its arguments (doesn't read from memory)
// pure: Function produces no observable side effects (may read from memory)
// flatten: Function calls inside this function are aggressively inlined

#define UNREACHABLE()	__builtin_unreachable()
#define LIKELY(x)		__builtin_expect(!!(x), 1)
#define UNLIKELY(x)		__builtin_expect(!!(x), 0)

#if defined(__clang__)
	#define ASSUME(x)	__builtin_assume(x)
#elif defined(__GNUC__)
	#define ASSUME(x) ((x) ? (void)0 : __builtin_unreachable())
#endif

// Defines
#define WORD_SIZE	sizeof(size_t)
#define WORD_BITS	(WORD_SIZE * CHAR_BIT)

// Macro Helpers
#define JOIN_MACROS_(a, b) a##b
#define JOIN_MACROS(a, b) JOIN_MACROS_(a, b)
#define STRINGIFY_(x)		#x
#define STRINGIFY(x)		STRINGIFY_(x)

#define PRINT_LN(fd, str)		((void)!::write(fd, str "\n", sizeof(str)))
#define PERR_RETURN(value, str)	return (PRINT_LN(2, str), (value))
#define PERR_EXIT(value, str)	_exit((PRINT_LN(2, str), (value)))

#ifdef DEBUG_MODE
	#define ON_DEBUG(x) (x)
	#define ASSERT(x, str) ((x) != 0 ? (void)0 : (PRINT_LN(2, str), __builtin_trap()))
#else
	#define ON_DEBUG(x) ((void)0)
	#define ASSERT(x, str) ((void)0)
#endif

#include "core_builtins.ipp"
#include "core_macros.ipp"
#include "core_info.ipp"
