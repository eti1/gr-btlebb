INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_BTLEBB btlebb)

FIND_PATH(
    BTLEBB_INCLUDE_DIRS
    NAMES btlebb/api.h
    HINTS $ENV{BTLEBB_DIR}/include
        ${PC_BTLEBB_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    BTLEBB_LIBRARIES
    NAMES gnuradio-btlebb
    HINTS $ENV{BTLEBB_DIR}/lib
        ${PC_BTLEBB_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BTLEBB DEFAULT_MSG BTLEBB_LIBRARIES BTLEBB_INCLUDE_DIRS)
MARK_AS_ADVANCED(BTLEBB_LIBRARIES BTLEBB_INCLUDE_DIRS)

