cmake_minimum_required(VERSION 2.8)
set(PROJNAME "TestGLFW")
project(${PROJNAME} C CXX)
set(${PROJNAME}_MAJOR_VERSION 0)
set(${PROJNAME}_MINOR_VERSION 1)

#add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(src)

set(SOURCES src/test.cpp src/anim.cpp src/gbuffer.cpp src/gbuffer.h)
IF("${CMAKE_SYSTEM}" MATCHES "Windows")
set(SOURCES ${SOURCES} src/win32/glhack.cpp src/win32/glhack.h)
ENDIF()

add_executable(${PROJNAME} ${SOURCES})
IF("${CMAKE_SYSTEM}" MATCHES "Windows")
	target_link_libraries(${PROJNAME} ../glfw/build/src/${CMAKE_CFG_INTDIR}/glfw3 ${GLFW_STATIC_LIBRARIES})
	target_link_libraries(${PROJNAME} opengl32)
ELSE()
	#target_link_libraries(${PROJNAME} ${CMAKE_SOURCE_DIR}/glfw/build/src/libglfw3.a ${GLFW_STATIC_LIBRARIES} GL rt X11 Xxf86vm pthread Xrandr Xi)
	target_link_libraries(${PROJNAME} ${CMAKE_SOURCE_DIR}/glfw/build/src/libglfw3.a)
	target_link_libraries(${PROJNAME} ${GLFW_STATIC_LIBRARIES} GL rt X11 Xxf86vm pthread Xrandr Xi)
ENDIF()

