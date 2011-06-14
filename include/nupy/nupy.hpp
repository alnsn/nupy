/*
 * Copyright 2011 Alexander Nasonov.
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef FILE_nupy_nupy_h_INCLUDED
#define FILE_nupy_nupy_h_INCLUDED

#if !defined(__cplusplus)

#define nupyStruct(C)
/* no nupyBase(C) in nupy-c-api */
#define nupyEnd()
#define nupyM(M) M

#else

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/type_traits/extent.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_signed.hpp>
#include <boost/type_traits/rank.hpp>
#include <boost/type_traits/remove_all_extents.hpp>
#include <boost/type_traits/remove_cv.hpp>
#include <boost/utility/enable_if.hpp>

#define NUPY_ENDIAN_SYM "<"

namespace nupy {

    struct noline {};

    template<int L>
    struct line
    {
        typedef line type;
        enum { value = L };
    };

    template<size_t N>
    struct type_with_size
    {
        typedef char (&type)[N];
    };

    /* use like this: sizeof(member_size(&C::member)) */
    template<class C, class T>
    typename type_with_size<sizeof(T)>::type
    member_size(T C::*);

    template<int L>
    struct line_tester
    {
        static char test(int (*)(line<L>, char *, size_t, size_t), int);
        static type_with_size<2>::type test(int (*)(noline), ...);
    };

    template<class T, int L>
    struct has_line
    {
        enum { value = sizeof(line_tester<L>::test(&T::_nupy_line, 0)) == 1 };
        typedef has_line type;
    };

    template<class T, int L>
    struct next_line
        : boost::mpl::eval_if< has_line<T,(L+1)>
                             , line<(L+1)>
                             , next_line<T,(L+1)>
                             >
    {
    };

    /*
     * cv char[4][2] -> char[2]
     * cv char[2]    -> char[2]
     * cv char       -> char[1]
     */
    template<class T>
    struct basic_str_type
    {
        typedef typename boost::rank<T>::type rank;
        typedef typename boost::extent<T,(rank::value-1)>::type last_extent;
        typedef char type[(last_extent::value ? last_extent::value : 1)];
    };

    /*
     * remove cv-qualifiers and extra extents, e.g.
     * volatile int[4][2] -> int
     * char[4][2][8]      -> char[8]
     */
    template<class T>
    struct basic_type
    {
        typedef typename boost::remove_all_extents<T>::type scalar;
        typedef typename boost::remove_cv<scalar>::type nocv;

        typedef typename boost::mpl::eval_if<
            boost::is_same<nocv,char>,
            basic_str_type<T>,
            boost::mpl::identity<nocv>
            >::type type;
    };

    /*
     * char[16] -> "'|S16'"
     * float[2] -> "'<f4'"
     * T is a result of basic_type<U>
     */
    template<class T, bool FAM, class Enable = void>
    struct typestr
    {
        static int copy(char *buf, size_t bufsz, size_t)
        {
            /* XXX Check that T doesn't have FAM member */
            return T::_nupy_dtype(buf, bufsz, 0, true);
        }
    };

    template<size_t N, bool FAM>
    struct typestr<char[N],FAM,void>
    {
        static int copy(char *buf, size_t bufsz, size_t famsz)
        {
            return snprintf(buf, bufsz, "'|S%zu'", (FAM ? famsz : N));
        }
    };

    template<class T, bool FAM>
    struct typestr< T
                  , FAM
                  , typename boost::enable_if<
                            boost::is_integral<T>
                        >::type
                  >
    {
        static int copy(char *buf, size_t bufsz, size_t)
        {
            return snprintf(buf, bufsz, "'" NUPY_ENDIAN_SYM "%c%zu'",
                boost::is_signed<T>::value ? 'i' : 'u', sizeof(T));
        }
    };

    template<class T, bool FAM>
    struct typestr< T
                  , FAM
                  , typename boost::enable_if<
                            boost::is_floating_point<T>
                        >::type
                  >
    {
        static int copy(char *buf, size_t bufsz, size_t)
        {
            return snprintf(buf, bufsz, "'" NUPY_ENDIAN_SYM "f%zu'", sizeof(T));
        }
    };

    /*
     * remove the last extent from cv char[A]...[Z] (leave other
     * types unchanged) and calculate a rank
     */
    template<class T>
    struct type_rank
    {
        typedef typename boost::remove_all_extents<T>::type scalar;
        typedef typename boost::remove_cv<scalar>::type nocv;

        enum { rank = boost::rank<T>::value };
        enum { ischararray = rank > 0 && boost::is_same<nocv,char>::value };
        enum { value = rank - (ischararray ? 1 : 0) };
    };

    /*
     * copy a shape of T to buf, e.g.
     * shapestr<int[4][2],false>::copy(buf, bufsz, 0)
     * or
     * shapestr<char[4][2][8],false>::copy(buf, bufsz, 0)
     * will copy ",(4,2)" to buf
     */
    template< class T
            , bool FAM                       /* flexible-array member? */
            , size_t N = 0                   /* current dimension      */
            , size_t R = type_rank<T>::value /* remaining dimensions   */
            >
    struct shapestr
    {
        static int copy(char *buf, size_t bufsz, size_t famsz)
        {
            int len, rv = 0;

            if (N == 0) {
                if ((len = snprintf(buf, bufsz, ",(")) < 0)
                    return len;
                rv += len;

                if (len + 0u < bufsz) {
                    buf += len;
                    bufsz -= len;
                } else {
                    bufsz = 0;
                }
            }

            char const *fmt = (R == 1) ? "%zu)" : "%zu,";
            size_t const extent =
                (FAM && N == 0) ? famsz : boost::extent<T,N>::value;
            if ((len = snprintf(buf, bufsz, fmt, extent)) < 0)
                return len;
            rv += len;

            if (len + 0u < bufsz) {
                buf += len;
                bufsz -= len;
            } else {
                bufsz = 0;
            }

            if ((len = shapestr<T,FAM,(N+1),(R-1)>::copy(buf, bufsz, 0)) < 0)
                return len;
            rv += len;

            return rv;
        }
    };

    template<class T, bool FAM, size_t N>
    struct shapestr<T,FAM,N,0>
    {
        static int copy(char *, size_t, size_t)
        {
            return 0;
        }
    };

    /*
     * start (complete == true) or continue (complete == false)
     * a chain of _nupy_line calls from C::nupy_dtype(buf, bufsz, famsz)
     * or from nupyBase(C), respectively
     */
    template<int L, class C>
    int
    dtype(char *buf, size_t bufsz, size_t famsz, bool complete)
    {
        int len, rv = 0;

        /* check for the end early */
        boost::mpl::identity<typename C::_nupy_this> id;
        C::_nupy_end(id);

        if (complete) {
            if ((len = snprintf(buf, bufsz, "[")) < 0)
                return len;
            rv += len;

            if (len + 0u < bufsz) {
                buf += len;
                bufsz -= len;
            } else {
                bufsz = 0;
            }
        }

        typename next_line<C,L>::type next;
        if ((len = C::_nupy_line(next, buf, bufsz, famsz)) < 0)
            return len;
        rv += len;

        /*
         * check that C has some members, empty class
         * should trigger an assert at compile-time:
         */
        assert(len > 0);

        if (complete && len + 0u < bufsz) {
            assert(buf[len - 1] == ',');
            buf[len - 1] = ']';
        }

        return rv;
    }

    /* continue a chain of _nupy_line calls from nupyBase(B) */
    template<int L, class C, class B>
    inline int
    base(char *buf, size_t bufsz)
    {
        int len, rv = 0;

        if ((len = B::_nupy_dtype(buf, bufsz, 0, false)) < 0)
            return len;
        rv += len;

        if (len + 0u < bufsz) {
            buf += len;
            bufsz -= len;
        } else {
            bufsz = 0;
        }

        typename next_line<C,L>::type next;
        if ((len = C::_nupy_line(next, buf, bufsz, 0)) < 0)
            return len;
        rv += len;

        return rv;
    }

    /* helper for _nupy_size */
    template<size_t Sz, int L, class C>
    inline void
    next_size()
    {
        typename next_line<C,L>::type next;
        C::template _nupy_size<Sz>(next);
    }

    template<class C, int L, bool FAM, /* deduced: */ class T>
    inline int
    member(T C::*, char const *name, char *buf, size_t bufsz, size_t famsz)
    {
        typedef typename boost::extent<T,0>::type e0;

        BOOST_MPL_ASSERT_RELATION(e0::value >= 1 || !FAM, ==, true);

        int len, rv = 0;

        if ((len = snprintf(buf, bufsz, "('%s',", name)) < 0)
            return len;
        rv += len;

        if (len + 0u < bufsz) {
            buf += len;
            bufsz -= len;
        } else {
            bufsz = 0;
        }

        typedef typename basic_type<T>::type basic;
        if ((len = typestr<basic,FAM>::copy(buf, bufsz, famsz)) < 0)
            return len;
        rv += len;

        if (len + 0u < bufsz) {
            buf += len;
            bufsz -= len;
        } else {
            bufsz = 0;
        }

        if ((len = shapestr<T,FAM>::copy(buf, bufsz, famsz)) < 0)
            return len;
        rv += len;

        if (len + 0u < bufsz) {
            buf += len;
            bufsz -= len;
        } else {
            bufsz = 0;
        }

        if ((len = snprintf(buf, bufsz, "),")) < 0)
            return len;
        rv += len;

        if (len + 0u < bufsz) {
            buf += len;
            bufsz -= len;
        } else {
            bufsz = 0;
        }

        typename next_line<C,L>::type next;
        if ((len = C::_nupy_line(next, buf, bufsz, famsz)) < 0)
            return len;
        rv += len;

        return rv;
    }
}

#define nupyStruct(C) \
    typedef C _nupy_this; static int _nupy_line( ::nupy::noline );       \
    static int                                                           \
    _nupy_dtype(char *buf, size_t bufsz, size_t famsz, bool complete)    \
    { ::nupy::next_size<0,__LINE__,C>();                                 \
      return ::nupy::dtype<__LINE__,C>(buf, bufsz, famsz, complete); }   \
    static int                                                           \
    nupy_dtype(char *buf, size_t bufsz, size_t famsz = 0)                \
    { return _nupy_dtype(buf, bufsz, famsz, true); }

#define nupyBase(C) \
    static int                                                           \
    _nupy_line( ::nupy::line<__LINE__>, char *buf, size_t bufsz, size_t) \
    { return ::nupy::base<__LINE__,_nupy_this,C>(buf, bufsz); }          \
    template<size_t _nupySz>                                             \
    static void _nupy_size( ::nupy::line<__LINE__> )                     \
    { ::nupy::next_size<(sizeof(C)+_nupySz),__LINE__,_nupy_this>(); }

#define nupyM(M) \
    static BOOST_PP_CAT(_nupy_member_,__LINE__)();                             \
    static int                                                                 \
    _nupy_line( ::nupy::line<__LINE__>, char *buf, size_t bufsz, size_t famsz) \
    { return ::nupy::member<_nupy_this,__LINE__,false>( &_nupy_this::M, #M,    \
                 buf, bufsz, famsz); }                                         \
    template<size_t _nupySz>                                                   \
    static void _nupy_size( ::nupy::line<__LINE__> )                           \
    { enum { _nupy_msz = sizeof(::nupy::member_size(&_nupy_this::M)) };        \
      ::nupy::next_size<(_nupySz+_nupy_msz),__LINE__,_nupy_this>(); }          \
    __typeof__(_nupy_this::BOOST_PP_CAT(_nupy_member_,__LINE__)()) M

#define nupyFAM(M) \
    static BOOST_PP_CAT(_nupy_member_,__LINE__)();                             \
    static int                                                                 \
    _nupy_line( ::nupy::line<__LINE__>, char *buf, size_t bufsz, size_t famsz) \
    { return ::nupy::member<_nupy_this,__LINE__,true>( &_nupy_this::M, #M,     \
                 buf, bufsz, famsz); }                                         \
    template<size_t _nupySz>                                                   \
    static void _nupy_size( ::nupy::line<__LINE__> )                           \
    { enum { _nupy_msz = sizeof(::nupy::member_size(&_nupy_this::M)) };        \
      ::nupy::next_size<(_nupySz+_nupy_msz),__LINE__,_nupy_this>();            \
      BOOST_MPL_ASSERT_RELATION(_nupySz+_nupy_msz,==,sizeof(_nupy_this)); }    \
    __typeof__(_nupy_this::BOOST_PP_CAT(_nupy_member_,__LINE__)()) M

#define nupyEnd() \
    static void _nupy_end( ::boost::mpl::identity<_nupy_this> ) {} \
    static int                                                     \
    _nupy_line( ::nupy::line<__LINE__>, char *buf, size_t, size_t) \
    { return 0; }                                                  \
    template<size_t _nupySz>                                       \
    static void _nupy_size( ::nupy::line<__LINE__> )               \
    { BOOST_MPL_ASSERT_RELATION(_nupySz,==,sizeof(_nupy_this)); }

#endif /* #if !defined(__cplusplus) */

#endif /* #ifndef FILE_nupy_nupy_h_INCLUDED */
