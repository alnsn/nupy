==================
Nupy C/C++ Library
==================

Introduction
============

Nupy is a simple set of C++ macros to generate a dtype string for
a class or a struct.

For example, the following definition of the struct Line:

::

	struct Line
	{
	    NUPY_BEGIN(Line)
	
	    double NUPY_MEMBER(start) [2];
	    double NUPY_MEMBER(end  ) [2];
	    char   NUPY_MEMBER(note ) [16];
	
	    NUPY_END()
	};

would generate

::

	int Line::nupy_dtype(const char* str, size_t sz);

with a semantic of snprintf. In fact, it uses only snprintf from
libc. All other dependencies are C++ metaprogramming stuff.

The function would return a dtype for an object of struct Line:

::

	"[('start','<f8',(2)),('end','<f8',(2)),('note','|S16')]"

In plain C `NUPY_BEGIN(class)` and `NUPY_END()` evaluate to nothing
and `NUPY_MEMBER(m)` evaluates to `m`.

No padding between members or at the end are allowed. If there is
a mismatch between a total size of members wrapped by NUPY_MEMBER(m)
and a size of a struct, compile-time assert will be triggered.

Class inheritance is supported with NUPY_BASE(class). There is no
compile-time check to detect a wrong order of NUPY_BASE(class)
for a class with multiple bases.

Requirements
============

You need a decent C++ compiler that works with boost and that
supports `__typeof__`.

The library is header-only. It depends on the following boost
libraries (all header-only):

- enable_if
- mpl
- preprocessor
- type_traits

At the moment, the library does not support big endian architectures.
