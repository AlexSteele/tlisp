# tlisp

An interpreter for tlisp, a play thing of mine.

## Language Overview

tlisp is a dynamically typed and imperative-leaning lisp.

Builtin language constructs include functions (lambdas), macros, structs, vectors,
dicts, and ... lists ... as well as control flow constructs (if and while) and
support for global, let-scoped, and function-scoped assignment. 

In addition, it provides basic file IO via a small set libc wrappers.

## Internals

All tlisp objects are twenty-four bytes. They're heap-allocated, and the
garbage collector uses a basic mark-and-sweep scheme (still very much in progress).

## Examples

See the examples directory :)
