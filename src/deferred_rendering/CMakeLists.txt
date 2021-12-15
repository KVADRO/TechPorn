cmake_minimum_required(VERSION 3.6)

project(deferred_rendering)

add_executable(${PROJECT_NAME} main.cpp)

set(EXTERNAL_PATH ${CMAKE_SOURCE_DIR}/../external/)

add_subdirectory(${EXTERNAL_PATH}/SDL external/SDL)
add_subdirectory(${EXTERNAL_PATH}/DirectXMath external/DirectXMath)
add_subdirectory(${EXTERNAL_PATH}/tinyobjloader external/tinyobjloader)

target_compile_definitions(${PROJECT_NAME} PUBLIC SDL_MAIN_HANDLED)

target_link_libraries(${PROJECT_NAME} 
	# SDL2
	PRIVATE SDL2main
	PRIVATE SDL2-static
	
	# TinyOBJ
	tinyobjloader
	
	# DXMath
	DirectXMath
	
	# DX
	PRIVATE dxgi
	PRIVATE d3d11
	PRIVATE D3DCompiler)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${EXTERNAL_PATH}/SDL/include
	PRIVATE ${EXTERNAL_PATH}/DirectXMath/Inc
	PRIVATE ${EXTERNAL_PATH}/tinyobjloader/)
	
set_property(
	TARGET ${PROJECT_NAME} 
	PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
	
#add_custom_command(
#	TARGET ${PROJECT_NAME} POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/vs_plain_color.hlsl ${CMAKE_CURRENT_BINARY_DIR}/vs_plain_color.hlsl
#	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/ps_plain_color.hlsl ${CMAKE_CURRENT_BINARY_DIR}/ps_plain_color.hlsl)