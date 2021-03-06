/****************************************************************************
** Copyright (c) 2017, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "utest_assert.h"

#include "stream_buffer.h"

#include "../src/gmio_core/error.h"
#include "../src/gmio_core/internal/byte_codec.h"
#include "../src/gmio_core/internal/byte_swap.h"
#include "../src/gmio_core/internal/convert.h"
#include "../src/gmio_core/internal/error_check.h"
#include "../src/gmio_core/internal/fast_atof.h"
#include "../src/gmio_core/internal/file_utils.h"
#include "../src/gmio_core/internal/helper_stream.h"
#include "../src/gmio_core/internal/itoa.h"
#include "../src/gmio_core/internal/locale_utils.h"
#include "../src/gmio_core/internal/numeric_utils.h"
#include "../src/gmio_core/internal/ostringstream.h"
#include "../src/gmio_core/internal/safe_cast.h"
#include "../src/gmio_core/internal/stringstream.h"
#include "../src/gmio_core/internal/stringstream_fast_atof.h"
#include "../src/gmio_core/internal/string_ascii_utils.h"
#include "../src/gmio_core/internal/zip_utils.h"
#include "../src/gmio_core/internal/zlib_utils.h"

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char* test_internal__byte_swap()
{
    UTEST_ASSERT(gmio_uint16_bswap(0x1122) == 0x2211);
    UTEST_ASSERT(gmio_uint32_bswap(0x11223344) == 0x44332211);
    return NULL;
}

static const char* test_internal__byte_codec()
{
    { /* decode */
        const uint8_t data[] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
        UTEST_ASSERT(gmio_decode_uint16_le(data) == 0x2211);
        UTEST_ASSERT(gmio_decode_uint16_be(data) == 0x1122);
        UTEST_ASSERT(gmio_decode_uint32_le(data) == 0x44332211);
        UTEST_ASSERT(gmio_decode_uint32_be(data) == 0x11223344);
#ifdef GMIO_HAVE_INT64_TYPE
        UTEST_ASSERT(gmio_decode_uint64_le(data) == 0x8877665544332211);
        UTEST_ASSERT(gmio_decode_uint64_be(data) == 0x1122334455667788);
#endif
    }

    { /* encode */
        uint8_t bytes[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

        gmio_encode_uint16_le(0x1122, bytes);
        UTEST_ASSERT(bytes[0] == 0x22 && bytes[1] == 0x11);
        gmio_encode_uint16_be(0x1122, bytes);
        UTEST_ASSERT(bytes[0] == 0x11 && bytes[1] == 0x22);

        gmio_encode_uint32_le(0x11223344, bytes);
        static const uint8_t bytes_uint32_le[] = { 0x44, 0x33, 0x22, 0x11 };
        UTEST_ASSERT(memcmp(bytes, bytes_uint32_le, 4) == 0);

        gmio_encode_uint32_be(0x11223344, bytes);
        static const uint8_t bytes_uint32_be[] = { 0x11, 0x22, 0x33, 0x44 };
        UTEST_ASSERT(memcmp(bytes, bytes_uint32_be, 4) == 0);

#ifdef GMIO_HAVE_INT64_TYPE
        gmio_encode_uint64_le(0x1122334455667788, bytes);
        static const uint8_t bytes_uint64_le[] =
                { 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 };
        UTEST_ASSERT(memcmp(bytes, bytes_uint64_le, 8) == 0);

        gmio_encode_uint64_be(0x1122334455667788, bytes);
        static const uint8_t bytes_uint64_be[] =
                { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
        UTEST_ASSERT(memcmp(bytes, bytes_uint64_be, 8) == 0);
#endif
    }

    return NULL;
}

static const char* test_internal__const_string()
{
    char buff[512] = {0};
    struct gmio_const_string lhs = { "file", 4 };
    struct gmio_const_string rhs = { ".txt", 4 };
    UTEST_COMPARE_UINT(
                lhs.len + rhs.len,
                gmio_const_string_concat(buff, sizeof(buff), &lhs, &rhs));
    UTEST_COMPARE_CSTR("file.txt", buff);

    lhs = gmio_const_string("a", 1);
    rhs = gmio_const_string("b", 1);
    UTEST_COMPARE_UINT(
                lhs.len + rhs.len,
                gmio_const_string_concat(buff, sizeof(buff), &lhs, &rhs));
    UTEST_COMPARE_CSTR("ab", buff);

    char small_buff[8] = {0};
    lhs = gmio_const_string("1234567890", 10);
    rhs = gmio_const_string("abc", 3);
    UTEST_COMPARE_UINT(
                sizeof(small_buff)-1,
                gmio_const_string_concat(
                    small_buff, sizeof(small_buff), &lhs, &rhs));
    UTEST_COMPARE_CSTR("1234abc", small_buff);

    return NULL;
}

static void __tc__fprintf_atof_err(
        const char* func_fast_atof_str,
        const char* val_str,
        float fast_val,
        float std_val)
{
    fprintf(stderr,
            "*** ERROR: %s() less accurate than strtod()\n"
            "    value_str : \"%s\"\n"
            "    fast_value: %.12f (%s) as_int: 0x%x\n"
            "    std_value : %.12f (%s) as_int: 0x%x\n"
            "    ulp_diff  : %u\n",
            func_fast_atof_str,
            val_str,
            fast_val,
            gmio_float32_sign(fast_val) > 0 ? "+" : "-",
            gmio_convert_uint32(fast_val),
            std_val,
            gmio_float32_sign(std_val) > 0 ? "+" : "-",
            gmio_convert_uint32(std_val),
            gmio_float32_ulp_diff(fast_val, std_val));
}

static bool __tc__check_calculation_atof(const char* val_str)
{
    const float std_val = (float)strtod(val_str, NULL);
    int accurate_count = 0;

    { /* Test fast_atof() */
        const float fast_val = fast_atof(val_str);
        if (gmio_float32_ulp_equals(fast_val, std_val, 1))
            ++accurate_count;
        else
            __tc__fprintf_atof_err("fast_atof", val_str, fast_val, std_val);
    }

    { /* Test gmio_stringstream_fast_atof() */
        char iobuff[512] = {0};
        struct gmio_ro_buffer ibuff =
                gmio_ro_buffer(val_str, strlen(val_str), 0);
        struct gmio_stringstream sstream =
                gmio_stringstream(
                    gmio_istream_buffer(&ibuff),
                    gmio_string(iobuff, 0, sizeof(iobuff)));
        const float fast_val = gmio_stringstream_fast_atof(&sstream);
        if (gmio_float32_ulp_equals(fast_val, std_val, 1)) {
            ++accurate_count;
        }
        else {
            __tc__fprintf_atof_err(
                        "gmio_stringstream_fast_atof", val_str, fast_val, std_val);
        }
    }

    return accurate_count == 2;
}

static const char* test_internal__fast_atof()
{
    bool ok = true;

    ok = ok && __tc__check_calculation_atof("340282346638528859811704183484516925440.000000");
    ok = ok && __tc__check_calculation_atof("3.402823466e+38F");
    ok = ok && __tc__check_calculation_atof("3402823466e+29F");
    ok = ok && __tc__check_calculation_atof("-340282346638528859811704183484516925440.000000");
    ok = ok && __tc__check_calculation_atof("-3.402823466e+38F");
    ok = ok && __tc__check_calculation_atof("-3402823466e+29F");
    ok = ok && __tc__check_calculation_atof("34028234663852885981170418348451692544.000000");
    ok = ok && __tc__check_calculation_atof("3.402823466e+37F");
    ok = ok && __tc__check_calculation_atof("3402823466e+28F");
    ok = ok && __tc__check_calculation_atof("-34028234663852885981170418348451692544.000000");
    ok = ok && __tc__check_calculation_atof("-3.402823466e+37F");
    ok = ok && __tc__check_calculation_atof("-3402823466e+28F");
    ok = ok && __tc__check_calculation_atof(".00234567");
    ok = ok && __tc__check_calculation_atof("-.00234567");
    ok = ok && __tc__check_calculation_atof("0.00234567");
    ok = ok && __tc__check_calculation_atof("-0.00234567");
    ok = ok && __tc__check_calculation_atof("1.175494351e-38F");
#if 0
    /* This check fails */
    ok = ok && __tc__check_calculation_atof("1175494351e-47F");
#endif
    ok = ok && __tc__check_calculation_atof("1.175494351e-37F");
    ok = ok && __tc__check_calculation_atof("1.175494351e-36F");
    ok = ok && __tc__check_calculation_atof("-1.175494351e-36F");
    ok = ok && __tc__check_calculation_atof("123456.789");
    ok = ok && __tc__check_calculation_atof("-123456.789");
    ok = ok && __tc__check_calculation_atof("0000123456.789");
    ok = ok && __tc__check_calculation_atof("-0000123456.789");
    ok = ok && __tc__check_calculation_atof("-0.0690462109446526");

    UTEST_ASSERT(ok);

    return NULL;
}

static const char* test_internal__safe_cast()
{
#if GMIO_TARGET_ARCH_BIT_SIZE > 32
    const size_t maxUInt32 = 0xFFFFFFFF;
    UTEST_ASSERT(gmio_size_to_uint32(maxUInt32 + 1) == 0xFFFFFFFF);
    UTEST_ASSERT(gmio_size_to_uint32(0xFFFFFFFF) == 0xFFFFFFFF);
    UTEST_ASSERT(gmio_size_to_uint32(100) == 100);
#endif

    UTEST_ASSERT(gmio_streamsize_to_size(-1) == ((size_t)-1));
#ifdef GMIO_HAVE_INT64_TYPE
#  if GMIO_TARGET_ARCH_BIT_SIZE < 64
    const gmio_streamsize_t overMaxSizet =
            ((gmio_streamsize_t)GMIO_MAX_SIZET) + 1;
    UTEST_ASSERT(gmio_streamsize_to_size(overMaxSizet) == GMIO_MAX_SIZET);
#  endif
    UTEST_ASSERT(gmio_streamsize_to_size(GMIO_MAX_SIZET) == GMIO_MAX_SIZET);
    UTEST_ASSERT(gmio_streamsize_to_size(150) == 150);
#endif

    return NULL;
}

static const char* test_internal__stringstream()
{
    static const char text[] =
            "Une    citation,\to je crois qu'elle est de moi :"
            "Parfois le chemin est rude.\n"
            "pi : 3.1415926535897932384626433832795";
    {
        struct gmio_ro_buffer buff =
                gmio_ro_buffer(text, sizeof(text) - 1, 0);

        char cstr_small[4];
        char cstr[32];
        struct gmio_stringstream sstream =
                gmio_stringstream(
                    gmio_istream_buffer(&buff),
                    gmio_string(cstr, 0, sizeof(cstr)));

        char cstr_copy[128];
        struct gmio_string str_copy =
                gmio_string(cstr_copy, 0, sizeof(cstr_copy));

        UTEST_ASSERT(gmio_stringstream_current_char(&sstream) != NULL);
        UTEST_ASSERT(*gmio_stringstream_current_char(&sstream) == 'U');

        str_copy.len = 0;
        UTEST_ASSERT(gmio_stringstream_eat_word(&sstream, &str_copy) == 0);
        /* printf("\ncopy_strbuff.ptr = \"%s\"\n", copy_strbuff.ptr); */
        UTEST_ASSERT(strcmp(str_copy.ptr, "Une") == 0);

        str_copy.len = 0;
        UTEST_ASSERT(gmio_stringstream_eat_word(&sstream, &str_copy) == 0);
        UTEST_ASSERT(strcmp(str_copy.ptr, "citation,") == 0);

        str_copy.len = 0;
        UTEST_ASSERT(gmio_stringstream_eat_word(&sstream, &str_copy) == 0);
        UTEST_ASSERT(strcmp(str_copy.ptr, "o") == 0);

        str_copy.len = 0;
        UTEST_ASSERT(gmio_stringstream_eat_word(&sstream, &str_copy) == 0);
        UTEST_ASSERT(strcmp(str_copy.ptr, "je") == 0);

        gmio_stringstream_skip_ascii_spaces(&sstream);
        UTEST_ASSERT(gmio_stringstream_next_char(&sstream) != NULL);
        UTEST_ASSERT(*gmio_stringstream_current_char(&sstream) == 'r');

        /* Test with very small string buffer */
        buff.pos = 0;
        sstream.strbuff = gmio_string(cstr_small, 0, sizeof(cstr_small));
        gmio_stringstream_init_pos(&sstream);

        UTEST_ASSERT(*gmio_stringstream_current_char(&sstream) == 'U');
        str_copy.len = 0;
        UTEST_ASSERT(gmio_stringstream_eat_word(&sstream, &str_copy) == 0);
        str_copy.len = 0;
        UTEST_ASSERT(gmio_stringstream_eat_word(&sstream, &str_copy) == 0);
        UTEST_ASSERT(strcmp(str_copy.ptr, "citation,") == 0);
    }

    {
        struct gmio_ro_buffer buff =
                gmio_ro_buffer(text, sizeof(text) - 1, 0);

        char cstr[32];
        struct gmio_stringstream sstream =
                gmio_stringstream(
                    gmio_istream_buffer(&buff),
                    gmio_string(cstr, 0, sizeof(cstr)));

        char cstr_copy[128];
        struct gmio_string str_copy =
                gmio_string(cstr_copy, 0, sizeof(cstr_copy));

        UTEST_ASSERT(gmio_stringstream_eat_word(&sstream, &str_copy) == 0);
        UTEST_ASSERT(strcmp(str_copy.ptr, "Une") == 0);

        UTEST_ASSERT(gmio_stringstream_eat_word(&sstream, &str_copy) == 0);
        UTEST_ASSERT(strcmp(str_copy.ptr, "Unecitation,") == 0);
        UTEST_ASSERT(gmio_stringstream_eat_word(&sstream, &str_copy) == 0);
        UTEST_ASSERT(strcmp(str_copy.ptr, "Unecitation,o") == 0);
    }

    return NULL;
}

static const char* test_internal__ostringstream()
{
    static const size_t size = 8192;
    char* input = g_testcore_memblock.ptr;
    char* output = (char*)g_testcore_memblock.ptr + size;
    char strbuff[256] = {0};
    struct gmio_rw_buffer rwbuff = gmio_rw_buffer(output, size, 0);
    struct gmio_ostringstream sstream =
            gmio_ostringstream(
                gmio_stream_buffer(&rwbuff),
                gmio_string(strbuff, 0, sizeof(strbuff) - 1));

    {   /* Create "input" string */
        size_t i = 0;
        for (i = 0; i < size; ++i) {
            const char c = 32 + (i % 94); /* Printable ASCII chars */
            input[i] = c;
        }
        /* Test gmio_ostringstream_write_char() */
        for (i = 0; i < size; ++i)
            gmio_ostringstream_write_char(&sstream, input[i]);
        gmio_ostringstream_flush(&sstream);
        UTEST_ASSERT(strncmp(input, output, size) == 0);

        /* Test gmio_ostringstream_write_[ui]32() */
        {
            static const char result[] =
                    "20 12345 0 -1 -12345678 4294967295 2147483647";
            static const unsigned result_len = sizeof(result) - 1;
            rwbuff.pos = 0;
            gmio_ostringstream_write_u32(&sstream, 20);
            gmio_ostringstream_write_char(&sstream, ' ');
            gmio_ostringstream_write_u32(&sstream, 12345);
            gmio_ostringstream_write_char(&sstream, ' ');
            gmio_ostringstream_write_u32(&sstream, 0);
            gmio_ostringstream_write_char(&sstream, ' ');
            gmio_ostringstream_write_i32(&sstream, -1);
            gmio_ostringstream_write_char(&sstream, ' ');
            gmio_ostringstream_write_i32(&sstream, -12345678);
            gmio_ostringstream_write_char(&sstream, ' ');
            gmio_ostringstream_write_u32(&sstream, (uint32_t)-1);
            gmio_ostringstream_write_char(&sstream, ' ');
            gmio_ostringstream_write_i32(&sstream, ((uint32_t)1 << 31) - 1);
            gmio_ostringstream_flush(&sstream);
            UTEST_ASSERT(strncmp(result, sstream.strbuff.ptr, result_len) == 0);
            UTEST_ASSERT(strncmp(result, output, result_len) == 0);
        }

        /* Test gmio_ostringstream_write_base64() */
        {
            static const char str[] = "Fougue+gmio";
            static const char str_b64[] = "Rm91Z3VlK2dtaW8=";
            static const unsigned str_len = sizeof(str) - 1;
            static const unsigned str_b64_len = sizeof(str_b64) - 1;
            rwbuff.pos = 0;
            gmio_ostringstream_write_base64(
                        &sstream, (unsigned const char*)str, str_len);
            gmio_ostringstream_flush(&sstream);
            UTEST_ASSERT(strncmp(str_b64, sstream.strbuff.ptr, str_b64_len) == 0);
            UTEST_ASSERT(strncmp(str_b64, output, str_b64_len) == 0);
        }

        /* Test gmio_ostringstream_write_xml...() */
        {
            static const char result[] =
                    " foo=\"crac\" bar=\"456789\"<![CDATA[]]>";
            static const unsigned result_len = sizeof(result) - 1;
            rwbuff.pos = 0;
            gmio_ostringstream_write_xmlattr_str(&sstream, "foo", "crac");
            gmio_ostringstream_write_xmlattr_u32(&sstream, "bar", 456789);
            gmio_ostringstream_write_xmlcdata_open(&sstream);
            gmio_ostringstream_write_xmlcdata_close(&sstream);
            gmio_ostringstream_flush(&sstream);
            UTEST_ASSERT(strncmp(result, sstream.strbuff.ptr, result_len) == 0);
            UTEST_ASSERT(strncmp(result, output, result_len) == 0);
        }
    }

    return NULL;
}

static const char* test_internal__string_ascii_utils()
{
    char c; /* for loop counter */

    UTEST_ASSERT(gmio_ascii_isspace(' '));
    UTEST_ASSERT(gmio_ascii_isspace('\t'));
    UTEST_ASSERT(gmio_ascii_isspace('\n'));
    UTEST_ASSERT(gmio_ascii_isspace('\r'));

    for (c = 0; 0 <= c && c <= 127; ++c) {
        if (65 <= c && c <= 90) {
            UTEST_ASSERT(gmio_ascii_isupper(c));
        }
        else if (97 <= c && c <= 122) {
            UTEST_ASSERT(gmio_ascii_islower(c));
        }
        else if (c == 0x20 || (0x09 <= c && c <= 0x0d)) {
            UTEST_ASSERT(gmio_ascii_isspace(c));
        }
        else if (48 <= c && c <= 57) {
            UTEST_ASSERT(gmio_ascii_isdigit(c));
        }
        else {
            UTEST_ASSERT(!gmio_ascii_isupper(c));
            UTEST_ASSERT(!gmio_ascii_islower(c));
            UTEST_ASSERT(!gmio_ascii_isspace(c));
            UTEST_ASSERT(!gmio_ascii_isdigit(c));
        }
    }

    UTEST_ASSERT(gmio_ascii_tolower('A') == 'a');
    UTEST_ASSERT(gmio_ascii_tolower('Z') == 'z');
    UTEST_ASSERT(gmio_ascii_tolower('(') == '(');
    UTEST_ASSERT(gmio_ascii_toupper('a') == 'A');
    UTEST_ASSERT(gmio_ascii_toupper('z') == 'Z');
    UTEST_ASSERT(gmio_ascii_toupper('(') == '(');

    UTEST_ASSERT(gmio_ascii_char_iequals('a', 'a'));
    UTEST_ASSERT(gmio_ascii_char_iequals('a', 'A'));
    UTEST_ASSERT(gmio_ascii_char_iequals('A', 'a'));
    UTEST_ASSERT(gmio_ascii_char_iequals('{', '{'));
    UTEST_ASSERT(!gmio_ascii_char_iequals('{', '['));

    UTEST_ASSERT(gmio_ascii_stricmp("FACET", "facet") == 0);
    UTEST_ASSERT(gmio_ascii_stricmp("facet", "FACET") == 0);
    UTEST_ASSERT(gmio_ascii_stricmp("facet", "facet") == 0);
    UTEST_ASSERT(gmio_ascii_stricmp("FACET", "FACET") == 0);
    UTEST_ASSERT(gmio_ascii_stricmp("", "") == 0);
    UTEST_ASSERT(gmio_ascii_stricmp("", "facet") != 0);
    UTEST_ASSERT(gmio_ascii_stricmp("facet", "facet_") != 0);
    UTEST_ASSERT(gmio_ascii_stricmp("facet_", "facet") != 0);

    UTEST_ASSERT(gmio_ascii_istarts_with("facet", ""));
    UTEST_ASSERT(gmio_ascii_istarts_with("facet", "f"));
    UTEST_ASSERT(gmio_ascii_istarts_with("facet", "fa"));
    UTEST_ASSERT(gmio_ascii_istarts_with("facet", "facet"));
    UTEST_ASSERT(!gmio_ascii_istarts_with("facet", "a"));
    UTEST_ASSERT(!gmio_ascii_istarts_with("facet", " facet"));
    UTEST_ASSERT(gmio_ascii_istarts_with("facet", "F"));
    UTEST_ASSERT(gmio_ascii_istarts_with("FACET", "f"));
    UTEST_ASSERT(gmio_ascii_istarts_with("FACET", "fa"));

    return NULL;
}

static const char* __tc__test_internal__locale_utils()
{
    const char* lc = setlocale(LC_NUMERIC, "");
    if (lc != NULL
            && gmio_ascii_stricmp(lc, "C") != 0
            && gmio_ascii_stricmp(lc, "POSIX") != 0)
    {
        int error = GMIO_ERROR_OK;
        UTEST_ASSERT(!gmio_lc_numeric_is_C());
        UTEST_ASSERT(!gmio_check_lc_numeric(&error));
        UTEST_COMPARE_INT(GMIO_ERROR_BAD_LC_NUMERIC, error);
    }
    else {
        fprintf(stderr, "\nskip: default locale is NULL or already C/POSIX");
    }

    lc = setlocale(LC_NUMERIC, "C");
    if (lc != NULL) {
        int error = GMIO_ERROR_OK;
        UTEST_ASSERT(gmio_lc_numeric_is_C());
        UTEST_ASSERT(gmio_check_lc_numeric(&error));
        UTEST_COMPARE_INT(GMIO_ERROR_OK, error);
    }

    return NULL;
}

static const char* test_internal__locale_utils()
{
    const char* error_str = NULL;
    gmio_lc_numeric_save();
    printf("\ninfo: current locale is \"%s\"", gmio_lc_numeric());
    error_str = __tc__test_internal__locale_utils();
    gmio_lc_numeric_restore();
    return error_str;
}

static const char* test_internal__error_check()
{
    /* gmio_check_memblock() */
    {
        int error = GMIO_ERROR_OK;
        uint8_t buff[128] = {0};
        struct gmio_memblock mblock = {0};

        UTEST_ASSERT(!gmio_check_memblock(&error, NULL));
        UTEST_ASSERT(error == GMIO_ERROR_NULL_MEMBLOCK);

        UTEST_ASSERT(!gmio_check_memblock(&error, &mblock));
        UTEST_ASSERT(error == GMIO_ERROR_NULL_MEMBLOCK);

        mblock = gmio_memblock(buff, 0, NULL);
        UTEST_ASSERT(!gmio_check_memblock(&error, &mblock));
        UTEST_ASSERT(error == GMIO_ERROR_INVALID_MEMBLOCK_SIZE);

        /* Verify that gmio_check_memblock() doesn't touch error when in case of
         * success */
        mblock = gmio_memblock(buff, sizeof(buff), NULL);
        UTEST_ASSERT(!gmio_check_memblock(&error, &mblock));
        UTEST_ASSERT(error == GMIO_ERROR_INVALID_MEMBLOCK_SIZE);

        error = GMIO_ERROR_OK;
        UTEST_ASSERT(gmio_check_memblock(&error, &mblock));
        UTEST_ASSERT(error == GMIO_ERROR_OK);
    }

    /* gmio_check_memblock_size() */
    {
        uint8_t buff[128] = {0};
        struct gmio_memblock mblock = gmio_memblock(buff, sizeof(buff), NULL);
        int error = GMIO_ERROR_OK;
        UTEST_ASSERT(!gmio_check_memblock_size(&error, &mblock, 2*sizeof(buff)));
        UTEST_ASSERT(error == GMIO_ERROR_INVALID_MEMBLOCK_SIZE);
    }

    return NULL;
}

static void __tc__write_file(const char* filepath, uint8_t* bytes, size_t len)
{
    FILE* file = fopen(filepath, "wb");
    fwrite(bytes, 1, len, file);
    fclose(file);
}

struct __tc__func_write_file_data_cookie {
    struct gmio_stream* stream;
    const uint8_t* data;
    const uint8_t* zdata;
    uint32_t data_crc32;
    size_t data_len;
    size_t zdata_len;
};

static int __tc__write_zip_file_data(
        void* cookie, struct gmio_zip_data_descriptor* dd)
{
    struct __tc__func_write_file_data_cookie* fcookie =
            (struct __tc__func_write_file_data_cookie*)cookie;
    gmio_stream_write_bytes(fcookie->stream, fcookie->zdata, fcookie->zdata_len);
    dd->crc32 = fcookie->data_crc32;
    dd->uncompressed_size = fcookie->data_len;
    dd->compressed_size = fcookie->zdata_len;
    return GMIO_ERROR_OK;
}

static const char* __tc__zip_compare_entry(
        const struct gmio_zip_file_entry* lhs,
        const struct gmio_zip_file_entry* rhs)
{
    UTEST_COMPARE_UINT(lhs->compress_method, rhs->compress_method);
    UTEST_COMPARE_UINT(lhs->feature_version, rhs->feature_version);
    UTEST_COMPARE_UINT(lhs->filename_len, rhs->filename_len);
    UTEST_ASSERT(strncmp(lhs->filename, rhs->filename, lhs->filename_len) == 0);
    return NULL;
}

static const char* test_internal__zip_utils()
{
    static const unsigned bytes_size = 1024;
    uint8_t* bytes = g_testcore_memblock.ptr;
    struct gmio_rw_buffer wbuff = gmio_rw_buffer(bytes, bytes_size, 0);
    struct gmio_stream stream = gmio_stream_buffer(&wbuff);
    int error;

    /* Write empty ZIP file */
    {
        struct gmio_zip_end_of_central_directory_record eocdr = {0};
        gmio_zip_write_end_of_central_directory_record(&stream, &eocdr, &error);
        UTEST_COMPARE_INT(error, GMIO_ERROR_OK);
#if 1
        __tc__write_file("test_output_empty.zip", wbuff.ptr, wbuff.pos);
#endif
    }

    /* Write empty Zip64 file */
    wbuff.pos = 0;
    {
        struct gmio_zip64_end_of_central_directory_record eocdr64 = {0};
        eocdr64.version_needed_to_extract =
                GMIO_ZIP_FEATURE_VERSION_FILE_ZIP64_FORMAT_EXTENSIONS;
        gmio_zip64_write_end_of_central_directory_record(
            &stream, &eocdr64, &error);
        UTEST_COMPARE_INT(error, GMIO_ERROR_OK);

        struct gmio_zip64_end_of_central_directory_locator eocdl64 = {0};
        gmio_zip64_write_end_of_central_directory_locator(
                    &stream, &eocdl64, &error);
        UTEST_COMPARE_INT(error, GMIO_ERROR_OK);

        struct gmio_zip_end_of_central_directory_record eocdr = {0};
        gmio_zip_write_end_of_central_directory_record(&stream, &eocdr, &error);
        UTEST_COMPARE_INT(error, GMIO_ERROR_OK);
#if 1
        __tc__write_file("test_output_empty_64.zip", wbuff.ptr, wbuff.pos);
#endif
    }

    /* Common constants */
    static const char zip_entry_filedata[] =
            "On ne fait bien que ce qu'on fait soi-même";
    struct __tc__func_write_file_data_cookie fcookie = {0};
    fcookie.stream = &stream;
    fcookie.data = (const uint8_t*)zip_entry_filedata;
    fcookie.zdata = fcookie.data; /* No compression */
    fcookie.data_len = GMIO_ARRAY_SIZE(zip_entry_filedata) - 1;
    fcookie.zdata_len = fcookie.data_len; /* No compression */
    fcookie.data_crc32 = gmio_zlib_crc32(fcookie.data, fcookie.data_len);
    static const char zip_entry_filename[] = "file.txt";
    static const uint16_t zip_entry_filename_len =
            GMIO_ARRAY_SIZE(zip_entry_filename) - 1;

    /* Common variables */
    struct gmio_zip_file_entry entry = {0};
    entry.compress_method = GMIO_ZIP_COMPRESS_METHOD_NO_COMPRESSION;
    entry.feature_version = GMIO_ZIP_FEATURE_VERSION_DEFAULT;
    entry.filename = zip_entry_filename;
    entry.filename_len = zip_entry_filename_len;
    entry.cookie_func_write_file_data = &fcookie;
    entry.func_write_file_data = __tc__write_zip_file_data;

    /*
     * Write one-entry ZIP file
     */
    {
        wbuff.pos = 0;
        entry.feature_version = GMIO_ZIP_FEATURE_VERSION_DEFAULT;
        UTEST_ASSERT(gmio_zip_write_single_file(&stream, &entry, &error));
        UTEST_COMPARE_UINT(error, GMIO_ERROR_OK);
#if 1
        __tc__write_file("test_output_one_file.zip", wbuff.ptr, wbuff.pos);
#endif

        const uintmax_t zip_archive_len = wbuff.pos;
        wbuff.pos = 0;
        /* -- Read ZIP local file header */
        struct gmio_zip_local_file_header zip_lfh = {0};
        const size_t lfh_read_len =
                gmio_zip_read_local_file_header(&stream, &zip_lfh, &error);
        UTEST_COMPARE_INT(error, GMIO_ERROR_OK);
        struct gmio_zip_file_entry lfh_entry = {0};
        lfh_entry.compress_method = zip_lfh.compress_method;
        lfh_entry.feature_version = zip_lfh.version_needed_to_extract;
        lfh_entry.filename = (const char*)wbuff.ptr + wbuff.pos;
        lfh_entry.filename_len = zip_lfh.filename_len;
        const char* check_str = __tc__zip_compare_entry(&entry, &lfh_entry);
        if (check_str != NULL) return check_str;
        /* -- Read ZIP end of central directory record */
        wbuff.pos =
                zip_archive_len - GMIO_ZIP_SIZE_END_OF_CENTRAL_DIRECTORY_RECORD;
        struct gmio_zip_end_of_central_directory_record zip_eocdr = {0};
        gmio_zip_read_end_of_central_directory_record(
                    &stream, &zip_eocdr, &error);
        UTEST_COMPARE_INT(error, GMIO_ERROR_OK);
        UTEST_COMPARE_UINT(zip_eocdr.entry_count, 1);
        /* -- Read ZIP central directory */
        wbuff.pos = zip_eocdr.central_dir_offset;
        struct gmio_zip_central_directory_header zip_cdh = {0};
        gmio_zip_read_central_directory_header(&stream, &zip_cdh, &error);
        struct gmio_zip_file_entry cdh_entry = {0};
        cdh_entry.compress_method = zip_cdh.compress_method;
        cdh_entry.feature_version = zip_cdh.version_needed_to_extract;
        cdh_entry.filename = (const char*)wbuff.ptr + wbuff.pos;
        cdh_entry.filename_len = zip_lfh.filename_len;
        UTEST_COMPARE_INT(error, GMIO_ERROR_OK);
        UTEST_ASSERT((zip_cdh.general_purpose_flags &
                      GMIO_ZIP_GENERAL_PURPOSE_FLAG_USE_DATA_DESCRIPTOR)
                     != 0);
        UTEST_COMPARE_UINT(fcookie.data_crc32, zip_cdh.crc32);
        UTEST_COMPARE_UINT(fcookie.data_len, zip_cdh.uncompressed_size);
        UTEST_COMPARE_UINT(fcookie.zdata_len, zip_cdh.compressed_size);
        UTEST_COMPARE_UINT(zip_cdh.local_header_offset, 0);
        check_str = __tc__zip_compare_entry(&entry, &cdh_entry);
        if (check_str != NULL) return check_str;
        /* -- Read file data */
        const size_t pos_file_data =
                lfh_read_len + zip_lfh.filename_len + zip_lfh.extrafield_len;
        UTEST_ASSERT(strncmp((const char*)wbuff.ptr + pos_file_data,
                             (const char*)fcookie.zdata,
                             fcookie.zdata_len)
                     == 0);
        /* -- Read ZIP data descriptor */
        wbuff.pos = pos_file_data + zip_cdh.compressed_size;
        struct gmio_zip_data_descriptor zip_dd = {0};
        gmio_zip_read_data_descriptor(&stream, &zip_dd, &error);
        UTEST_COMPARE_INT(error, GMIO_ERROR_OK);
        UTEST_COMPARE_UINT(zip_dd.crc32, zip_cdh.crc32);
        UTEST_COMPARE_UINT(zip_dd.compressed_size, zip_cdh.compressed_size);
        UTEST_COMPARE_UINT(zip_dd.uncompressed_size, zip_cdh.uncompressed_size);
    }

    /*
     * Write one-entry Zip64 file
     */
    {
        wbuff.pos = 0;
        entry.feature_version =
                GMIO_ZIP_FEATURE_VERSION_FILE_ZIP64_FORMAT_EXTENSIONS;
        UTEST_ASSERT(gmio_zip_write_single_file(&stream, &entry, &error));
        UTEST_COMPARE_UINT(error, GMIO_ERROR_OK);
#if 1
        __tc__write_file("test_output_one_file_64.zip", wbuff.ptr, wbuff.pos);
#endif

        const uintmax_t zip_archive_len = wbuff.pos;
        /* -- Read ZIP end of central directory record */
        wbuff.pos =
                zip_archive_len - GMIO_ZIP_SIZE_END_OF_CENTRAL_DIRECTORY_RECORD;
        struct gmio_zip_end_of_central_directory_record zip_eocdr = {0};
        gmio_zip_read_end_of_central_directory_record(
                    &stream, &zip_eocdr, &error);
        UTEST_COMPARE_INT(GMIO_ERROR_OK, error);
        UTEST_COMPARE_UINT(UINT16_MAX, zip_eocdr.entry_count);
        UTEST_COMPARE_UINT(UINT32_MAX, zip_eocdr.central_dir_size);
        UTEST_COMPARE_UINT(UINT32_MAX, zip_eocdr.central_dir_offset);
        /* -- Read Zip64 end of central directory locator */
        wbuff.pos =
                zip_archive_len
                - GMIO_ZIP_SIZE_END_OF_CENTRAL_DIRECTORY_RECORD
                - GMIO_ZIP64_SIZE_END_OF_CENTRAL_DIRECTORY_LOCATOR;
        struct gmio_zip64_end_of_central_directory_locator zip64_eocdl = {0};
        gmio_zip64_read_end_of_central_directory_locator(
                    &stream, &zip64_eocdl, &error);
        UTEST_COMPARE_INT(GMIO_ERROR_OK, error);
        /* -- Read Zip64 end of central directory record */
        wbuff.pos = zip64_eocdl.zip64_end_of_central_dir_offset;
        struct gmio_zip64_end_of_central_directory_record zip64_eocdr = {0};
        gmio_zip64_read_end_of_central_directory_record(
                    &stream, &zip64_eocdr, &error);
        UTEST_COMPARE_INT(GMIO_ERROR_OK, error);
        UTEST_COMPARE_UINT(
                    GMIO_ZIP_FEATURE_VERSION_FILE_ZIP64_FORMAT_EXTENSIONS,
                    zip64_eocdr.version_needed_to_extract);
        UTEST_COMPARE_UINT(1, zip64_eocdr.entry_count);
        /* -- Read ZIP central directory header */
        wbuff.pos = zip64_eocdr.central_dir_offset;
        struct gmio_zip_central_directory_header zip_cdh = {0};
        size_t zip_cdh_len =
                gmio_zip_read_central_directory_header(&stream, &zip_cdh, &error);
        zip_cdh_len +=
                zip_cdh.filename_len
                + zip_cdh.extrafield_len
                + zip_cdh.filecomment_len;
        UTEST_COMPARE_INT(GMIO_ERROR_OK, error);
        UTEST_COMPARE_UINT(zip64_eocdr.central_dir_size, zip_cdh_len);
        UTEST_ASSERT((zip_cdh.general_purpose_flags &
                      GMIO_ZIP_GENERAL_PURPOSE_FLAG_USE_DATA_DESCRIPTOR)
                     != 0);
        UTEST_COMPARE_UINT(fcookie.data_crc32, zip_cdh.crc32);
        UTEST_COMPARE_UINT(UINT32_MAX, zip_cdh.uncompressed_size);
        UTEST_COMPARE_UINT(UINT32_MAX, zip_cdh.compressed_size);
        UTEST_COMPARE_UINT(UINT32_MAX, zip_cdh.local_header_offset);
        struct gmio_zip_file_entry cdh_entry = {0};
        cdh_entry.compress_method = zip_cdh.compress_method;
        cdh_entry.feature_version = zip_cdh.version_needed_to_extract;
        cdh_entry.filename = (const char*)wbuff.ptr + wbuff.pos;
        cdh_entry.filename_len = zip_cdh.filename_len;
        const char* check_str = __tc__zip_compare_entry(&entry, &cdh_entry);
        if (check_str != NULL) return check_str;
        /* Read Zip64 extra field */
        wbuff.pos =
                zip64_eocdr.central_dir_offset
                + GMIO_ZIP_SIZE_CENTRAL_DIRECTORY_HEADER
                + zip_cdh.filename_len;
        struct gmio_zip64_extrafield zip64_extra = {0};
        gmio_zip64_read_extrafield(&stream, &zip64_extra, &error);
        UTEST_COMPARE_INT(GMIO_ERROR_OK, error);
        UTEST_COMPARE_UINT(fcookie.data_len, zip64_extra.uncompressed_size);
        UTEST_COMPARE_UINT(fcookie.zdata_len, zip64_extra.compressed_size);
        UTEST_COMPARE_UINT(0, zip64_extra.local_header_offset);
        /* Read ZIP local file header */
        wbuff.pos = 0;
        struct gmio_zip_local_file_header zip_lfh = {0};
        gmio_zip_read_local_file_header(&stream, &zip_lfh, &error);
        UTEST_COMPARE_INT(GMIO_ERROR_OK, error);
        UTEST_COMPARE_INT(0, zip_lfh.crc32);
        UTEST_COMPARE_INT(0, zip_lfh.compressed_size);
        UTEST_COMPARE_INT(0, zip_lfh.uncompressed_size);
        /* Read ZIP local file header extrafield */
        wbuff.pos = GMIO_ZIP_SIZE_LOCAL_FILE_HEADER + zip_lfh.filename_len;
        gmio_zip64_read_extrafield(&stream, &zip64_extra, &error);
        UTEST_COMPARE_INT(GMIO_ERROR_OK, error);
        UTEST_COMPARE_UINT(0, zip64_extra.uncompressed_size);
        UTEST_COMPARE_UINT(0, zip64_extra.compressed_size);
        UTEST_COMPARE_UINT(0, zip64_extra.local_header_offset);
        /* Read Zip64 data descriptor */
        wbuff.pos += fcookie.zdata_len;
        struct gmio_zip_data_descriptor zip_dd = {0};
        gmio_zip64_read_data_descriptor(&stream, &zip_dd, &error);
        UTEST_COMPARE_INT(GMIO_ERROR_OK, error);
        UTEST_COMPARE_UINT(fcookie.data_crc32, zip_dd.crc32);
        UTEST_COMPARE_UINT(fcookie.data_len, zip_dd.uncompressed_size);
        UTEST_COMPARE_UINT(fcookie.zdata_len, zip_dd.compressed_size);
    }

    return NULL;
}

static const char* test_internal__zlib_enumvalues()
{
    struct __int_pair { int v1; int v2; };
    static const struct __int_pair enumConst[] = {
        /* enum gmio_zlib_compress_level */
        { Z_BEST_SPEED, GMIO_ZLIB_COMPRESS_LEVEL_BEST_SPEED },
        { Z_BEST_COMPRESSION, GMIO_ZLIB_COMPRESS_LEVEL_BEST_SIZE },
        { 0, GMIO_ZLIB_COMPRESS_LEVEL_DEFAULT },
        { -1, GMIO_ZLIB_COMPRESS_LEVEL_NONE },
        /* enum gmio_zlib_compress_strategy */
        { Z_DEFAULT_STRATEGY, GMIO_ZLIB_COMPRESSION_STRATEGY_DEFAULT },
        { Z_FILTERED, GMIO_ZLIB_COMPRESSION_STRATEGY_FILTERED },
        { Z_HUFFMAN_ONLY, GMIO_ZLIB_COMPRESSION_STRATEGY_HUFFMAN_ONLY },
        { Z_RLE, GMIO_ZLIB_COMPRESSION_STRATEGY_RLE },
        { Z_FIXED, GMIO_ZLIB_COMPRESSION_STRATEGY_FIXED }
    };
    for (size_t i = 0; i < GMIO_ARRAY_SIZE(enumConst); ++i)
        UTEST_COMPARE_INT(enumConst[i].v1, enumConst[i].v2);
    return NULL;
}

static const char* test_internal__file_utils()
{
    struct gmio_const_string cstr = {0};

    cstr = gmio_fileutils_find_basefilename("");
    UTEST_ASSERT(gmio_const_string_is_empty(&cstr));

    cstr = gmio_fileutils_find_basefilename("file");
    UTEST_ASSERT(strncmp("file", cstr.ptr, cstr.len) == 0);

    cstr = gmio_fileutils_find_basefilename("file.txt");
    UTEST_ASSERT(strncmp("file", cstr.ptr, cstr.len) == 0);

    cstr = gmio_fileutils_find_basefilename("file.txt.zip");
    UTEST_ASSERT(strncmp("file.txt", cstr.ptr, cstr.len) == 0);

    cstr = gmio_fileutils_find_basefilename("/home/me/file.txt");
    UTEST_ASSERT(strncmp("file", cstr.ptr, cstr.len) == 0);

    cstr = gmio_fileutils_find_basefilename("/home/me/file.");
    UTEST_ASSERT(strncmp("file", cstr.ptr, cstr.len) == 0);

    cstr = gmio_fileutils_find_basefilename("/home/me/file");
    UTEST_ASSERT(strncmp("file", cstr.ptr, cstr.len) == 0);

    cstr = gmio_fileutils_find_basefilename("/home/me/file for you.txt");
    UTEST_ASSERT(strncmp("file for you", cstr.ptr, cstr.len) == 0);

    cstr = gmio_fileutils_find_basefilename("C:\\Program Files\\gmio\\file.txt");
    UTEST_ASSERT(strncmp("file", cstr.ptr, cstr.len) == 0);

    return NULL;
}

static const char* test_internal__itoa()
{
    char buff[512] = {0};
    {
        gmio_u32toa(0, buff);
        UTEST_COMPARE_CSTR("0", buff);
        gmio_u32toa(100, buff);
        UTEST_COMPARE_CSTR("100", buff);
        gmio_u32toa(UINT32_MAX, buff);
        UTEST_COMPARE_CSTR("4294967295", buff);

        memset(buff, 0, sizeof(buff));
        gmio_i32toa(0, buff);
        UTEST_COMPARE_CSTR("0", buff);
        gmio_i32toa(-100, buff);
        UTEST_COMPARE_CSTR("-100", buff);
    }
#ifdef GMIO_HAVE_INT64_TYPE
    {
        uint64_t u64 = UINT32_MAX;
        u64 += UINT32_MAX;
        gmio_u64toa(u64, buff);
        UTEST_COMPARE_CSTR("8589934590", buff);

        const int64_t i64 = -1 * u64;
        gmio_i64toa(i64, buff);
        UTEST_COMPARE_CSTR("-8589934590", buff);
    }
#endif
    return NULL;
}
