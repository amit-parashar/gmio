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

/* TODO : documentation */

#ifndef GMIO_INTERNAL_STL_RW_COMMON_H
#define GMIO_INTERNAL_STL_RW_COMMON_H

#include "stl_funptr_typedefs.h"

#include "../../gmio_core/global.h"
#include "../../gmio_core/endian.h"

struct gmio_memblock;
struct gmio_stl_mesh;

bool gmio_check_memblock(int* error, const struct gmio_memblock* mblock);

bool gmio_check_memblock_size(
        int* error, const struct gmio_memblock* mblock, size_t minsize);

bool gmio_stl_check_mesh(int* error, const struct gmio_stl_mesh* mesh);

bool gmio_stla_check_float32_precision(int* error, uint8_t prec);

bool gmio_stlb_check_byteorder(int* error, enum gmio_endianness byte_order);

#endif /* GMIO_INTERNAL_STLB_RW_COMMON_H */
