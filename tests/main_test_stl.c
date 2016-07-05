/****************************************************************************
** gmio
** Copyright Fougue (24 Jun. 2016)
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

#include "utest_lib.h"

#include "../src/gmio_core/memblock.h"

#include "test_stl_infos.c"
#include "test_stl_internal.c"
#include "test_stl_io.c"
#include "test_stl_triangle.c"
#include "test_stlb_header.c"

/* Static memblock */
struct gmio_memblock gmio_memblock_for_tests()
{
    static uint8_t buff[1024]; /* 1KB */
    return gmio_memblock(buff, sizeof(buff), NULL);
}

const char* all_tests()
{
    UTEST_SUITE_START();

    gmio_memblock_set_default_constructor(gmio_memblock_for_tests);

    UTEST_RUN(test_stl_coords_packing);
    UTEST_RUN(test_stl_triangle_packing);
    UTEST_RUN(test_stl_triangle_compute_normal);

    UTEST_RUN(test_stl_internal__rw_common);

    UTEST_RUN(test_stl_infos);

    UTEST_RUN(test_stl_read);
    UTEST_RUN(test_stl_read_multi_solid);
    UTEST_RUN(test_stla_lc_numeric);
    UTEST_RUN(test_stla_write);
    UTEST_RUN(test_stlb_read);
    UTEST_RUN(test_stlb_write);
    UTEST_RUN(test_stlb_header_write);

    UTEST_RUN(test_stlb_header_str);
    UTEST_RUN(test_stlb_header_to_printable_str);

#if 0
    generate_stlb_tests_models();
#endif

    return NULL;
}
UTEST_MAIN(all_tests)
