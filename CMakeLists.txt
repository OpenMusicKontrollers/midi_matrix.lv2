cmake_minimum_required(VERSION 3.0.0)

project(midi_matrixplex.lv2)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/libosc)

set(DEST lib/lv2/midi_matrixplex.lv2)
set(LIB_EXT ".so")

find_package(PkgConfig) # ${PKG_CONFIG_FOUND}

pkg_search_module(LV2 REQUIRED lv2>=1.10.0)
include_directories(${LV2_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LV2_LDFLAGS})

pkg_search_module(EINA REQUIRED eina>=1.11.2)
include_directories(${EINA_INCLUDE_DIRS})
set(LIBS_UI ${LIBS_UI} ${EINA_LDFLAGS})

pkg_search_module(ECORE REQUIRED ecore>=1.11.2)
include_directories(${ECORE_INCLUDE_DIRS})
set(LIBS_UI ${LIBS_UI} ${ECORE_LDFLAGS})

pkg_search_module(EVAS REQUIRED evas>=1.11.2)
include_directories(${EVAS_INCLUDE_DIRS})
set(LIBS_UI ${LIBS_UI} ${EVAS_LDFLAGS})

pkg_search_module(ECORE_EVAS REQUIRED ecore-evas>=1.11.2)
include_directories(${ECORE_EVAS_INCLUDE_DIRS})
set(LIBS_UI ${LIBS_UI} ${ECORE_EVAS_LDFLAGS})

pkg_search_module(EDJE REQUIRED edje>=1.11.2)
include_directories(${EDJE_INCLUDE_DIRS})
set(LIBS_UI ${LIBS_UI} ${EDJE_LDFLAGS})

add_library(midi_matrixplex MODULE
	midi_matrixplex.c
	midi_matrixplex_channel.c)
target_link_libraries(midi_matrixplex ${LIBS})
set_target_properties(midi_matrixplex PROPERTIES PREFIX "")
install(TARGETS midi_matrixplex DESTINATION ${DEST})

add_library(midi_matrixplex_ui MODULE
	midi_matrixplex_ui.c
	midi_matrixplex_channel_ui.c)
target_link_libraries(midi_matrixplex_ui ${LIBS_UI})
set_target_properties(midi_matrixplex_ui PROPERTIES PREFIX "")
install(TARGETS midi_matrixplex_ui DESTINATION ${DEST})

configure_file("${PROJECT_SOURCE_DIR}/manifest.ttl.in" "${PROJECT_BINARY_DIR}/manifest.ttl")

find_program (EDJE_CC NAMES edje_cc)
if (EDJE_CC_NOTFOUND)
  message (SEND_ERROR "edje_cc not found")
else (EDJE_CC_NOTFOUND)
  message (STATUS "edje_cc found: " ${EDJE_CC})
endif (EDJE_CC_NOTFOUND)

add_custom_command(
	OUTPUT "${PROJECT_BINARY_DIR}/midi_matrixplex.edj"
	COMMAND ${EDJE_CC} ARGS
		"-fd" "${PROJECT_SOURCE_DIR}"
		"-id" "${PROJECT_SOURCE_DIR}"
		"${PROJECT_SOURCE_DIR}/midi_matrixplex.edc"
		"${PROJECT_BINARY_DIR}/midi_matrixplex.edj"
	MAIN_DEPENDENCY "${PROJECT_SOURCE_DIR}/midi_matrixplex.edc")
add_custom_target(THEME ALL DEPENDS "${PROJECT_BINARY_DIR}/midi_matrixplex.edj")

install(FILES "${PROJECT_BINARY_DIR}/manifest.ttl" DESTINATION ${DEST})
install (FILES "${PROJECT_BINARY_DIR}/midi_matrixplex.edj" DESTINATION ${DEST})
install(FILES "${PROJECT_SOURCE_DIR}/midi_matrixplex.ttl" DESTINATION ${DEST})
