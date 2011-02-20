#ifndef FILE_nupy_nupy_h_INCLUDED
#define FILE_nupy_nupy_h_INCLUDED

#include <stddef.h>

#if !defined(__cplusplus)

#define NUPY_BEGIN(C)
#define NUPY_END()
#define NUPY_BASE(C)
#define NUPY_MEMBER(M) M

#else

#include <assert.h>
#include <stdio.h>

#include <boost/mpl/eval_if.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/utility/enable_if.hpp>

#define NUPY_ENDIAN_SYM '<'

namespace nupy {

    struct noline {};

    template<int L>
    struct line
    {
        typedef line type;
        enum { value = L };
    };

    template<class T>
    struct add_noline : T
    {
        using T::_nupy_line;
        void _nupy_line(noline);
    };

    template<class L>
    struct line_tester
    {
        BOOST_STATIC_ASSERT(sizeof(long) != sizeof(char));

        template<class T>
        static char test(void (T::*)(noline), ...);

        template<class T>
        static long test(void (T::*)(L), int);
    };

    template<class T, class L>
    struct has_line
    {
        enum
        {
            value = sizeof(
                line_tester<L>::test(&add_noline<T>::_nupy_line, 0)) != 1
        };
        typedef has_line type;
    };

    template<class T, class L>
    struct next_line
        : boost::mpl::eval_if<
            has_line< T, line<(L::value+1)> >,
            line<(L::value+1)>,
            next_line< T, line<(L::value+1)> >
        >
    {
    };

    template<class C, class T>
    inline typename boost::enable_if< boost::is_integral<T>, int >::type
    dtype_type(T C::*, char* buf, size_t sz)
    {
        return snprintf(buf, sz, "'%ci%d'", NUPY_ENDIAN_SYM, sizeof(T));
    }

    template<class C, class T>
    inline typename boost::enable_if< boost::is_floating_point<T>, int >::type
    dtype_type(T C::*, char* buf, size_t sz)
    {
        return snprintf(buf, sz, "'%cf%d'", NUPY_ENDIAN_SYM, sizeof(T));
    }

    template<class C, int N>
    inline int
    dtype_type(char (C::*)[N], char* buf, size_t sz)
    {
        return snprintf(buf, sz, "'|S%d'", N);
    }

    /* T class declaration starts at line L */
    template<class T, int L>
    int
    dtype_begin(char* buf, size_t sz, size_t varlen)
    {
        int len = snprintf(buf, sz, "%c", '[');

        if(len < 0) {
            return len;
        } else if(len < sz) {
            buf += len;
            sz  -= len;
        } else {
            buf = NULL;
            sz  = 0;
        }

        /* check for the end early */
        BOOST_STATIC_ASSERT(T::_nupy_end > L);

        typename next_line< T, line<L> >::type next;
        int tail = T::_nupy_do(next, buf, sz, varlen);
        return tail < 0 ? tail : len + tail;
    }

    template<class C, class T, int L>
    inline int
    dtype_member( line<L>, T C::* m, char const* member,
                  char* buf, size_t sz, size_t varlen )
    {
        int len1 = snprintf(buf, sz, "('%s',", member);

        if(len1 < 0) {
            return len1;
        } else if(len1 < sz) {
            buf += len1;
            sz  -= len1;
        } else {
            buf = NULL;
            sz  = 0;
        }

        int len2 = dtype_type(m, buf, sz);

        if(len2 < 0) {
            return len2;
        } else if(len2 < sz) {
            buf += len2;
            sz  -= len2;
        } else {
            buf = NULL;
            sz  = 0;
        }

        int len3 = snprintf(buf, sz, "),");

        if(len3 < 0) {
            return len3;
        } else if(len3 < sz) {
            buf += len3;
            sz  -= len3;
        } else {
            buf = NULL;
            sz  = 0;
        }

        int len = len1 + len2 + len3;

        typename next_line< C, line<L> >::type next;
        int tail = C::_nupy_do(next, buf, sz, varlen);
        return tail < 0 ? tail : len + tail;
    }

    inline int
    dtype_end(char* buf, size_t sz)
    {
        int len = 0;

        if(buf == NULL)
            return 0;
        
        if(*(buf - 1) == ',') {
            *(buf - 1) = ']';
        } else {
            assert(*(buf - 1) == '['); /* dtype == "[]" */
            len = snprintf(buf, sz, "%c", ']');
        }
        
        return len;
    }
}

#define NUPY_BEGIN(C) \
    typedef C _nupy_this; \
    static int nupy_dtype(char* buf, size_t sz, size_t varlen = 0 ) \
    { return ::nupy::dtype_begin<C,__LINE__>(buf, sz, varlen); }

#define NUPY_MEMBER(M) static BOOST_PP_CAT(_nupy_member_,__LINE__)(); \
    void _nupy_line( ::nupy::line<__LINE__> ); \
    static int \
    _nupy_do( ::nupy::line<__LINE__> l, char* buf, size_t sz, size_t varlen) \
    { return ::nupy::dtype_member(l, &_nupy_this::M, #M, buf, sz, varlen); } \
    __typeof__(_nupy_this::BOOST_PP_CAT(_nupy_member_,__LINE__)()) M

#define NUPY_END() enum { _nupy_end = __LINE__ }; \
    void _nupy_line( ::nupy::line<__LINE__> ); \
    static int \
    _nupy_do( ::nupy::line<__LINE__>, char* buf, size_t sz, size_t) \
    { return ::nupy::dtype_end(buf, sz); }

#endif /* #if !defined(__cplusplus) */

#endif /* #ifndef FILE_nupy_nupy_h_INCLUDED */
