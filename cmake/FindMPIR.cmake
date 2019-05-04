# Try to find the MPIR librairies
#  MPIR_FOUND - system has MPIR lib
#  MPIR_INCLUDE_DIR - the MPIR include directory
#  MPIR_LIBRARIES - Libraries needed to use MPIR

if (MPIR_INCLUDE_DIR AND MPIR_LIBRARIES)
    # Already in cache, be silent
    set(MPIR_FIND_QUIETLY TRUE)
endif()

find_path(MPIR_INCLUDE_DIR
    NAMES
        mpir.h
    HINTS
        ${Ark_SOURCE_DIR}/mpir-3.0.0/dll/
        ${Ark_SOURCE_DIR}/mpir-3.0.0/lib/
)
find_library(MPIR_LIBRARIES 
    NAMES
        mpir libmpir
    HINTS
        ${Ark_SOURCE_DIR}/mpir-3.0.0/dll/
        ${Ark_SOURCE_DIR}/mpir-3.0.0/lib/
)
find_library(MPIRXX_LIBRARIES
    NAMES
        mpirxx libmpirxx
    HINTS
        ${Ark_SOURCE_DIR}/mpir-3.0.0/dll/
        ${Ark_SOURCE_DIR}/mpir-3.0.0/lib/
)
MESSAGE(STATUS "MPIR libs: " ${MPIR_LIBRARIES} " " ${MPIRXX_LIBRARIES})

SET(MPIR_LIBRARIES ${MPIR_LIBRARIES} ${MPIRXX_LIBRARIES})
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MPIR DEFAULT_MSG MPIR_INCLUDE_DIR MPIR_LIBRARIES)