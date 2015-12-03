/****************************************************************************
** gmio
** Copyright Fougue (2 Mar. 2015)
** contact@fougue.pro
**
** This software is a reusable library whose purpose is to provide complete
** I/O support for various CAD file formats (eg. STL)
**
** This software is governed by the CeCILL-B license under French law and
** abiding by the rules of distribution of free software.  You can  use,
** modify and/ or redistribute the software under the terms of the CeCILL-B
** license as circulated by CEA, CNRS and INRIA at the following URL
** "http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html".
****************************************************************************/

#include "utest_assert.h"

#include "../src/gmio_core/global.h"
#include "../src/gmio_core/rwargs.h"
#include "../src/gmio_stl/stl_triangle.h"

#include <stddef.h>
#include <string.h>

/* Disable MSVC warning "conditional expression is constant" */
GMIO_PRAGMA_MSVC_WARNING_PUSH_AND_DISABLE(4127)

const char* test_platform__alignment()
{
    UTEST_ASSERT(offsetof(struct gmio_stl_coords, x) == 0);
    UTEST_ASSERT(offsetof(struct gmio_stl_coords, y) == 4);
    UTEST_ASSERT(offsetof(struct gmio_stl_coords, z) == 8);
    UTEST_ASSERT(sizeof(struct gmio_stl_coords) == GMIO_STL_COORDS_RAWSIZE);

    UTEST_ASSERT(offsetof(struct gmio_stl_triangle, normal) == 0);
    UTEST_ASSERT(offsetof(struct gmio_stl_triangle, v1) == GMIO_STL_COORDS_RAWSIZE);
    UTEST_ASSERT(offsetof(struct gmio_stl_triangle, v2) == 2*GMIO_STL_COORDS_RAWSIZE);
    UTEST_ASSERT(offsetof(struct gmio_stl_triangle, v3) == 3*GMIO_STL_COORDS_RAWSIZE);
    UTEST_ASSERT(offsetof(struct gmio_stl_triangle, attribute_byte_count) == 4*GMIO_STL_COORDS_RAWSIZE);
    UTEST_ASSERT(sizeof(struct gmio_stl_triangle) >= GMIO_STLB_TRIANGLE_RAWSIZE);

    return NULL;
}

const char* test_platform__global_h()
{
    UTEST_ASSERT(sizeof(int8_t) == 1);
    UTEST_ASSERT(sizeof(uint8_t) == 1);

    UTEST_ASSERT(sizeof(int16_t) == 2);
    UTEST_ASSERT(sizeof(uint16_t) == 2);

    UTEST_ASSERT(sizeof(int32_t) == 4);
    UTEST_ASSERT(sizeof(uint32_t) == 4);

#ifdef GMIO_HAVE_INT64_TYPE
    UTEST_ASSERT(sizeof(int64_t) == 8);
    UTEST_ASSERT(sizeof(uint64_t) == 8);
#endif

    UTEST_ASSERT(sizeof(gmio_float32_t) == 4);
    UTEST_ASSERT(sizeof(gmio_float64_t) == 8);

    return NULL;
}

const char* test_platform__compiler()
{
    /* Check that initialization with {0} works as expected
     *
     * See http://www.ex-parrot.com/~chris/random/initialise.html
     *
     * Depending on your version, GCC can incorrectly reports the warning
     * "missing braces around initializer [-Wmissing-braces]"
     * See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119
     */
    {
        const struct gmio_rwargs args_null_bracket0 = {0};
        struct gmio_rwargs args_null_memset0;

        memset(&args_null_memset0, 0, sizeof(struct gmio_rwargs));

        UTEST_ASSERT(memcmp(
                         &args_null_bracket0,
                         &args_null_memset0,
                         sizeof(struct gmio_rwargs))
                     == 0);

        UTEST_ASSERT(sizeof(struct gmio_rwargs)
                     >= (sizeof(struct gmio_stream)
                         + sizeof(struct gmio_memblock)
                         + sizeof(struct gmio_task_iface)));
    }

    /* Check sizeof() operator with fixed-size arrays */
    {
        uint8_t buff[4 * 1024]; /* 4KB */
        UTEST_ASSERT(sizeof(buff) == 4 * 1024);
    }

    /* gmio doesn't support platforms where NULL != 0 */
    UTEST_ASSERT(NULL == 0);

    return NULL;
}

GMIO_PRAGMA_MSVC_WARNING_POP()
