cmake_minimum_required(VERSION 3.23)

project(PWTWin32 VERSION 1.0 DESCRIPTION "Shared library of Windows utils for PowerTuner" LANGUAGES CXX)

set(PROJECT_AUTHOR "kylon")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core)

configure_file(pwtWin32/Resources/Windows/win.rc.in ${CMAKE_CURRENT_SOURCE_DIR}/win.rc)

set(PROJECT_SOURCES
	pwtWin32/libExport.h
	pwtWin32/win.h
	pwtWin32/win32Svc.h
	pwtWin32/win32Svc.cpp
	pwtWin32/win32Reg.h
	pwtWin32/win32Reg.cpp
	win.rc
)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})
add_library("PWT::WIN32" ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt::Core)
target_compile_definitions(${PROJECT_NAME} PUBLIC PWTWIN32_LIBRARY) #todo should be private?
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(CheckIPOSupported)
check_ipo_supported(RESULT has_ipo OUTPUT ipo_error)
if (has_ipo)
	message(STATUS "${PROJECT_NAME}: IPO/LTO enabled")
	set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
	VERSION ${PROJECT_VERSION}
	CXX_VISIBLITY_PRESET hidden
	VISIBILITY_INLINE_HIDDEN YES
)
