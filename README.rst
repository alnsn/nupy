==================
Nupy C/C++ Library
==================

-----------------------------
Expose C/C++ structs to numpy
-----------------------------

SYNOPSIS
========

    #define NUPY_BEGIN(T)  begin a declaration of T

    #define NUPY_END()     end a declaration

    #define NUPY_BASE(T)   declare a base T - C++ only

    #define NUPY_MEMBER(m) declare a member m

DESCRIPTION
===========

Nupy is a simple set of C++ macros to generate a dtype string for
a class or a struct.

For example, C/C++ ``struct Line``

::

	struct Line
	{
		double start[2];
		double end  [2];
		char   note [16];
	};

can be "decorated" by ``NUPY_BEGIN(Line)`` and ``NUPY_END()``
macros and all members can be wrapped by ``NUPY_MEMBER(m)``

::

	struct Line
	{
		NUPY_BEGIN(Line)
		
		double NUPY_MEMBER(start) [2];
		double NUPY_MEMBER(end  ) [2];
		char   NUPY_MEMBER(note ) [16];
		
		NUPY_END()
	};

In plain C ``NUPY_BEGIN(class)`` and ``NUPY_END()`` evaluate to nothing
and ``NUPY_MEMBER(m)`` evaluates to ``m`` producing a struct identical
to the original.

In C++, the second definition generates ``nupy_dtype`` static
member function

::

	int Line::nupy_dtype(const char* str, size_t bufsz);

If ``bufsz`` is big enough the function copies ``Line``'s dtype
to ``buf`` producing a string similar to

::

	"[('start','<f8',(2)),('end','<f8',(2)),('note','|S16')]"


A call to ``nupy_dtype(buf, bufsz)`` is equivalent to ``snprintf``
call

::

	snprintf(buf, bufsz, "%s", dtypestr);

except that ``snprintf`` can be called multiple times.

In fact, ``nupy_dtype`` uses only ``snprintf`` from ``-lc``. All other
dependencies are C++ metaprogramming stuff.

No padding between members or at the end are allowed. If there is
a mismatch between a total size of members wrapped by ``NUPY_MEMBER(m)``
and a size of a struct, compile-time assert will be triggered.

Class inheritance is supported with ``NUPY_BASE(class)``. There is no
compile-time check to detect a wrong order of ``NUPY_BASE(class)``
for a class with multiple bases.

Only one member declaration per line is allowed.

Compile-time complexity of C++ code is proportional to a number of
members and it also increases as a distance between ``NUPY_BEGIN(class)``
and ``NUPY_END()`` increases (even lines with comments count!).
You may need to change a default value of template depth to
compile big structs.

REQUIREMENTS
============

You need a decent C++ compiler that works with boost and that
supports ``__typeof__``.

DEPENDENCIES
============

The library is header-only. It depends on the following boost
libraries (all header-only)

- enable_if
- mpl
- preprocessor
- type_traits

SEE ALSO
========

``snprintf(3)``

BUGS
====

At the moment, the library does not support big endian architectures.

AUTHOR
======

Alexander Nasonov
