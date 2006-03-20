/***************************************************************************
 *
 * 21.string.replace.cpp - string test exercising [lib.string::replace]
 *
 * $Id$ 
 *
 ***************************************************************************
 *
 * Copyright 2006 The Apache Software Foundation or its licensors,
 * as applicable.
 *
 * Copyright 2006 Rogue Wave Software.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 **************************************************************************/

#include <string>       // for string
#include <cstddef>      // for size_t
#include <stdexcept>    // for out_of_range, length_error

#include <cmdopt.h>     // for rw_enabled()
#include <driver.h>     // for rw_test()

#include <rw_char.h>    // for rw_widen()
#include <alg_test.h>   // for InputIter<>

/**************************************************************************/

struct MemFun
{
    enum charT  { Char, WChar, UChar };
    enum Traits { DefaultTraits, UserTraits };

    MemFun (charT cid, const char *cname,
          Traits tid, const char *tname)
        : cid_ (cid), tid_ (tid), 
          cname_ (cname), tname_ (tname), aname_ ("allocator"),
          fname_ ("replace") { /* empty */ }

    charT       cid_;     // character type id (char or wchar_t)
    Traits      tid_;     // traits type id (default or user-defined)
    const char *cname_;   // character type name
    const char *tname_;   // traits name
    const char *aname_;   // allocator name
    const char *fname_;   // function name
};

/**************************************************************************/

static const int long_string_len = 4096;
static char long_string [long_string_len];

static const int long_parity_len = (long_string_len - 1) & 0x01 ? 
           (long_string_len - 1) / 2 : (long_string_len - 2) / 2;

static const char* exp_exceptions[] = 
    { "unknown exception", "out_of_range", "length_error" };

/**************************************************************************/

static const struct TestCase {

    int  line;

    int  pos1;
    int  num1;
    int  pos2;
    int  num2;
    int  cnt;
    char ch;

    char* str;
    std::size_t str_len;

    char* src;
    std::size_t src_len;

    char* res;
    std::size_t res_len;

    char* it_res;
    std::size_t it_res_len;

    int bthrow;

} test_cases [] = {

#undef TEST
#define TEST(str, pos1, num1, src, pos2, num2, cnt, ch, res, it_res, bthrow) \
    { __LINE__, pos1, num1, pos2, num2, cnt, ch, (char*) str,                \
      sizeof str - 1, (char*) src, sizeof src - 1, (char*) res,              \
      sizeof res - 1, (char*) it_res, sizeof it_res - 1, bthrow }

    //    +----------------------------------------- controlled sequence
    //    |      +---------------------------------- replace() pos1 argument
    //    |      |  +------------------------------- replace() n1 argument
    //    |      |  |  +---------------------------- replace with sequence
    //    |      |  |  |          +----------------- replace() pos2 argument 
    //    |      |  |  |          |  +-------------- replace() n2 argument 
    //    |      |  |  |          |  |  +----------- replace() count argument 
    //    |      |  |  |          |  |  |  +-------- replace() with character
    //    |      |  |  |          |  |  |  |    +--- expected result sequence
    //    |      |  |  |          |  |  |  |    |  + expected result sequence 
    //    |      |  |  |          |  |  |  |    |  | for iterators
    //    |      |  |  |          |  |  |  |    |  |    exception info ----+ 
    //    |      |  |  |          |  |  |  |    |  |    0 - no exception   |   
    //    |      |  |  |          |  |  |  |    |  |    1,2 - out_of_range |   
    //    |      |  |  |          |  |  |  |    |  |    3 - length_error   |   
    //    |      |  |  |          |  |  |  |    |  |                       |
    //    |      |  |  |          |  |  |  |    |  +----------+            |
    //    V      V  V  V          V  V  V  V    V             V            V
    TEST ("ab",  0, 1, "c",       0, 1, 2, 'c', "cb",         "cb",        0),

    TEST ("",    0, 0, "",        0, 0, 0, 'c', "",           "",          0),
    TEST ("",    0, 0, "abc",     0, 3, 3, 'c', "abc",        "abc",       0),

    TEST ("ab",  0, 2, "",        0, 0, 0, 'c', "",           "",          0),
    TEST ("ab",  0, 1, "",        0, 0, 0, 'c', "b",          "b",         0),

    TEST ("\0",  0, 1, "",        0, 0, 0, ' ', "",           "",          0),
    TEST ("\0",  0, 1, "a",       0, 1, 1, 'a', "a",          "a",         0),
    TEST ("\0",  0, 1, "\0\0",    1, 1, 1, '\0', "\0",        "\0",        0),
    TEST ("\0",  0, 1, "\0\0",    0, 2, 2, '\0', "\0\0",      "\0\0",      0),

    TEST ("ah",  0, 1, "bcdefg",  0, 3, 1, 'c',  "bcdh",      "bcdefgh",   0),
    TEST ("ah",  1, 1, "bcdefg",  0, 3, 1, 'c',  "abcd",      "abcdefg",   0),
    TEST ("ah",  0, 1, "bcdefg",  1, 3, 4, 'c',  "cdeh",      "bcdefgh",   0),
    TEST ("ah",  1, 1, "bcdefg",  1, 3, 4, 'c',  "acde",      "abcdefg",   0),
    TEST ("ah",  0, 1, "bcdefg",  0, 6, 1, 'c',  "bcdefgh",   "bcdefgh",   0),
    TEST ("ah",  1, 1, "bcdefg",  0, 6, 1, 'c',  "abcdefg",   "abcdefg",   0),

    TEST ("abc", 0, 2, "cc",      0, 2, 2, 'c',  "ccc",       "ccc",       0),
    TEST ("abc", 1, 2, "cc",      0, 2, 2, 'c',  "acc",       "acc",       0),

    TEST ("abc", 0, 3, "d",       0, 1, 1, 'c',  "d",         "d",         0),
    TEST ("abc", 0, 3, "def",     0, 3, 3, 'c',  "def",       "def",       0),
    TEST ("abc", 0, 3, "defgh",   0, 5, 5, 'c',  "defgh",     "defgh",     0),
    TEST ("abc", 2, 1, "defgh",   4, 1, 1, 'c',  "abh",       "abdefgh",   0),
    TEST ("abc", 2, 1, "de\0gh",  2, 1, 1, 'c',  "ab\0",      "abde\0gh",  0),
    TEST ("abc", 2, 1, "",        0, 0, 0, 'c',  "ab",        "ab",        0),

    TEST ("abc", 1, 1, "defgh",   1, 2, 5, 'c',  "aefc",      "adefghc",   0),
    TEST ("abc", 0, 0, "ee",      0, 2, 2, 'c',  "eeab",      "eeab",      0),
    TEST ("abc", 0, 0, "\0\0e\0", 0, 4, 4, '\0', "\0\0e\0ab", "\0\0e\0ab", 0),
    TEST ("abc", 2, 0, "ee",      0, 2, 2, 'c',  "abee",      "abee",      0),
    TEST ("abc", 2, 0, "\0e\0\0", 0, 4, 4, '\0', "ab\0e\0\0", "ab\0e\0\0", 0),
    TEST ("abc", 1, 0, "ee",      0, 2, 2, 'c',  "aeebc",     "aeebc",     0),

    TEST ("abc", 1, 0, "\0e\0\0", 0, 4, 4, '\0', "a\0e\0\0bc", 
                                                             "a\0e\0\0bc", 0),

    TEST ("a\0b\0\0c", 0, 3, "",    0, 0, 1, '\0', "\0\0c",    "\0\0c",    0),
    TEST ("\0ab\0\0c", 0, 3, "",    0, 0, 1, '\0', "\0\0c",    "\0\0c",    0),
    TEST ("a\0b\0\0c", 0, 3, "\0e", 0, 2, 1, '\0', "\0e\0\0c", "\0e\0\0c", 0),

    TEST ("a\0b\0\0c", 2, 3, "\0e", 0, 2, 1, '\0', "a\0\0ec",  "a\0\0ec",  0),
    TEST ("a\0b\0\0c", 2, 3, "\0e", 1, 1, 1, '\0', "a\0ec",    "a\0\0ec",  0),
    TEST ("\0ab\0\0c", 2, 3, "\0e", 0, 2, 1, '\0', "\0a\0ec",  "\0a\0ec",  0),
    TEST ("\0ab\0\0c", 2, 3, "\0e\0\0", 
                                    1, 3, 5, '\0', "\0ae\0\0c", 
                                                            "\0a\0e\0\0c", 0),

    TEST ("a\0b\0\0c", 0, 6, "\0e", 0, 2, 2, '\0', "\0e",      "\0e",      0),
    TEST ("a\0b\0\0c", 0, 6, "\0e", 0, 1, 2, '\0', "\0",       "\0e",      0),

    TEST ("\0ab\0\0c", 0, 0, "\0e", 0, 2, 2, '\0', "\0e\0ab\0\0c", 
                                                           "\0e\0ab\0\0c", 0),
    TEST ("a\0b\0c\0", 6, 0, "e\0", 0, 2, 2, '\0', "a\0b\0c\0e\0", 
                                                           "a\0b\0c\0e\0", 0),
    TEST ("\0ab\0\0c", 5, 0, "\0e", 0, 2, 2, '\0', "\0ab\0\0\0ec", 
                                                           "\0ab\0\0\0ec", 0),

    TEST (long_string, 0, long_string_len - 1, "ab", 
                                      0, 2, 2, 'a',  "ab",      "ab",      0),
    TEST (long_string, 1, long_string_len - 2, "ab", 
                                      0, 2, 2, 'a',  "xab",     "xab",     0),
    TEST (long_string, 0, long_string_len - 2, "ab", 
                                      0, 2, 2, 'a',  "abx",     "abx",     0),
    TEST (long_string, 1, long_string_len - 3, "", 
                                      0, 0, 0, 'a',  "xx",      "xx",      0),
    TEST (long_string, 1, long_string_len - 4, "\0\0", 
                                      0, 2, 1, '\0', "x\0\0xx", "x\0\0xx", 0),

    TEST ("a", 0, 1, long_string, 0, long_string_len - 1, long_string_len - 1, 
                                            'a', long_string, long_string, 0),

    TEST (long_string, 0, long_string_len - 1, long_string, 0, 
                                 long_string_len - 1, long_string_len - 1, 
                                            'a', long_string, long_string, 0),

    TEST (long_string, 0, long_parity_len, long_string, 0, 
                                 long_parity_len, long_parity_len, 
                                            'a', long_string, long_string, 0),

    TEST (long_string, long_parity_len, long_string_len - 1 - long_parity_len, 
       long_string, 0, long_string_len - 1 - long_parity_len, 
                       long_string_len - 1 - long_parity_len, 
                                            'a', long_string, long_string, 0),

#ifndef _RWSTD_NO_EXCEPTIONS

    TEST ("\0",        2, 0, "",    0, 0, 0, ' ',  "",        "",          1),
    TEST ("\0",        0, 0, "\0",  2, 0, 0, ' ',  "",        "",          2),

    TEST ("a",        10, 0, "",    0, 0, 0, ' ',  "",        "",          1),
    TEST ("a",         0, 0, "a",  10, 0, 0, ' ',  "",        "",          2),

    TEST (long_string, long_string_len + 10, 0, "", 0, 0, 0,  ' ', "", "", 1),
    TEST (long_string, 0, 0, long_string, long_string_len + 10, 
                                                       0, 0,  ' ', "", "", 2),

#endif   // _RWSTD_NO_EXCEPTIONS

    TEST ("last",  0, 4, "test",    0, 4, 4, 't',  "test",    "test",      0)
};

/**************************************************************************/

template <class String>
String& test_replace (int                          which,
                      const TestCase              &cs,
                      String                      &str, 
                      typename String::value_type *wsrc, 
                      bool                         test,
                      std::size_t                 *exp_len)
{
    *exp_len = 0;

    if (!test && (4 == which || 7 == which))
        return str;

    const String& ref_src = String (wsrc, cs.src_len);

    // construct iterators
    int first = std::size_t (cs.pos1) > str.size () ? 0 : cs.pos1;
    int last  = cs.pos1 + cs.num1;

    typename String::iterator it_first (str.begin () + first);

    typename String::iterator it_last (std::size_t (last) >= cs.str_len ?
        str.end () : str.begin () + last);

    switch (which) {
        case 1: {
            if (test)
                return str.replace (cs.pos1, cs.num1, wsrc);
            else
                return str.replace (cs.pos1, cs.num1, String (wsrc));
        }
        case 2: {
            if (test)
                return str.replace (cs.pos1, cs.num1, ref_src);
            else
                return str.replace (cs.pos1, cs.num1, ref_src, 
                                    0, String::npos);
        }
        case 3: {
            if (test)
                return str.replace (cs.pos1, cs.num1, wsrc, cs.num2);
            else
                return str.replace (cs.pos1, cs.num1, String (wsrc, cs.num2));
        }
        case 4: {
            if (test)
                return str.replace (cs.pos1, cs.num1, ref_src, 
                                    cs.pos2, cs.num2);
            else
                return str;
        }
        case 5: {
            if (test)
                return str.replace (cs.pos1, cs.num1, cs.cnt, make_char (
                                    cs.ch, (typename  String::value_type*)0));
            else
                return str.replace (cs.pos1, cs.num1, 
                                    String (cs.cnt, make_char (cs.ch, 
                                    (typename String::value_type*)0)));
        }
        case 6: {
            *exp_len = str.length () + String::traits_type::length (wsrc) -
               (it_last - it_first);

            if (test)
                return str.replace (it_first, it_last, wsrc);
            else
                return str.replace (it_first, it_last, String (wsrc));
        }
        case 7: {
            *exp_len = str.length () + ref_src.length () - 
               (it_last - it_first);

            if (test)
                return str.replace (it_first, it_last, ref_src);
            else
                return str;
        }
        case 8: {
            *exp_len = str.length () + cs.num2 - (it_last - it_first);

            if (test)
                return str.replace (it_first, it_last, wsrc, cs.num2);
            else
                return str.replace (it_first, it_last, 
                                    String (wsrc, cs.num2));
        }
        case 9: {
            *exp_len = str.length () + cs.cnt - (it_last - it_first);

            if (test)
                return str.replace (it_first, it_last, cs.cnt, 
                                    make_char (cs.ch, 
                                    (typename String::value_type*)0));
            else
                return str.replace (it_first, it_last, 
                                    String (cs.cnt, make_char (cs.ch, 
                                    (typename String::value_type*)0)));
        }
        case 10: {
            // construct iterators
            int last2 = cs.pos2 + cs.num2;

            InputIter<typename String::value_type> src_first = 
                make_iter (wsrc + cs.pos2, wsrc + cs.pos2, wsrc + last2, 
                        InputIter<typename String::value_type> (0, 0, 0));

            InputIter<typename String::value_type> src_last  = 
                make_iter (wsrc + last2, wsrc + cs.pos2, wsrc + last2, 
                        InputIter<typename String::value_type> (0, 0, 0));

            *exp_len = str.length () + (src_last.cur_ - src_first.cur_) - 
                (it_last - it_first);

            if (test)
                return str.replace (it_first, it_last, src_first, src_last);
            else
                return str.replace (it_first, it_last, 
                                    String (src_first, src_last));
        }
    }

    return str;
}

/**************************************************************************/

template <class charT, class Traits>
void test_replace (charT, Traits*, 
                   int             which,
                   MemFun         *pfid, 
                   const TestCase &cs)
{
    typedef std::basic_string <charT, Traits, 
                               std::allocator<charT> > TestString;

    static charT wstr [long_string_len];
    static charT wsrc [long_string_len];
    static charT wres [long_string_len];

    // construct strings
    bool use_res = (4 == which || 7 == which);
    bool use_iters = (6 <= which);

    char* pstrres = use_res ? 
        use_iters ? cs.it_res : cs.res 
      : cs.str;

    std::size_t res_len = use_res ? 
        use_iters ? cs.it_res_len : cs.res_len 
      : cs.str_len;

    rw_widen (wstr, cs.str, cs.str_len);
    rw_widen (wsrc, cs.src, cs.src_len);
    rw_widen (wres, pstrres, res_len);

    TestString s_str (wstr, cs.str_len);
    TestString s_src (wsrc, cs.src_len);
    TestString s_res (wres, res_len);

    // used to workaround the problem with empty UserChar strings output
    static charT wempty [10]; 
    rw_widen (wempty, "<empty>", 7);
    TestString s_empty (wempty, 7);
    bool src_use_empty = 
        0 == s_src.size () && sizeof (charT) > sizeof (wchar_t);

    int first1_off = cs.pos1;
    int last1_off  = cs.pos1 + cs.num1; 

    int first2_off = cs.pos2;
    int last2_off  = cs.pos2 + cs.num2;

#ifndef _RWSTD_NO_EXCEPTIONS

    // is some exception expected ?
    const char* expected = 0;
    if (!use_iters) {
        if (1 == cs.bthrow || 2 == cs.bthrow && 4 == which)
            expected = exp_exceptions[1];
        else if (3 == cs.bthrow)
            expected = exp_exceptions[2];
    }

    const char* caught = 0;

    try {

#endif   // _RWSTD_NO_EXCEPTIONS

    // do replace
    std::size_t exp_len = 0;
    std::size_t dummy_len = 0;

    TestString& res_str = 
        test_replace (which, cs, s_str, wsrc, true, &exp_len);
    TestString& ctl_str = 
        test_replace (which, cs, s_res, wsrc, false, &dummy_len);

#define CALLFMAT                                                             \
    "line %d. std::basic_string<%s, %s<%2$s>, %s<%2$s>>(%{#*s})."            \
    "replace (%{?}%zu%{;}%{?}begin + %zu%{;}"                                \
    "%{?}, %zu%{;}%{?}, begin + %zu%{;}"                                     \
    "%{?}, %{#*s}%{;}%{?}, %{#*S}%{;}"                                       \
    "%{?}, %zu%{;}%{?}, %zu%{;}"                                             \
    "%{?}, %zu%{;}%{?}, %#c%{;}"                                             \
    "%{?}, begin + %zu%{;}%{?}, begin + %zu%{;})"

#define CALLARGS                                                             \
    __LINE__, pfid->cname_, pfid->tname_, pfid->aname_, int (cs.str_len),    \
    cs.str, 5 >= which, cs.pos1, 6 <= which, first1_off,                     \
    5 >= which, cs.num1, 6 <= which, last1_off,                              \
    1 == which || 3 == which || 6 == which || 8 == which,                    \
    int (cs.src_len), cs.src, 2 == which || 4 == which || 7 == which,        \
    int (sizeof (charT)), src_use_empty ? &s_empty : &s_src,                 \
    5 == which || 9 == which, cs.cnt, 4 == which, cs.pos2,                   \
    3 == which || 4 == which || 8 == which, cs.num2,                         \
    5 == which || 9 == which, cs.ch, 10 == which, first2_off,                \
    10 == which, last2_off

    // verify the results
    if (4 == which || 7 == which) {
        // verify the returned value
        rw_assert (&res_str == &s_str, 0, cs.line,
                   CALLFMAT " : the returned reference is invalid", CALLARGS);
    }

    // verify the result string length for iterator versions
    if (use_iters) {
        // verify the length
        std::size_t len = res_str.length ();
        rw_assert (len == exp_len, 0, cs.line,
                   CALLFMAT " : result length is %zu, %zu expected",
                   CALLARGS, len, exp_len);
    }

    bool success = !TestString::traits_type::compare (res_str.c_str(), 
                        ctl_str.c_str(), ctl_str.size ());

    // to workaround UserChar's empty string output problem
    bool use_empty_res = 
        0 == res_str.size () && sizeof (charT) > sizeof (wchar_t);
    bool use_empty_ctl = 
        0 == ctl_str.size () && sizeof (charT) > sizeof (wchar_t);

    rw_assert (success, 0, cs.line,
               CALLFMAT " == %{#*S}, got %{#*S}", CALLARGS, 
               int (sizeof (charT)), use_empty_ctl ? &s_empty : &ctl_str,
               int (sizeof (charT)), use_empty_res ? &s_empty : &res_str);

#ifndef _RWSTD_NO_EXCEPTIONS

    }
    catch (std::out_of_range) {
        caught = exp_exceptions[1];
    }
    catch (std::length_error) {
        caught = exp_exceptions[2];
    }
    catch (...) {
        caught = exp_exceptions[0];
    }

#else   // if defined (_RWSTD_NO_EXCEPTIONS)
    _RWSTD_UNUSED (should_throw);
#endif   // _RWSTD_NO_EXCEPTIONS

    rw_assert (caught == expected, 0, cs.line,
               CALLFMAT " %{?}expected %s, caught %s"
               "%{:}unexpectedly caught %s%{;}",
               CALLARGS, 0 != expected, expected, caught, caught);
}

/**************************************************************************/

static int rw_opt_no_char_traits;              // for --no-char_traits
static int rw_opt_no_user_traits;              // for --no-user_traits

static int rw_opt_no_user_chars;               // for --no-user_chars
static int rw_opt_no_exceptions;               // for --no-exceptions

static int rw_opt_no_replace_ptr;              // for --no-replace-ptr
static int rw_opt_no_replace_str;              // for --no-replace-str
static int rw_opt_no_replace_num_ptr;          // for --no-replace-num-ptr
static int rw_opt_no_replace_num_str;          // for --no-replace-num-str
static int rw_opt_no_replace_char;             // for --no-replace-char
static int rw_opt_no_replace_iters_ptr;        // for --no-replace-iters-ptr
static int rw_opt_no_replace_iters_str;        // for --no-replace-iters-str
static int rw_opt_no_replace_iters_num_ptr;    // for --no-replace-iters-num-ptr
static int rw_opt_no_replace_iters_char;       // for --no-replace-iters-char
static int rw_opt_no_replace_iters_range;      // for --no-replace-iters-range

/**************************************************************************/

static void 
test_replace (MemFun *pfid,
              int     which)
{
    rw_info (0, 0, 0, "std::basic_string<%s, %s<%1$s>, %s<%1$s>>::"
             "replace (%{?}size_type pos1%{;}%{?}iterator i1%{;}"
             "%{?}, size_type n1%{;}%{?}, iterator i2%{;}"
             "%{?}, const charT* s%{;}%{?}, const basic_string& str%{;}"
             "%{?}, size_type n%{;}%{?}, size_type pos2%{;}"
             "%{?}, size_type n2%{;}%{?}, const charT c%{;}"
             "%{?}, InputIterator j1%{;}%{?}, InputIterator j2%{;})",
             pfid->cname_, pfid->tname_, pfid->aname_, 
             5 >= which, 6 <= which, 5 >= which, 6 <= which, 
             1 == which || 3 == which || 6 == which || 8 == which,
             2 == which || 4 == which || 7 == which, 5 == which || 9 == which, 
             4 == which, 3 == which || 4 == which || 8 == which,
             5 == which || 9 == which, 10 == which, 10 == which);

#undef TEST
#define TEST(charT, Traits, cs)	                            \
    test_replace (charT(), (Traits*)0, which, pfid, cs)

    static const std::size_t ncases = sizeof test_cases / sizeof *test_cases;

    for (std::size_t i = 0; i != ncases; ++i) {

        if (!rw_enabled (test_cases[i].line)) {
            rw_note (0, 0, __LINE__, 
                     "test on line %d disabled", test_cases[i].line);
            continue;
        }

        // do not exercise exceptions if they were disabled
        if (0 != rw_opt_no_exceptions && 0 != test_cases[i].bthrow)
            continue;

        if (MemFun:: DefaultTraits == pfid->tid_) {
            if (MemFun::Char == pfid->cid_)
                TEST (char, std::char_traits<char>, test_cases[i]);

    #ifndef _RWSTD_NO_WCHAR_T
            else
                TEST (wchar_t, std::char_traits<wchar_t>, test_cases[i]);
    #endif   // _RWSTD_NO_WCHAR_T

        }
        else {
            if (MemFun::Char == pfid->cid_)
                TEST (char, UserTraits<char>, test_cases[i]);

    #ifndef _RWSTD_NO_WCHAR_T
            else if (MemFun::WChar == pfid->cid_)
                TEST (wchar_t, UserTraits<wchar_t>, test_cases[i]);
    #endif   // _RWSTD_NO_WCHAR_T

            else
                TEST (UserChar, UserTraits<UserChar>, test_cases[i]);
        }
    }
}

/**************************************************************************/

static void 
note_test_disabled (MemFun *pfid, int which)
{
    rw_note (0, 0, 0, "std::basic_string<%s, %s<%1$s>, %s<%1$s>>::"
             "replace (%{?}size_type pos1%{;}%{?}iterator i1%{;}"
             "%{?}, size_type n1%{;}%{?}, iterator i2%{;}"
             "%{?}, const charT* s%{;}%{?}, const basic_string& str%{;}"
             "%{?}, size_type n%{;}%{?}, size_type pos2%{;}"
             "%{?}, size_type n2%{;}%{?}, const charT c%{;}"
             "%{?}, InputIterator j1%{;}%{?}, InputIterator j2%{;})"
             " test disabled",
             pfid->cname_, pfid->tname_, pfid->aname_, 
             5 >= which, 6 <= which, 5 >= which, 6 <= which, 
             1 == which || 3 == which || 6 == which || 8 == which,
             2 == which || 4 == which || 7 == which, 5 == which || 9 == which, 
             4 == which, 3 == which || 4 == which || 8 == which,
             5 == which || 9 == which, 10 == which, 10 == which);
}

static void
run_test (MemFun *pfid)
{
    if (MemFun::UserTraits == pfid->tid_ && rw_opt_no_user_traits) {
        rw_note (1 < rw_opt_no_user_traits++, 0, 0,
                 "user defined traits test disabled");
    }
    else if (MemFun::DefaultTraits == pfid->tid_  && rw_opt_no_char_traits) {
        rw_note (1 < rw_opt_no_char_traits++, 0, 0,
                 "char_traits test disabled");
    }
    else {

        if (rw_opt_no_exceptions)
            rw_note (1 < rw_opt_no_exceptions++, 0, 0,
                     "string::replace exceptions tests disabled");

        // exercise all replace overloads
#undef TEST
#define TEST(option, n)                         \
        if (option)                             \
            note_test_disabled (pfid, n);       \
        else                                    \
            test_replace (pfid, n);               

    // replace (size_type pos1, size_type n1, const charT* p)
    TEST (rw_opt_no_replace_ptr,           1);         
    // replace (size_type pos1, size_type n1, basic_string& s)
    TEST (rw_opt_no_replace_str,           2);      
    // replace (size_type pos1, size_type n1, charT* p, size_type n2)
    TEST (rw_opt_no_replace_num_ptr,       3);          
    // replace (pos1, n1, basic_string& s, size_type pos2, size_type n2)
    TEST (rw_opt_no_replace_num_str,       4);  
    // replace (size_type pos1, size_type n1, size_type n, charT c)
    TEST (rw_opt_no_replace_char,          5);     
    // replace (iterator i1, iterator i2, const charT* p)
    TEST (rw_opt_no_replace_iters_ptr,     6);   
    // replace (iterator i1, iterator i2, basic_string& s)
    TEST (rw_opt_no_replace_iters_str,     7);    
    // replace (iterator i1, iterator i2, charT* p, size_type n2)
    TEST (rw_opt_no_replace_iters_num_ptr, 8);    
    // replace (iterator i1, iterator i2, size_type n, charT c)
    TEST (rw_opt_no_replace_iters_char,    9);    
    // replace (iterator i1, iterator i2, InputIterator j1, InputIterator j2)
    TEST (rw_opt_no_replace_iters_range,  10);           
    }
}

/**************************************************************************/

int run_test (int, char*[])
{
    if ('\0' == long_string [0]) {
        // initialize long_string
        for (std::size_t i = 0; i != sizeof long_string - 1; ++i)
            long_string [i] = 'x';
    }

    if (rw_enabled ("char")) {

        MemFun fid (MemFun::Char, "char", MemFun::DefaultTraits, 0);

        fid.tname_ = "char_traits";

        run_test (&fid);

        fid.tid_   = MemFun::UserTraits;
        fid.tname_ = "UserTraits";

        run_test (&fid);
    }
    else
        rw_note (0, 0, 0, "string::replace char tests disabled");

    if (rw_enabled ("wchar_t")) {

        MemFun fid (MemFun::WChar, "wchar_t", MemFun::DefaultTraits, 0);

        fid.tname_ = "char_traits";

        run_test (&fid);

        fid.tid_   = MemFun::UserTraits;
        fid.tname_ = "UserTraits";

        run_test (&fid);
    }
    else
        rw_note (0, 0, 0, "string::replace wchar tests disabled");

    if (rw_opt_no_user_chars) {
        rw_note (0, 0, 0, "user defined chars test disabled");
    }
    else {
        MemFun fid (MemFun::UChar, "UserChar", MemFun::UserTraits, 0);
        fid.tname_ = "UserTraits";
        run_test (&fid);
    }

    // silence a bogus EDG eccp remark #550-D:
    // variable "exp_exceptions" was set but never used
    _RWSTD_UNUSED (exp_exceptions);

    return 0;
}

/**************************************************************************/

int main (int argc, char** argv)
{
    return rw_test (argc, argv, __FILE__,
                    "lib.string.replace",
                    0 /* no comment */, run_test,
                    "|-no-char_traits# "
                    "|-no-user_traits# "
                    "|-no-user_chars# "
                    "|-no-exceptions# "
                    "|-no-replace-ptr# "
                    "|-no-replace-str# "
                    "|-no-replace-num-ptr# "
                    "|-no-replace-num-str# "
                    "|-no-replace-char# "
                    "|-no-replace-iters_ptr# "
                    "|-no-replace-iters_str# "
                    "|-no-replace-iters_num-ptr# "
                    "|-no-replace-iters-char# "
                    "|-no-replace-iters-range#",
                    &rw_opt_no_char_traits,
                    &rw_opt_no_user_traits,
                    &rw_opt_no_user_chars,
                    &rw_opt_no_exceptions,
                    &rw_opt_no_replace_ptr,
                    &rw_opt_no_replace_str,
                    &rw_opt_no_replace_num_ptr,
                    &rw_opt_no_replace_num_str,
                    &rw_opt_no_replace_char,
                    &rw_opt_no_replace_iters_ptr,
                    &rw_opt_no_replace_iters_str,
                    &rw_opt_no_replace_iters_num_ptr,
                    &rw_opt_no_replace_iters_char,
                    &rw_opt_no_replace_iters_range);
}
