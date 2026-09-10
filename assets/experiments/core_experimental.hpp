#pragma once
#include "core.hpp"

union m64 {
	union {
		void* v;
		::u8* u8;
		::i8* i8;
		::u16* u16;
		::i16* i16;
		::u32* u32;
		::i32* i32;
		::u64* u64;
		::i64* i64;
	}	p;
	union {
		::usize usize;
		::isize isize;
		::u64 u64;
		::i64 i64;
		::f64 f64;
		::u32 u32[2];
		::u16 u16[4];
		::u8 u8[8];
	};
};

struct Pair {
    m64 first;
    m64 second;
};

// Insane shit

// #define JOIN_MACROS_(a, b) a##b
// #define JOIN_MACROS(a, b) JOIN_MACROS_(a, b)
// #define PROBE() ~, 1
// #define SECOND(a, b, ...) b
// #define IS_PROBE(...) SECOND(__VA_ARGS__, 0)

// #define IS_INL(x) IS_PROBE(JOIN_MACROS(IS_INL_, x))
// #define IS_INL_inl PROBE()

// #define FIRST(a, ...) a

// #define IF_0(t, f) f
// #define IF_1(t, f) t
// #define IF(x) JOIN_MACROS(IF_, x)

// #define FN_INL(_inl, ...) \
//     inline __attribute__((always_inline __VA_OPT__(,) __VA_ARGS__))

// #define FN_ATTR(...) \
//     __attribute__((__VA_ARGS__))

// #define FN(...) \
// 	IF(IS_INL(FIRST(__VA_ARGS__)))( \
// 		FN_INL(__VA_ARGS__), \
// 		FN_ATTR(__VA_ARGS__) \
// 	)

// 	FN(inl, pure)