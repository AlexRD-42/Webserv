# Style Guidelines
Class/Struct names:	PascalCase
Booleans: 			Predicate names like is_visible, has_focus, can_jump

`k_` — Constant Variable
`g_` — Global variable
`s_` — Static variable or function
`m_` — Non-static class member variable

`x_` — Local static variable
`e_` — Enumerated constant type
`t_` — Template argument

stt_ = static
stf_ = static filescope

# Different types
Static:
	Variable (function scope)
	Variable (global scope)
	Variable (class scope)
	Function

Attribute	(Public)
Attribute	(Private)
Variable	(Function parameter)
Variable	(Function variable)
Variable	(Constant):

Class name
Enums

# Modern Hungarian Notation
The goal of this is to encode meaningful semantic information onto the variable, without adding too much noise

Axioms:
- s_, m_ and g_ prefixes are commonly used in codebases
- k is hard to encode because it can be part of all prefixes

I think the best approach might be to combine common underscore prefixes with regular prefixes to encode additional information
For example, `g_kVar` is better than `gkVar`

The problem is with combining them. Assume we have k_ prefixes for constant and s_ prefixes for static. What happens for a static const?
Is it `sk_var`? that looks weird because all prefixes are generally one letter

I think scope prefixes are high value, 
`g_` Global variable
`s_` Static variable or function

## Primitives:
`b` : bool				: Could be identified instead with predicates like has_ is_ can_
what is better, `is_visible`, `bVisible` or `isVisible`?
`u`: unsigned integer	: Could be default to reduce pollution
`i`: signed integer		:
`f`: floating-point

## Semantic
n: count (presumed unsigned)
k: constant

`sm_knItems`