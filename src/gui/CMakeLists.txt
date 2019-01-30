project( clTEM_gui )
cmake_minimum_required( VERSION 3.5 )

cmake_policy(SET CMP0071 NEW)
cmake_policy(SET CMP0074 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

IF(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvar -tracking")
ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)

add_definitions ( -Wall )

add_subdirectory(controls/oglviewwidget)

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

find_package (QCustomPlot REQUIRED)
if(QCUSTOMPLOT_FOUND)
    message(STATUS "QCUSTOMPLOT found (include: ${QCustomPlot_INCLUDE_DIR})")
    message(STATUS "QCUSTOMPLOT found (library: ${QCustomPlot_LIBRARIES})")
endif(QCUSTOMPLOT_FOUND)

add_definitions(${QCustomPlot_DEFINITIONS})

if(CMAKE_BUILD_TYPE MATCHES Release)
    if(WIN32)
        if(CMAKE_COMPILER_IS_GNUCXX)
            # I think this is needed to make it a full GUI app, otherwise it will show the console
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
        endif(CMAKE_COMPILER_IS_GNUCXX)
    endif(WIN32)
endif(CMAKE_BUILD_TYPE MATCHES Release)

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
        controls/imageplot/imageplotwidget.cpp
        frames/simulationframe.cpp
        frames/temframe.cpp
        frames/stemframe.cpp
        frames/cbedframe.cpp
        frames/aberrationframe.cpp
        dialogs/settings/settingsdialog.cpp
        dialogs/settings/frames/openclframe.cpp
        dialogs/settings/frames/globalsimsettingsframe.cpp
        dialogs/settings/frames/fullaberrationframe.cpp
        dialogs/settings/frames/ctemareaframe.cpp
        dialogs/settings/frames/stemareaframe.cpp
        dialogs/settings/frames/cbedareaframe.cpp
        dialogs/settings/frames/stemdetectorframe.cpp
        dialogs/settings/frames/thermalscatteringframe.cpp
        dialogs/settings/frames/arealayoutframe.cpp
        utils/stringutils.cpp
        utils/guiutils.cpp
        controls/editunitsbox.cpp
        controls/borderlesswindow.cpp
        controls/borderlessdialog.cpp)

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
        dialogs/settings/frames/globalsimsettingsframe.ui
        dialogs/settings/frames/fullaberrationframe.ui
        dialogs/settings/frames/stemdetectorframe.ui
        dialogs/settings/frames/cifcreatorframe.ui
        dialogs/settings/frames/ctemareaframe.ui
        dialogs/settings/frames/stemareaframe.ui
        dialogs/settings/frames/cbedareaframe.ui
        dialogs/settings/frames/arealayoutframe.ui
        dialogs/settings/frames/thermalscatteringframe.ui
        )

IF(WIN32)
    # add the ui stuff for themes, if we can set them (aka on windows)
    set ( clTEM_UIS ${clTEM_UIS} dialogs/settings/frames/generalsettingsframe.ui )
    set ( clTEM_SRCS ${clTEM_SRCS} dialogs/settings/frames/generalsettingsframe.cpp theme/thememanager.cpp controls/flattitlebar.cpp dialogs/settings/frames/cifcreatorframe.cpp dialogs/settings/frames/cifcreatorframe.h)
    #    # add the theme resources if they are needed
    set ( clTEM_RSCS theme/theme.qrc )
    QT5_ADD_RESOURCES(RSCS ${clTEM_RSCS})
ENDIF()

QT5_WRAP_UI(UIS ${clTEM_UIS})

# I need this to bypass some errors with windres and the rc files
# essentially I removed the <flags> section to achieve this
# Thanks very much to https://stackoverflow.com/questions/3526794/how-do-i-build-a-win32-app-with-a-resource-file-using-cmake-and-mingw
if(MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT
            "<CMAKE_RC_COMPILER> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif(MINGW)

add_executable ( clTEM_gui ${clTEM_SRCS} ${UIS} ${RSCS} cltem.rc)
target_include_directories(clTEM_gui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${QCustomPlot_INCLUDE_DIR} simulation glViewWidget)
target_link_libraries ( clTEM_gui  Qt5::Widgets Qt5::PrintSupport ${QCustomPlot_LIBRARY} simulation glViewWidget)

# This is for the theming. I need to get a better way of doing it instead of hard coding it.
if(WIN32)
    if(EXISTS "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/um/x64/dwmapi.lib")
        target_link_libraries( clTEM_gui "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.17763.0/um/x64/dwmapi.lib")
    elseif(EXISTS "C:/Program Files/Microsoft SDKs/Windows/v6.0A/Lib/x64/dwmapi.lib")
        target_link_libraries( clTEM_gui "C:/Program Files/Microsoft SDKs/Windows/v6.0A/Lib/x64/dwmapi.lib")
    else()
        message( SEND_ERROR "Cannot find dwmapi.lib from Windows SDK v6.0A." )
    endif()
endif()