#ifndef FOUG_DATAX_C_LIBSTL_STLA_READ_H
#define FOUG_DATAX_C_LIBSTL_STLA_READ_H

#include "stl_global.h"
#include "stl_triangle.h"
#include "../transfer.h"

/* foug_stla_geom_input */
typedef struct
{
  void*  cookie;
  void (*begin_solid_func)     (void*, const char*); /* Optional */
  void (*process_triangle_func)(void*, uint32_t, const foug_stl_triangle_t*);
  void (*end_solid_func)       (void*); /* Optional */
} foug_stla_geom_input_t;
typedef void (*foug_stla_begin_solid_func_t)(void*, const char*);
typedef void (*foug_stla_process_triangle_func_t)(void*, uint32_t, const foug_stl_triangle_t*);

/* foug_stla_read() */
FOUG_DATAX_LIBSTL_EXPORT int foug_stla_read(foug_stla_geom_input_t* geom,
                                            foug_transfer_t*        trsf,
                                            size_t                  data_size_hint);

/* Specific error codes returned by foug_stla_read() */
#define FOUG_STLA_READ_PARSE_ERROR  1

#endif /* FOUG_DATAX_C_LIBSTL_STLA_READ_H */
