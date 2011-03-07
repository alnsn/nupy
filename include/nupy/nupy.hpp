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

#define NUPY_BEGIN(C)
#define NUPY_END()
#define NUPY_MEMBER(M) M

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

    template<class C, class T>
    typename type_with_size<sizeof(T)>::type
    member_size(T C::*);

    template<int L>
    struct line_tester
    {
        static char test(int (*)(line<L>, char*, size_t), int);
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
     * Remove cv-qualifiers and extra extents, e.g.
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

    /* char[8] -> "'|S8'", T is a result of basic_type<U> */
    template<class T, class Enable = void>
    struct typestr
    {
        static int copy(char* buf, size_t bufsz)
        {
            return T::_nupy_dtype(buf, bufsz, true);
        }
    };

    template<size_t N>
    struct typestr<char[N],void>
    {
        static int copy(char* buf, size_t bufsz)
        {
            return snprintf(buf, bufsz, "'|S%zu'", N);
        }
    };

    template<class T>
    struct typestr< T
                  , typename boost::enable_if<
                            boost::is_integral<T>
                        >::type
                  >
    {
        static int copy(char* buf, size_t bufsz)
        {
            return snprintf(buf, bufsz, "'" NUPY_ENDIAN_SYM "%c%zu'",
                boost::is_signed<T>::value ? 'i' : 'u', sizeof(T));
        }
    };

    template<class T>
    struct typestr< T
                  , typename boost::enable_if<
                            boost::is_floating_point<T>
                        >::type
                  >
    {
        static int copy(char* buf, size_t bufsz)
        {
            return snprintf(buf, bufsz, "'" NUPY_ENDIAN_SYM "%cf%zu'",
                sizeof(T));
        }
    };

    /* Remove the last extent from cv char[A]...[Z] and calculate a rank */
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
     * Copy a shape of T to buf, e.g. int[4][2] and char[4][2][8]
     * will copy ",(4,2)" to buf
     */
    template< class T
            , size_t N = 0                   /* Current dimension    */
            , size_t R = type_rank<T>::value /* Remaining dimensions */
            >
    struct shapestr
    {
        static int copy(char* buf, size_t bufsz)
        {
            int len, rv = 0;

            if(N == 0) {
                if((len = snprintf(buf, bufsz, ",(")) < 0)
                    return len;
                rv += len;

                if(len + 0u < bufsz) {
                    buf += len;
                    bufsz -= len;
                } else {
                    bufsz = 0;
                }
            }

            char terminator = (R == 1) ? ')' : ',';
            const size_t extent = boost::extent<T,N>::value;
            if((len = snprintf(buf, bufsz, "%zu%c", extent, terminator)) < 0)
                return len;
            rv += len;

            if(len + 0u < bufsz) {
                buf += len;
                bufsz -= len;
            } else {
                bufsz = 0;
            }

            if((len = shapestr<T,(N+1),(R-1)>::copy(buf, bufsz)) < 0)
                return len;
            rv += len;

            return rv;
        }
    };

    template<class T, size_t N>
    struct shapestr<T,N,0>
    {
        static int copy(char* buf, size_t bufsz)
        {
            return 0;
        }
    };

    /* C class declaration starts at line L */
    template<int L, class C>
    int
    dtype(char* buf, size_t bufsz, bool complete)
    {
        int len, rv = 0;

        /* check for the end early */
        boost::mpl::identity<typename C::_nupy_this> id;
        C::_nupy_end(id);

        if(complete) {
            if((len = snprintf(buf, bufsz, "[")) < 0)
                return len;
            rv += len;

            if(len + 0u < bufsz) {
                buf += len;
                bufsz -= len;
            } else {
                bufsz = 0;
            }
        }

        typename next_line<C,L>::type next;
        if((len = C::_nupy_line(next, buf, bufsz)) < 0)
            return len;
        rv += len;

        /*
         * check that C has some members, empty class
         * should trigger an assert at compile-time:
         */
        assert(len > 0);

        if(complete && len + 0u < bufsz) {
            assert(buf[len - 1] == ',');
            buf[len - 1] = ']';
        }

        return rv;
    }

    template<int L, class C, class B>
    inline int
    base(char* buf, size_t bufsz)
    {
        int len, rv = 0;
        
        if((len = B::_nupy_dtype(buf, bufsz, false)) < 0)
            return len;
        rv += len;

        if(len + 0u < bufsz) {
            buf += len;
            bufsz -= len;
        } else {
            bufsz = 0;
        }

        typename next_line<C,L>::type next;
        if((len = C::_nupy_line(next, buf, bufsz)) < 0)
            return len;
        rv += len;

        return rv;
    }

    template<int L, class C, class T>
    inline int
    member(T C::*, char const* name, char* buf, size_t bufsz)
    {
        int len, rv = 0;
        
        if((len = snprintf(buf, bufsz, "('%s',", name)) < 0)
            return len;
        rv += len;

        if(len + 0u < bufsz) {
            buf += len;
            bufsz -= len;
        } else {
            bufsz = 0;
        }

        typedef typename basic_type<T>::type basic;
        if((len = typestr<basic>::copy(buf, bufsz)) < 0)
            return len;
        rv += len;

        if(len + 0u < bufsz) {
            buf += len;
            bufsz -= len;
        } else {
            bufsz = 0;
        }

        if((len = shapestr<T>::copy(buf, bufsz)) < 0)
            return len;
        rv += len;

        if(len + 0u < bufsz) {
            buf += len;
            bufsz -= len;
        } else {
            bufsz = 0;
        }

        if((len = snprintf(buf, bufsz, "),")) < 0)
            return len;
        rv += len;

        if(len + 0u < bufsz) {
            buf += len;
            bufsz -= len;
        } else {
            bufsz = 0;
        }

        typename next_line<C,L>::type next;
        if((len = C::_nupy_line(next, buf, bufsz)) < 0)
            return len;
        rv += len;

        return rv;
    }
}

#define NUPY_BEGIN(C) \
    typedef C _nupy_this; static int _nupy_line( ::nupy::noline ); \
    static int _nupy_dtype(char* buf, size_t bufsz, bool complete) \
    { C::_nupy_size<0>( ::nupy::next_line<C,__LINE__>::type() );   \
      return ::nupy::dtype<__LINE__,C>(buf, bufsz, complete); }    \
    static int nupy_dtype(char* buf, size_t bufsz)                 \
    { return _nupy_dtype(buf, bufsz, true); }

#define NUPY_BASE(C) \
    static int                                                     \
    _nupy_line( ::nupy::line<__LINE__> l, char* buf, size_t bufsz) \
    { return ::nupy::base<__LINE__,_nupy_this,C>(buf, bufsz); }    \
    template<size_t _nupySz>                                       \
    static void _nupy_size( ::nupy::line<__LINE__> )               \
    { _nupy_this::_nupy_size<(sizeof(C) + _nupySz)>(               \
        ::nupy::next_line<_nupy_this,__LINE__>::type() ); }

#define NUPY_MEMBER(M) \
    static BOOST_PP_CAT(_nupy_member_,__LINE__)();                 \
    static int                                                     \
    _nupy_line( ::nupy::line<__LINE__> l, char* buf, size_t bufsz) \
    { return ::nupy::member<__LINE__,_nupy_this>(                  \
        &_nupy_this::M, #M, buf, bufsz); }                         \
    template<size_t _nupySz>                                       \
    static void _nupy_size( ::nupy::line<__LINE__> )               \
    { _nupy_this::_nupy_size<(_nupySz +                            \
        sizeof(::nupy::member_size(&_nupy_this::M)))>(             \
        ::nupy::next_line<_nupy_this,__LINE__>::type() ); }        \
    __typeof__(_nupy_this::BOOST_PP_CAT(_nupy_member_,__LINE__)()) \
    M

#define NUPY_END() \
    static void _nupy_end( ::boost::mpl::identity<_nupy_this> )    \
    {}                                                             \
    static int                                                     \
    _nupy_line( ::nupy::line<__LINE__>, char* buf, size_t bufsz)   \
    { return 0; }                                                  \
    template<size_t _nupySz>                                       \
    static void _nupy_size( ::nupy::line<__LINE__> )               \
    { BOOST_MPL_ASSERT_RELATION(_nupySz,==,sizeof(_nupy_this)); }

#endif /* #if !defined(__cplusplus) */

#endif /* #ifndef FILE_nupy_nupy_h_INCLUDED */
