#pragma once
#include "core.hpp"
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("avx"), __min_vector_width__(256)))
#define __DEFAULT_FN_ATTRS128 __attribute__((__always_inline__, __nodebug__, __target__("avx"), __min_vector_width__(128)))
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
