cmake_minimum_required(VERSION 2.8)

project(midi_matrix.lv2)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/pugl)

set(CMAKE_FIND_FRAMEWORK FIRST)

set(CMAKE_C_FLAGS "-fdata-sections -ffunction-sections ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS "-std=gnu11 -Wextra -Wno-unused-parameter -ffast-math -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS "-Wshadow -Wimplicit-function-declaration -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes ${CMAKE_C_FLAGS}")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(CMAKE_MODULE_LINKER_FLAGS "-Wl,-z,defs ${CMAKE_MODULE_LINKER_FLAGS}")
	set(CMAKE_MODULE_LINKER_FLAGS "-Wl,-z,nodelete ${CMAKE_MODULE_LINKER_FLAGS}")
elseif(WIN32)
	set(CMAKE_C_FLAGS "-mstackrealign ${CMAKE_C_FLAGS}")
endif()

if(APPLE)
	set(CMAKE_MODULE_LINKER_FLAGS "-Wl,-dead_strip ${CMAKE_MODULE_LINKER_FLAGS}")
else()
	set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--gc-sections -Wl,-s ${CMAKE_MODULE_LINKER_FLAGS}")
endif()

add_definitions("-D_GNU_SOURCE=1") # asprintf

if(WIN32)
	set(MIDI_MATRIX_UI_TYPE "WindowsUI")
elseif(APPLE)
	set(MIDI_MATRIX_UI_TYPE "CocoaUI")
else()
	set(MIDI_MATRIX_UI_TYPE "X11UI")
endif()

file(STRINGS "VERSION" MIDI_MATRIX_VERSION)
string(REPLACE "." ";" VERSION_LIST ${MIDI_MATRIX_VERSION})
list(GET VERSION_LIST 0 MIDI_MATRIX_MAJOR_VERSION)
list(GET VERSION_LIST 1 MIDI_MATRIX_MINOR_VERSION)
list(GET VERSION_LIST 2 MIDI_MATRIX_MICRO_VERSION)
add_definitions("-DMIDI_MATRIX_VERSION=\"${MIDI_MATRIX_VERSION}\"")

if(NOT DEFINED PLUGIN_DEST)
	set(PLUGIN_DEST lib/lv2/midi_matrix.lv2)
endif()

find_package(PkgConfig) # ${PKG_CONFIG_FOUND}

pkg_search_module(LV2 REQUIRED lv2>=1.10)
include_directories(${LV2_INCLUDE_DIRS})

set(LIBS_UI m)

find_package(OpenGL)
if(${OPENGL_FOUND})
	set(LIBS_UI ${LIBS_UI} ${OPENGL_LIBRARIES})
else() # try pkg-config
	pkg_search_module(GL REQUIRED gl)
	if(${GL_FOUND})
		set(LIBS_UI ${LIBS_UI} ${GL_LDFLAGS})
	else()
		message(FATAL_ERROR "OpenGL not found")
	endif()
endif()
add_definitions("-DPUGL_HAVE_GL")

if(WIN32)
	find_library(GDI32_LIBRARY NAMES gdi32)
	if(GDI32_LIBRARY)
		set(LIBS_UI ${LIBS_UI} ${GDI32_LIBRARY})
	else()
		message(FATAL_ERROR "gdi32 library not found")
	endif()

	find_library(USER32_LIBRARY NAMES user32)
	if(USER32_LIBRARY)
		set(LIBS_UI ${LIBS_UI} ${USER32_LIBRARY})
	else()
		message(FATAL_ERROR "user32 library not found")
	endif()

	set(TAR_UI ${TAR_UI} pugl/pugl/pugl_win.cpp)

elseif(APPLE)
	find_library(COCOA_LIBRARY NAMES Cocoa)
	if(COCOA_LIBRARY)
		set(LIBS_UI ${LIBS_UI} ${COCOA_LIBRARY})
	else()
		message(FATAL_ERROR "Cocoa framework not found")
	endif()

	set(TAR_UI ${TAR_UI} pugl/pugl/pugl_osx.m)

else() # GNU/Linux
	pkg_search_module(X11 REQUIRED x11>=1.6)
	include_directories(${X11_INCLUDE_DIRS})
	set(LIBS_UI ${LIBS_UI} ${X11_LDFLAGS})

	pkg_search_module(XEXT REQUIRED xext>=1.3)
	include_directories(${XEXT_INCLUDE_DIRS})
	set(LIBS_UI ${LIBS_UI} ${XEXT_LDFLAGS})

	set(TAR_UI ${TAR_UI} pugl/pugl/pugl_x11.c)
endif()

add_library(midi_matrix MODULE
	midi_matrix.c
	midi_matrix_channel_filter.c)
set_target_properties(midi_matrix PROPERTIES PREFIX "")
if(NOT WIN32)
set_target_properties(midi_matrix PROPERTIES LINK_FLAGS "-Wl,-e,lv2_descriptor")
endif()
install(TARGETS midi_matrix DESTINATION ${PLUGIN_DEST})

add_library(midi_matrix_ui MODULE
	midi_matrix_nk.c
	midi_matrix_channel_filter_nk.c
	${TAR_UI})
target_link_libraries(midi_matrix_ui ${LIBS_UI})
set_target_properties(midi_matrix_ui PROPERTIES PREFIX "")
if(NOT WIN32)
set_target_properties(midi_matrix_ui PROPERTIES LINK_FLAGS "-Wl,-e,lv2ui_descriptor")
endif()
install(TARGETS midi_matrix_ui DESTINATION ${PLUGIN_DEST})

configure_file(${PROJECT_SOURCE_DIR}/manifest.ttl.in ${PROJECT_BINARY_DIR}/manifest.ttl)
install(FILES ${PROJECT_BINARY_DIR}/manifest.ttl DESTINATION ${PLUGIN_DEST})
install(FILES ${PROJECT_SOURCE_DIR}/midi_matrix.ttl DESTINATION ${PLUGIN_DEST})
install(FILES ${PROJECT_SOURCE_DIR}/midi_matrix_ui.ttl DESTINATION ${PLUGIN_DEST})
install(FILES ${PROJECT_SOURCE_DIR}/nuklear/extra_font/Cousine-Regular.ttf DESTINATION ${PLUGIN_DEST})
