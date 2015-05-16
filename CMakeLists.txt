cmake_minimum_required(VERSION 2.8)

project(midi_matrix.lv2)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/eo_ui.lv2)

set(DEST lib/lv2/midi_matrix.lv2)
if(WIN32)
	set(LIB_EXT ".dll")
elseif(APPLE)
	set(LIB_EXT ".so")
else()
	set(LIB_EXT ".so")
endif()

find_package(PkgConfig) # ${PKG_CONFIG_FOUND}

pkg_search_module(LV2 REQUIRED lv2>=1.10.0)
include_directories(${LV2_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LV2_LDFLAGS})

pkg_search_module(ELM REQUIRED elementary>=1.8)
include_directories(${ELM_INCLUDE_DIRS})
set(LIBS_UI ${LIBS_UI} ${ELM_LDFLAGS})

pkg_search_module(ECORE_X OPTIONAl ecore-x)

if(${ECORE_X_FOUND} AND ((${ELM_VERSION} VERSION_EQUAL "1.13.0") OR (${ELM_VERSION} VERSION_GREATER "1.13.0")))
	set(X11_UI_WRAP "")
	add_definitions("-DX11_UI_WRAP")
else()
	set(X11_UI_WRAP "#")
endif()

add_library(midi_matrix MODULE
	midi_matrix.c
	midi_matrix_channel_filter.c)
target_link_libraries(midi_matrix ${LIBS})
set_target_properties(midi_matrix PROPERTIES PREFIX "")
install(TARGETS midi_matrix DESTINATION ${DEST})

add_library(midi_matrix_ui MODULE
	midi_matrix_ui.c
	midi_matrix_channel_filter_ui.c)
target_link_libraries(midi_matrix_ui ${LIBS_UI})
set_target_properties(midi_matrix_ui PROPERTIES PREFIX "")
install(TARGETS midi_matrix_ui DESTINATION ${DEST})

configure_file("${PROJECT_SOURCE_DIR}/manifest.ttl.in" "${PROJECT_BINARY_DIR}/manifest.ttl")

find_program (EDJE_CC NAMES edje_cc)
if (EDJE_CC_NOTFOUND)
  message (SEND_ERROR "edje_cc not found")
else (EDJE_CC_NOTFOUND)
  message (STATUS "edje_cc found: " ${EDJE_CC})
endif (EDJE_CC_NOTFOUND)

add_custom_command(
	OUTPUT "${PROJECT_BINARY_DIR}/midi_matrix.edj"
	COMMAND ${EDJE_CC} ARGS
		"-fd" "${PROJECT_SOURCE_DIR}"
		"-id" "${PROJECT_SOURCE_DIR}"
		"${PROJECT_SOURCE_DIR}/midi_matrix.edc"
		"${PROJECT_BINARY_DIR}/midi_matrix.edj"
	MAIN_DEPENDENCY
		"${PROJECT_SOURCE_DIR}/midi_matrix.edc"
		"${PROJECT_SOURCE_DIR}/midi_matrix_channel_filter.edc")
add_custom_target(THEME ALL DEPENDS "${PROJECT_BINARY_DIR}/midi_matrix.edj")

install(FILES "${PROJECT_BINARY_DIR}/manifest.ttl" DESTINATION ${DEST})
install(FILES "${PROJECT_BINARY_DIR}/midi_matrix.edj" DESTINATION ${DEST})
install(FILES "${PROJECT_SOURCE_DIR}/midi_matrix.ttl" DESTINATION ${DEST})
