==================
Nupy C/C++ Library
==================

-----------------------------
Expose C/C++ structs to numpy
-----------------------------

SYNOPSIS
========

    #define nupyStruct(T) begin a declaration of struct ``T``

    #define nupyEnd()     end a declaration

    #define nupyBase(T)   declare a base class ``T`` - C++ only

    #define nupyM(M)      declare a member ``M``

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

can be "decorated" by ``nupyStruct(Line)`` and ``nupyEnd()``
macros and all members can be wrapped by ``nupyM(m)``

::

	struct Line
	{
		nupyStruct(Line)
		
		double nupyM(start) [2];
		double nupyM(end  ) [2];
		char   nupyM(note ) [16];
		
		nupyEnd()
	};

In plain C ``nupyStruct(T)`` and ``nupyEnd()`` evaluate to nothing
and ``nupyM(M)`` evaluates to ``M`` producing a struct identical
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
a mismatch between a total size of members wrapped by ``nupyM(M)``
and a size of a struct, compile-time assert will be triggered.

Class inheritance is supported with ``nupyBase(T)``. There is no
compile-time check to detect a wrong order of ``nupyBase(T)``
for a class with multiple bases.

Only one member declaration per line is allowed.

Compile-time complexity of C++ code is proportional to a number of
members and it also increases as a distance between ``nupyStruct(T)``
and ``nupyEnd()`` increases (even lines with comments count!).
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
