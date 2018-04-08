PROJECT(BLOCK_SAMPLE)

if(WIN32)
    SET(BLOCK_PATH C:/github/block CACHE INTERNAL "block path")
endif()
if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++14 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++1z")
    set(CMAKE_EXE_LINKER_FLAGS "-lrt")
    SET(BLOCK_PATH /home/github/block CACHE INTERNAL "block path")
endif()
if(APPLE)
    set(CMAKE_CXX_FLAGS "-std=c++14 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++11")
endif()


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


#ADD_SUBDIRECTORY(distributed)
ADD_SUBDIRECTORY(timer)
ADD_SUBDIRECTORY(hello_world)
ADD_SUBDIRECTORY(echo_client)
ADD_SUBDIRECTORY(echo_server)
ADD_SUBDIRECTORY(mysql)
ADD_SUBDIRECTORY(hotfix)
ADD_SUBDIRECTORY(entitys)