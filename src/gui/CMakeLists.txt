project( clTEM_gui )
cmake_minimum_required( VERSION 3.5 )
#set ( CMAKE_BUILD_TYPE Release )

cmake_policy(SET CMP0071 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvar -tracking")
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

add_definitions ( -Wall )

include_directories(simulation)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

if(Qt5Widgets_FOUND)
	message(STATUS "Found Qt version: " ${Qt5Widgets_VERSION})
endif()

include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

include_directories(${Qt5PrintSupport_INCLUDE_DIRS})
add_definitions(${Qt5PrintSupport_DEFINITIONS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} ${Qt5PrintSupport_EXECUTABLE_COMPILE_FLAGS}")

if(CMAKE_BUILD_TYPE MATCHES RELEASE)
	if(WIN32)
		if(CMAKE_COMPILER_IS_GNUCXX)
			# I think this is needed to make it a full GUI app, otherwise it will show the console
			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
		endif(CMAKE_COMPILER_IS_GNUCXX)
	endif(WIN32)
endif(CMAKE_BUILD_TYPE MATCHES RELEASE)

set ( clTEM_HDRS
	)

set ( clTEM_SRCS
		main.cpp
		mainwindow.cpp
		simulationthread.cpp
        controls/statuslayout.cpp
		controls/tabpanel.cpp
        controls/tabpanelbar.cpp
		controls/imagetab.cpp
        controls/imageplot/qcustomplot.cpp
		controls/imageplot/imageplotwidget.cpp
        frames/simulationframe.cpp
		frames/temframe.cpp
		frames/stemframe.cpp
		frames/cbedframe.cpp
		frames/aberrationframe.cpp
		dialogs/settings/settingsdialog.cpp
		dialogs/settings/frames/openclframe.cpp
		dialogs/settings/frames/globalsettingsframe.cpp
		dialogs/settings/frames/fullaberrationframe.cpp
        dialogs/settings/frames/ctemareaframe.cpp
		dialogs/settings/frames/stemareaframe.cpp
        dialogs/settings/frames/cbedareaframe.cpp
		dialogs/settings/frames/stemdetectorframe.cpp
		dialogs/settings/frames/thermalscatteringframe.cpp
		dialogs/settings/frames/arealayoutframe.cpp
        utils/stringutils.cpp utils/stringutils.h utils/guiutils.cpp utils/guiutils.h controls/editunitsbox.cpp controls/editunitsbox.h)

set ( clTEM_UIS
		mainwindow.ui
		frames/simulationframe.ui
		frames/temframe.ui
		frames/stemframe.ui
		frames/cbedframe.ui
		frames/aberrationframe.ui
		controls/imagetab.ui
        controls/statuslayout.ui
		dialogs/settings/settingsdialog.ui
		dialogs/settings/frames/openclframe.ui
        dialogs/settings/frames/globalsettingsframe.ui
		dialogs/settings/frames/fullaberrationframe.ui
		dialogs/settings/frames/stemdetectorframe.ui
        dialogs/settings/frames/ctemareaframe.ui
        dialogs/settings/frames/stemareaframe.ui
        dialogs/settings/frames/cbedareaframe.ui
		dialogs/settings/frames/arealayoutframe.ui
        dialogs/settings/frames/thermalscatteringframe.ui
	)

QT5_WRAP_UI(UIS ${clTEM_UIS})

add_executable ( clTEM_gui ${clTEM_SRCS} ${UIS})
target_include_directories(clTEM_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} simulation)
target_link_libraries ( clTEM_gui  Qt5::Widgets Qt5::PrintSupport simulation )
