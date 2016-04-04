cmake_minimum_required(VERSION 2.8)

project(midi_matrix.lv2)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/sandbox_ui.lv2)
include_directories(${PROJECT_SOURCE_DIR}/symap)

set(CMAKE_C_FLAGS "-std=gnu11 -Wextra -Wno-unused-parameter -ffast-math -fvisibility=hidden ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS "-Wshadow -Wimplicit-function-declaration -Wmissing-prototypes -Wstrict-prototypes ${CMAKE_C_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "-Wl,-z,nodelete ${CMAKE_MODULE_LINKER_FLAGS}")
add_definitions("-D_GNU_SOURCE=1") # asprintf

set(MIDI_MATRIX_MAJOR_VERSION 0)
set(MIDI_MATRIX_MINOR_VERSION 1)
set(MIDI_MATRIX_MICRO_VERSION 6)
set(MIDI_MATRIX_VERSION "${MIDI_MATRIX_MAJOR_VERSION}.${MIDI_MATRIX_MINOR_VERSION}.${MIDI_MATRIX_MICRO_VERSION}")

set(DEST lib/lv2/midi_matrix.lv2)

find_package(PkgConfig) # ${PKG_CONFIG_FOUND}

pkg_search_module(LV2 REQUIRED lv2>=1.10)
include_directories(${LV2_INCLUDE_DIRS})

pkg_search_module(ELM REQUIRED elementary>=1.8)
include_directories(${ELM_INCLUDE_DIRS})

pkg_search_module(NANOMSG REQUIRED libnanomsg>=2.0)
include_directories(${NANOMSG_INCLUDE_DIRS})

pkg_search_module(SRATOM REQUIRED sratom-0>=0.4.0)
include_directories(${SRATOM_INCLUDE_DIRS})

pkg_search_module(LILV REQUIRED lilv-0>=0.20.0)
include_directories(${LILV_INCLUDE_DIRS})
if((${LILV_VERSION} VERSION_EQUAL "0.22.0") OR (${LILV_VERSION} VERSION_GREATER "0.22.0"))
	add_definitions("-DLILV_0_22")
endif()

add_library(midi_matrix MODULE
	midi_matrix.c
	midi_matrix_channel_filter.c)
set_target_properties(midi_matrix PROPERTIES PREFIX "")
install(TARGETS midi_matrix DESTINATION ${DEST})

add_library(midi_matrix_ui MODULE
	${PROJECT_SOURCE_DIR}/sandbox_ui.lv2/sandbox_ui.c
	${PROJECT_SOURCE_DIR}/sandbox_ui.lv2/sandbox_master.c
	midi_matrix_ui.c)
target_link_libraries(midi_matrix_ui
	${NANOMSG_LDFLAGS}
	${SRATOM_LDFLAGS})
set_target_properties(midi_matrix_ui PROPERTIES PREFIX "")
install(TARGETS midi_matrix_ui DESTINATION ${DEST})

add_library(midi_matrix_eo MODULE
	midi_matrix_eo.c
	midi_matrix_channel_filter_eo.c)
target_link_libraries(midi_matrix_eo
	${ELM_LDFLAGS})
set_target_properties(midi_matrix_eo PROPERTIES PREFIX "")
install(TARGETS midi_matrix_eo DESTINATION ${DEST})

add_executable(sandbox_efl
	${PROJECT_SOURCE_DIR}/sandbox_ui.lv2/sandbox_slave.c
	${PROJECT_SOURCE_DIR}/sandbox_ui.lv2/sandbox_efl.c
	${PROJECT_SOURCE_DIR}/symap/symap.c)
target_link_libraries(sandbox_efl
	${ELM_LDFLAGS}
	${NANOMSG_LDFLAGS}
	${LILV_LDFLAGS})
install(TARGETS sandbox_efl DESTINATION ${DEST})

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

configure_file(${PROJECT_SOURCE_DIR}/manifest.ttl.in ${PROJECT_BINARY_DIR}/manifest.ttl)
install(FILES ${PROJECT_BINARY_DIR}/manifest.ttl DESTINATION ${DEST})
install(FILES ${PROJECT_BINARY_DIR}/midi_matrix.edj DESTINATION ${DEST})
install(FILES ${PROJECT_SOURCE_DIR}/midi_matrix.ttl DESTINATION ${DEST})
install(FILES ${PROJECT_SOURCE_DIR}/midi_matrix_ui.ttl DESTINATION ${DEST})
