cmake_minimum_required (VERSION 2.6)
project (HelloWorldFacebook)

add_subdirectory(../../../../oxygine-framework/ oxygine-framework)
add_definitions(${OXYGINE_DEFINITIONS})
include_directories(${OXYGINE_INCLUDE_DIRS})
link_directories(${OXYGINE_LIBRARY_DIRS})


add_subdirectory(../../../ oxygine-facebook)
include_directories(${OXYGINE_FACEBOOK_INCLUDE_DIRS})


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OXYGINE_CXX_FLAGS}")

add_executable(HelloWorldFacebook ../src/example.cpp ../src/main.cpp ../src/test.cpp  ../src/example.h ../src/test.h )
target_link_libraries(HelloWorldFacebook ${OXYGINE_CORE_LIBS} ${OXYGINE_FACEBOOK_LIBS} oxygine-facebook)



if (WIN32) #disable console mode for VC++
	set_target_properties(HelloWorldFacebook PROPERTIES WIN32_EXECUTABLE TRUE)
endif(WIN32)



if (EMSCRIPTEN)
	SET(CMAKE_EXECUTABLE_SUFFIX ".html")	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_EXIT_RUNTIME=1 -s WASM=0 -s WARN_ON_UNDEFINED_SYMBOLS=1 --memory-init-file 0 -s TOTAL_MEMORY=50331648 -s FORCE_FILESYSTEM=1")
	em_link_pre_js(HelloWorldFacebook  ${OXYGINE_JS_LIBRARIES}  ${CMAKE_CURRENT_SOURCE_DIR}/data.js)
endif(EMSCRIPTEN)