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

/*! \file stream_pos.h
 *  Declaration of gmio_stream_pos and utility functions
 *
 *  \addtogroup gmio_core
 *  @{
 */

#ifndef GMIO_STREAM_POS_H
#define GMIO_STREAM_POS_H

#include "global.h"

enum { GMIO_STREAM_POS_COOKIE_SIZE = 32 }; /* 32 bytes */

/*! Stream position
 *
 */
struct gmio_stream_pos
{
    uint8_t cookie[GMIO_STREAM_POS_COOKIE_SIZE];
};
typedef struct gmio_stream_pos gmio_stream_pos_t;

GMIO_C_LINKAGE_BEGIN

GMIO_LIB_EXPORT gmio_stream_pos_t gmio_stream_pos_null();

GMIO_C_LINKAGE_END

#endif /* GMIO_STREAM_POS_H */
/*! @} */
