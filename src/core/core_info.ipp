#pragma once
#include "core.hpp"
#include "limits"

/* Acts as an expansion to sizeof function:
	sizeof_bits:	number of bits of that type
	sizeof_max:		maximum size of that type
	sizeof_min:		lowest size of that type
	sizeof_array:	element count of an array
	sizeof_digits:	number of digits to represent the max size of the type
*/

template <typename Type>
static usize sizeof_bits(const Type&) {
	return sizeof(Type) * CHAR_BIT;
}

template <typename Type>
static Type sizeof_max(const Type&) {
	return std::numeric_limits<Type>::max();
}

template <typename Type>
static Type sizeof_min(const Type&) {
	return std::numeric_limits<Type>::lowest();
}

template <typename Type, usize Count>
static usize sizeof_array(const Type (&)[Count]) {
	return Count;
}

template <typename Type>
static usize sizeof_digits8(const Type&) {
	return (sizeof(Type) * CHAR_BIT + 2) / 3;
}

template <typename Type>
static usize sizeof_digits10(const Type&) {
	return std::numeric_limits<Type>::digits10 + 1;
}

template <typename Type>
static usize sizeof_digits16(const Type&) {
	return (sizeof(Type) * CHAR_BIT + 3) / 4;
}

template <>
inline f32 sizeof_min<f32>(const f32&) {
	return -std::numeric_limits<f32>::max();
}

template <>
inline f64 sizeof_min<f64>(const f64&) {
	return -std::numeric_limits<f64>::max();
}
