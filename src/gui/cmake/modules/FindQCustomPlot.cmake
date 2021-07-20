# - Try to find QCustomPlot
# Once done this will define
#  QCustomPlot_FOUND - System has QCustomPlot
#  QCustomPlot_INCLUDE_DIRS - The QCustomPlot include directories
#  QCustomPlot_LIBRARIES - The libraries needed to use QCustomPlot
#  QCustomPlot_DEFINITIONS - Compiler switches required for using QCustomPlot

find_package(PkgConfig)
pkg_check_modules(PC_QCustomPlot QUIET qcustomplot)
set(QCustomPlot_DEFINITIONS ${PC_QCustomPlot_CFLAGS_OTHER})

find_path(QCustomPlot_INCLUDE_DIR qcustomplot.h
		  PATH_SUFFIXES include
		  HINTS ${QCUSTOMPLOT_ROOT} $ENV{QCUSTOMPLOT_ROOT} $ENV{QCUSTOMPLOT_DIR} ${PC_QCustomPlot_INCLUDEDIR} ${PC_QCustomPlot_INCLUDE_DIRS})

find_library(QCustomPlot_LIBRARY_RELEASE NAMES qcustomplot qcustomplot2
			 PATH_SUFFIXES lib
			 HINTS ${QCUSTOMPLOT_ROOT} $ENV{QCUSTOMPLOT_ROOT} $ENV{QCUSTOMPLOT_DIR} ${PC_QCustomPlot_LIBDIR} ${PC_QCustomPlot_LIBRARY_DIRS})

find_library(QCustomPlot_LIBRARY_DEBUG NAMES qcustomplotd qcustomplotd2
			 PATH_SUFFIXES lib
			 HINTS ${QCUSTOMPLOT_ROOT} $ENV{QCUSTOMPLOT_DIR} ${PC_QCustomPlot_LIBDIR} ${PC_QCustomPlot_LIBRARY_DIRS} )

set(QCustomPlot_LIBRARY debug ${QCustomPlot_LIBRARY_DEBUG} optimized ${QCustomPlot_LIBRARY_RELEASE})

set(QCustomPlot_LIBRARIES ${QCustomPlot_LIBRARY} )
set(QCustomPlot_INCLUDE_DIRS ${QCustomPlot_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)

message(${QCustomPlot_INCLUDE_DIR})
message(${QCustomPlot_LIBRARY_RELEASE})
message(${QCustomPlot_LIBRARY_DEBUG})

find_package_handle_standard_args(QCustomPlot
								  DEFAULT_MSG
								  QCustomPlot_LIBRARY
								  QCustomPlot_INCLUDE_DIR)

mark_as_advanced(QCustomPlot_INCLUDE_DIR QCustomPlot_LIBRARY)
