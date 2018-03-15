PROJECT(echo_server)

#引用头文件目录
if(WIN32)
	INCLUDE_DIRECTORIES(
		${GSF_PATH}/3rd/Libevent/include
		${GSF_PATH}/3rd/protobuf-3.2.0/src
		${GSF_PATH}/
		${GSF_PATH}/modules
		${GSF_PATH}/3rd/lua-5.3.4/src
		${GSF_PATH}/3rd/sol2
		${GSF_PATH}/3rd/fmt
	)
endif()
if(UNIX)
	INCLUDE_DIRECTORIES(
		${GSF_PATH}/lib/libevent/include
		${GSF_PATH}/
		${GSF_PATH}/modules
		${GSF_PATH}/lib/glog/include
		${GSF_PATH}/lib/lua
		${GSF_PATH}/3rd/sol2
		${GSF_PATH}/lib/protobuf/include
		)
endif()
if(APPLE)
    INCLUDE_DIRECTORIES(
            ${GSF_PATH}/3rd/libevent-2.0.22-stable/include
            ${GSF_PATH}/
	    ${GSF_PATH}/modules
	)
endif()

#引用库目录
if(WIN32)
	LINK_DIRECTORIES(
			${GSF_PATH}/lib/win/Debug
	)
endif()
if(UNIX)
	find_package(Threads)
	LINK_DIRECTORIES(
			${GSF_PATH}/lib
			${GSF_PATH}/lib/lua
			${GSF_PATH}/lib/libevent/lib
			${GSF_PATH}/lib/glog/lib
			${GSF_PATH}/lib/protobuf/lib
	)
endif()
if(APPLE)
endif()

FILE(GLOB_RECURSE SRC "*.cpp" "*.h" "*.cc" "*.lua")
ADD_EXECUTABLE(echo_server ${SRC})

if(MSVC)
    # Use the static C library for all build types
    foreach(var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
      )
      if(${var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${var} "${${var}}")
      endif()
    endforeach()

        TARGET_LINK_LIBRARIES(echo_server libevent.lib libevent_core.lib libevent_extras.lib network.lib core.lib ws2_32.lib timer.lib glog.lib log.lib lua53.lib luaProxy.lib fmt.lib)
endif()
if(UNIX)
	if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 3.1)
		target_link_libraries(echo_server ${CMAKE_THREAD_LIBS_INIT})
	else()
		target_link_libraries(echo_server Threads::Threads)
	endif()

    TARGET_LINK_LIBRARIES(echo_server libgsf_core.a libnetwork.a libevent_core.a libevent_extra.a libtimer.a liblog.a libglog.a liblua_proxy.a liblua.a libprotobuf.a -ldl)
endif()
if(APPLE)

endif()
