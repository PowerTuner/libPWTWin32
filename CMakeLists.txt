cmake_minimum_required(VERSION 3.23)

project(PWTWin32 VERSION 2.0 DESCRIPTION "C++ library to easy Windows api usage in apps" LANGUAGES CXX)

set(PROJECT_AUTHOR "kylon")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(src/Resources/win.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/win.rc)

add_library(${PROJECT_NAME}
    src/export.h
    src/win.h
    src/elapsedTimer.h
    src/win32Svc.h
    src/win32Svc.cpp
    src/win32Reg.h
    src/win32Reg.cpp
    win.rc
)
add_library("PWT::WIN32" ALIAS ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PUBLIC PWTWIN32_LIBRARY)

include(CheckIPOSupported)
check_ipo_supported(RESULT has_ipo OUTPUT ipo_error)
if (has_ipo)
	message(STATUS "${PROJECT_NAME}: IPO/LTO enabled")
	set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	CXX_VISIBLITY_PRESET hidden
	VISIBILITY_INLINE_HIDDEN TRUE
)
