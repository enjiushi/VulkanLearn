cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
cmake_policy(VERSION 3.5)

set(NAME VulkanLearn)

project(${NAME} CXX)

include_directories(external/vulkan)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

find_library(VULKAN_LIB NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/lib/vulkan)
find_library(ASSIMP_LIB NAMES assimp PATHS ${CMAKE_SOURCE_DIR}/lib/assimp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")


function(buildExample EXAMPLE)
	#file(GLOB PROJECT_HEADER ${EXAMPLE}/*.h)
	#file(GLOB PROJECT_SOURCE ${EXAMPLE}/*.cpp)
	file(GLOB VULKAN vulkan/*.h vulkan/*.cpp)
	file(GLOB MATHS_DEFS maths/*.h maths/*.inl maths/*.cpp)
	file(GLOB COMMON common/*.h common/*.cpp)
	file(GLOB BASE base/*.h base/*.cpp)
	IF(WIN32)
		SET(PROJECT_WIN32_SOURCE "${PROJECT_SOURCE_DIR}/Win32Entry.cpp")
	ENDIF(WIN32)
	message(STATUS ${PROJECT_SOURCE})
	add_executable(${EXAMPLE} WIN32 ${PROJECT_SOURCE} ${PROJECT_HEADER} ${PROJECT_WIN32_SOURCE} ${MATHS_DEFS} ${COMMON} ${BASE} ${VULKAN})
	source_group("maths\\" FILES ${MATHS_DEFS})
	source_group("common\\" FILES ${COMMON})
	source_group("base\\" FILES ${BASE})
	source_group("vulkan\\" FILES ${VULKAN})
	target_link_libraries(${EXAMPLE} ${VULKAN_LIB} ${ASSIMP_LIB})
endfunction(buildExample)

function(buildExamples EXAMPLES)
	foreach(EXAMPLE ${EXAMPLES})
		buildExample(${EXAMPLE})
	endforeach(EXAMPLE)
endfunction(buildExamples)

set(PROJECTS VulkanLearn)
buildExamples(${PROJECTS})