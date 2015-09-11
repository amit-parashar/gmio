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

#include "stl_utils.h"

#include "utils.h"

#include "../src/gmio_core/internal/min_max.h"
#include "../src/gmio_core/internal/safe_cast.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void gmio_stl_nop_add_triangle(
        void *cookie, uint32_t tri_id, const gmio_stl_triangle_t *triangle)
{
    GMIO_UNUSED(cookie);
    GMIO_UNUSED(tri_id);
    GMIO_UNUSED(triangle);
}

gmio_stl_triangle_array_t gmio_stl_triangle_array_malloc(size_t tri_count)
{
    gmio_stl_triangle_array_t array = {0};
    if (tri_count > 0) {
        array.ptr =
                (gmio_stl_triangle_t*)malloc(tri_count * sizeof(gmio_stl_triangle_t));
    }
    array.count = gmio_size_to_uint32(tri_count);
    array.capacity = array.count;
    return array;
}

static void gmio_stl_data__ascii_begin_solid(
        void* cookie, size_t stream_size, const char* solid_name)
{
    gmio_stl_data_t* data = (gmio_stl_data_t*)cookie;

    memset(&data->solid_name[0], 0, sizeof(data->solid_name));
    if (solid_name != NULL) {
        const size_t len =
                GMIO_MIN(sizeof(data->solid_name), strlen(solid_name));
        strncpy(&data->solid_name[0], solid_name, len);
    }

    /* Try to guess how many vertices we could have assume we'll need 200 bytes
     * for each face */
    {
        const size_t facet_size = 200;
        const size_t facet_count = GMIO_MAX(1, stream_size / facet_size);
        data->tri_array = gmio_stl_triangle_array_malloc(facet_count);
    }
}

static void gmio_stl_data__binary_begin_solid(
        void* cookie, uint32_t tri_count, const gmio_stlb_header_t* header)
{
    gmio_stl_data_t* data = (gmio_stl_data_t*)cookie;
    memcpy(&data->header, header, GMIO_STLB_HEADER_SIZE);
    data->tri_array = gmio_stl_triangle_array_malloc(tri_count);
}

static void gmio_stl_data__add_triangle(
        void* cookie, uint32_t tri_id, const gmio_stl_triangle_t* triangle)
{
    gmio_stl_data_t* data = (gmio_stl_data_t*)cookie;
    if (tri_id >= data->tri_array.capacity) {
        uint32_t cap = data->tri_array.capacity;
        cap += cap >> 3; /* Add 12.5% more capacity */
        data->tri_array.ptr =
                realloc(data->tri_array.ptr, cap * sizeof(gmio_stl_triangle_t));
        data->tri_array.capacity = cap;
    }
    data->tri_array.ptr[tri_id] = *triangle;
    data->tri_array.count = GMIO_MAX(data->tri_array.count, tri_id + 1);
}

static void gmio_stl_data__get_triangle(
        const void* cookie, uint32_t tri_id, gmio_stl_triangle_t* triangle)
{
    const gmio_stl_data_t* data = (const gmio_stl_data_t*)cookie;
    *triangle = data->tri_array.ptr[tri_id];
}

gmio_stl_mesh_creator_t gmio_stl_data_mesh_creator(gmio_stl_data_t *data)
{
    gmio_stl_mesh_creator_t creator = {0};
    creator.cookie = data;
    creator.func_ascii_begin_solid = &gmio_stl_data__ascii_begin_solid;
    creator.func_binary_begin_solid = &gmio_stl_data__binary_begin_solid;
    creator.func_add_triangle = &gmio_stl_data__add_triangle;
    return creator;
}

gmio_stl_mesh_t gmio_stl_data_mesh(const gmio_stl_data_t *data)
{
    gmio_stl_mesh_t mesh = {0};
    mesh.cookie = data;
    mesh.triangle_count = data->tri_array.count;
    mesh.func_get_triangle = &gmio_stl_data__get_triangle;
    return mesh;
}

void gmio_stlb_header_to_printable_string(
        const gmio_stlb_header_t *header, char *str, char replacement)
{
    size_t i = 0;
    for (; i < GMIO_STLB_HEADER_SIZE; ++i) {
        str[i] = isprint((int)header->data[i]) ?
                    (char) header->data[i] :
                    replacement;
    }
    str[GMIO_STLB_HEADER_SIZE] = 0;
}

gmio_bool_t gmio_stl_coords_equal(
        const gmio_stl_coords_t *lhs,
        const gmio_stl_coords_t *rhs,
        uint32_t max_ulp_diff)
{
    return gmio_float32_equals_by_ulp(lhs->x, rhs->x, max_ulp_diff)
            && gmio_float32_equals_by_ulp(lhs->y, rhs->y, max_ulp_diff)
            && gmio_float32_equals_by_ulp(lhs->z, rhs->z, max_ulp_diff);
}

gmio_bool_t gmio_stl_triangle_equal(
        const gmio_stl_triangle_t *lhs,
        const gmio_stl_triangle_t *rhs,
        uint32_t max_ulp_diff)
{
    return gmio_stl_coords_equal(&lhs->normal, &rhs->normal, max_ulp_diff)
            && gmio_stl_coords_equal(&lhs->v1, &rhs->v1, max_ulp_diff)
            && gmio_stl_coords_equal(&lhs->v2, &rhs->v2, max_ulp_diff)
            && gmio_stl_coords_equal(&lhs->v3, &rhs->v3, max_ulp_diff)
            && lhs->attribute_byte_count == rhs->attribute_byte_count;
}