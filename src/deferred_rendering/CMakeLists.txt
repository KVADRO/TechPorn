cmake_minimum_required(VERSION 3.6)

project(deferred_rendering)

add_executable(${PROJECT_NAME} 
	main.cpp
	VertexShader.h
	VertexShader.cpp
	FragmentShader.h
	FragmentShader.cpp
	Mesh.h
	Mesh.cpp)

set(EXTERNAL_PATH ${CMAKE_SOURCE_DIR}/../external/)

if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++17 -stdlib=libc++")
	#set(CMAKE_XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

	set_target_properties(${PROJECT_NAME} PROPERTIES
			XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

elseif(WIN32)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
	message(FATAL_ERROR "Unsupported platform")
endif()

add_subdirectory(${EXTERNAL_PATH}/SDL external/SDL)
#add_subdirectory(${EXTERNAL_PATH}/DirectXMath external/DirectXMath)
add_subdirectory(${EXTERNAL_PATH}/tinyobjloader external/tinyobjloader)

target_compile_definitions(${PROJECT_NAME} PUBLIC SDL_MAIN_HANDLED)

target_link_libraries(${PROJECT_NAME} 
	# SDL2
	PRIVATE SDL2main
	PRIVATE SDL2-static
	
	# TinyOBJ
	tinyobjloader
	
	# DXMath
#	DirectXMath
	
	# DX
#	PRIVATE dxgi
#	PRIVATE d3d11
#	PRIVATE D3DCompiler)
	)

if(APPLE)
	find_library(OPENGL OpenGL)

	if(NOT OPENGL)
		message(FATAL_ERROR "Can't find OpenGL framework")
	endif()

	target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL})
endif()

target_include_directories(${PROJECT_NAME}
	PRIVATE ${EXTERNAL_PATH}/SDL/include
	PRIVATE ${EXTERNAL_PATH}/glm
#	PRIVATE ${EXTERNAL_PATH}/DirectXMath/Inc
	PRIVATE ${EXTERNAL_PATH}/tinyobjloader/)
	
#set_property(
#	TARGET ${PROJECT_NAME} 
#	PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	
#add_custom_command(
#	TARGET ${PROJECT_NAME} POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/vs_plain_color.hlsl ${CMAKE_CURRENT_BINARY_DIR}/vs_plain_color.hlsl
#	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/ps_plain_color.hlsl ${CMAKE_CURRENT_BINARY_DIR}/ps_plain_color.hlsl)