#ifndef FOUG_DATAX_C_LIBSTL_STLB_WRITE_H
#define FOUG_DATAX_C_LIBSTL_STLB_WRITE_H

#include "stl_global.h"
#include "stlb_triangle.h"
#include "../endian.h"
#include "../transfer.h"

/* foug_stlb_geom_output */
typedef struct foug_stlb_geom_output foug_stlb_geom_output_t;
struct foug_stlb_geom_output
{
  void*    cookie;
  uint8_t* header; /* May be NULL if empty header*/
  uint32_t triangle_count;
  void     (*get_triangle_func)(const foug_stlb_geom_output_t*, uint32_t, foug_stlb_triangle_t*);
};

/* foug_stlb_write() */
FOUG_DATAX_LIBSTL_EXPORT int foug_stlb_write(const foug_stlb_geom_output_t* geom,
                                             foug_transfer_t*               trsf,
                                             foug_endianness_t              byte_order);

/* Specific error codes returned by foug_stlb_write() */
#define FOUG_STLB_WRITE_NULL_GET_TRIANGLE_FUNC       1
#define FOUG_STLB_WRITE_UNSUPPORTED_BYTE_ORDER       2

#endif /* FOUG_DATAX_C_LIBSTL_STLB_WRITE_H */
