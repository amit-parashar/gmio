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

#include "string_parse.h"

#include "helper_stream.h"

void gmio_string_stream_fwd_iterator_init(gmio_string_stream_fwd_iterator_t *it)
{
    /* Trick: declaring the buffer exhausted will actually trigger the first
     * call to gmio_stream_read() inside gmio_next_char()
     */
    it->buffer.len = 0;
    it->buffer_pos = it->buffer.max_len;
    gmio_next_char(it);
}

gmio_eat_word_error_t gmio_eat_word(
        gmio_string_stream_fwd_iterator_t *it, gmio_string_buffer_t *buffer)
{
    char* buffer_ptr = buffer->ptr;
    const size_t buffer_capacity = buffer->max_len;
    const char* stream_curr_char = NULL;
    size_t i = buffer->len;

    /* assert(buffer != NULL && buffer->ptr != NULL); */

    stream_curr_char = gmio_skip_spaces(it);
    if (stream_curr_char == NULL) { /* Empty word */
        buffer_ptr[i] = 0;
        return GMIO_EAT_WORD_ERROR_EMPTY;
    }

    do {
        buffer_ptr[i] = *stream_curr_char;
        stream_curr_char = gmio_next_char(it);
        ++i;
    } while(i < buffer_capacity
            && stream_curr_char != NULL
            && !gmio_clocale_isspace(*stream_curr_char));

    if (i < buffer_capacity) {
        buffer_ptr[i] = 0; /* End string with terminating null byte */
        buffer->len = i;
        return GMIO_EAT_WORD_ERROR_OK;
    }
    return GMIO_EAT_WORD_ERROR_CAPACITY_OVERFLOW;
}

#if 0
gmio_bool_t gmio_checked_next_chars(
        gmio_string_stream_fwd_iterator_t *it, const char *str)
{
    size_t pos = 0;
    const char* curr_char = gmio_current_char(it);
    gmio_bool_t same = curr_char != NULL && *curr_char == *str;

    while (same) {
        curr_char = gmio_next_char(it);
        same = curr_char != NULL && *curr_char == str[++pos];
    }

    return same;
}
#endif
