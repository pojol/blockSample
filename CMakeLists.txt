PROJECT(GSF_SAMPLE)

if(WIN32)

endif()
if(UNIX)
    set(CMAKE_CXX_FLAGS "-std=c++14 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++1z")
    set(CMAKE_EXE_LINKER_FLAGS "-lrt")
endif()
if(APPLE)
    set(CMAKE_CXX_FLAGS "-std=c++14 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++11")
endif()


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(GSF_PATH E:/github/gsf CACHE INTERNAL "gsf path")



ADD_SUBDIRECTORY(distributed_login)
ADD_SUBDIRECTORY(distributed_root)
ADD_SUBDIRECTORY(distributed_gate)
ADD_SUBDIRECTORY(client)