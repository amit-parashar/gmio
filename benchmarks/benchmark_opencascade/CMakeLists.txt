#############################################################################
## gmio
## Copyright Fougue (2 Mar. 2015)
## contact@fougue.pro
##
## This software is a reusable library whose purpose is to provide complete
## I/O support for various CAD file formats (eg. STL)
##
## This software is governed by the CeCILL-B license under French law and
## abiding by the rules of distribution of free software.  You can  use,
## modify and/ or redistribute the software under the terms of the CeCILL-B
## license as circulated by CEA, CNRS and INRIA at the following URL
## "http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html".
#############################################################################

set(ROOTDIR_OPENCASCADE ${CMAKE_SOURCE_DIR} CACHE PATH
    "Directory where the OpenCascade library resides")

# Returns the version number of Visual C++
function(get_msvc_vernum outVerNum)
    if(MSVC60)
        set(${outVerNum} 6 PARENT_SCOPE)
    elseif(MSVC70)
        set(${outVerNum} 7 PARENT_SCOPE)
    elseif(MSVC80)
        set(${outVerNum} 8 PARENT_SCOPE)
    elseif(MSVC90)
        set(${outVerNum} 9 PARENT_SCOPE)
    elseif(MSVC10)
        set(${outVerNum} 10 PARENT_SCOPE)
    elseif(MSVC11)
        set(${outVerNum} 11 PARENT_SCOPE)
    elseif(MSVC12)
        set(${outVerNum} 12 PARENT_SCOPE)
    endif()
endfunction()

add_executable(
    benchmark_opencascade
    main.cpp
    ${CMAKE_SOURCE_DIR}/src/gmio_support/stl_occ.cpp
    ${COMMONS_FILES})

# TODO: define only if target arch is 64b
add_definitions(-D_OCC64)

if(WIN32)
    add_definitions(-DWNT)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(
        -DHAVE_CONFIG_H
        -DHAVE_FSTREAM
        -DHAVE_IOSTREAM
        -DHAVE_IOMANIP
        -DHAVE_LIMITS_H)
endif()

include_directories(${ROOTDIR_OPENCASCADE}/inc)

# Libs
if(MSVC)
    set(_MSVC_VERNUM_ 0) # Init
    get_msvc_vernum(_MSVC_VERNUM_)
    # TODO: choose win32/win64 depending on the architecture
    set(LIBDIR_OPENCASCADE ${ROOTDIR_OPENCASCADE}/win64/vc${_MSVC_VERNUM_}/lib)
elseif((${CMAKE_SYSTEM_NAME} MATCHES "Linux") AND CMAKE_COMPILER_IS_GNUCXX)
    # TODO: choose lin32/lin64 depending on the architecture
    set(LIBDIR_OPENCASCADE ${ROOTDIR_OPENCASCADE}/lin64/gcc/lib)
endif()
#message(STATUS ${ROOTDIR_OPENCASCADE})

find_library(
    LIB_OPENCASCADE_TKERNEL TKernel ${LIBDIR_OPENCASCADE}
    DOC "Path to the TKernel import library")
find_library(
    LIB_OPENCASCADE_TKMATH TKMath ${LIBDIR_OPENCASCADE}
    DOC "Path to the TKMath import library")
find_library(
    LIB_OPENCASCADE_TKSTL TKSTL ${LIBDIR_OPENCASCADE}
    DOC "Path to the TKSTL import library")
target_link_libraries(
    benchmark_opencascade
    ${LIB_OPENCASCADE_TKERNEL}
    ${LIB_OPENCASCADE_TKMATH}
    ${LIB_OPENCASCADE_TKSTL})