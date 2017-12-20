PROJECT(distributed_client)

if(WIN32)
	INCLUDE_DIRECTORIES(
		${GSF_PATH}/3rd/protobuf-3.2.0/src
		${GSF_PATH}/3rd/libevent-2.0.22-stable/include
		${GSF_PATH}/
		${GSF_PATH}/modules
	)
endif()
if(UNIX)
	INCLUDE_DIRECTORIES(
		${GSF_PATH}/
		${GSF_PATH}/modules
		${GSF_PATH}/lib/libevent/include
		${GSF_PATH}/3rd/lua53/lua-5.3.4/src
		${GSF_PATH}/3rd/sol2
	)
endif()
if(APPLE)
    INCLUDE_DIRECTORIES(
		${GSF_PATH}/3rd/libevent-2.0.22-stable/include
        	${GSF_PATH}/
		${GSF_PATH}/modules
	)
endif()

if(WIN32)
	LINK_DIRECTORIES(
			${GSF_PATH}/lib/win/Debug
	)
endif()
if(UNIX)
	find_package(Threads)
	LINK_DIRECTORIES(
			${GSF_PATH}/lib/
			${GSF_PATH}/lib/libevent/lib
			${GSF_PATH}/lib/glog/lib
	)
endif()
if(APPLE)
endif()

FILE(GLOB_RECURSE SRC "*.cpp" "*.h" "*.cc")
ADD_EXECUTABLE(distributed_client ${SRC})

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

        TARGET_LINK_LIBRARIES(distributed_client libevent.lib libevent_core.lib libevent_extras.lib network.lib gsf_core.lib ws2_32.lib pbc.lib libprotobufd.lib glog.lib log.lib distributed.lib)
endif()
if(UNIX)
		TARGET_LINK_LIBRARIES(distributed_client  ${CMAKE_THREAD_LIBS_INIT} libnetwork.a libevent.a libevent_core.a libevent_extra.a libtimer.a libgsf_core.a liblog.a libglog.a liblua_proxy.a)
endif()
if(APPLE)

endif()
