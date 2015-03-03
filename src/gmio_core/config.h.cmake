/****************************************************************************
**
** GeomIO Library
** Copyright FougSys (2 Mar. 2015)
** contact@fougsys.fr
**
** This software is a reusable library whose purpose is to provide complete
** I/O support for various CAD file formats (eg. STL)
**
** This software is governed by the CeCILL-B license under French law and
** abiding by the rules of distribution of free software.  You can  use,
** modify and/ or redistribute the software under the terms of the CeCILL-B
** license as circulated by CEA, CNRS and INRIA at the following URL
** "http://www.cecill.info".
**
****************************************************************************/

/* Generated by CMake */

#ifndef GMIO_CONFIG_H_CMAKE
#define GMIO_CONFIG_H_CMAKE

#cmakedefine GMIO_HAVE_STDINT_H
#cmakedefine GMIO_HAVE_STDBOOL_H
#cmakedefine GMIO_HAVE_STRTOF_FUNC

#cmakedefine GMIO_HAVE_GCC_BUILTIN_BSWAP16_FUNC
#cmakedefine GMIO_HAVE_GCC_BUILTIN_BSWAP32_FUNC

#cmakedefine GMIO_HAVE_MSVC_BUILTIN_BSWAP_FUNC

#if defined(__APPLE__)
#  if defined(__i386__) || defined(__ppc__)
#    define GMIO_TARGET_ARCH_BIT_SIZE  32
#  elif defined(__x86_64__) || defined(__ppc64__)
#    define GMIO_TARGET_ARCH_BIT_SIZE  64
#  else
#    error "Unknown architecture!"
#  endif
#else
#  define GMIO_TARGET_ARCH_BIT_SIZE  @GMIO_TARGET_ARCH_BIT_SIZE@
#endif

#endif /* GMIO_CONFIG_H_CMAKE */
