PROJECT(entitys_db)

#引用头文件目录
if(WIN32)
	INCLUDE_DIRECTORIES(
			${GSF_PATH}/3rd/protobuf-3.2.0/src
			${GSF_PATH}/3rd/Libevent/include
			${GSF_PATH}/
			${GSF_PATH}/modules
			${GSF_PATH}/3rd/fmt
			${GSF_PATH}/3rd/lua-5.3.4/src
			${GSF_PATH}/3rd/sol2
			${GSF_PATH}/3rd/mysql_connect/include
			${GSF_PATH}/3rd/redis/deps/hiredis
	)
endif()
if(UNIX)
	INCLUDE_DIRECTORIES(
			${GSF_PATH}/lib/libevent/include
			${GSF_PATH}/
			${GSF_PATH}/modules
			${GSF_PATH}/lib/protobuf/include
			${GSF_PATH}/lib/glog/include	
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
	LINK_DIRECTORIES(
			${GSF_PATH}/lib
			${GSF_PATH}/lib/libevent/lib
			${GSF_PATH}/lib/glog/lib
			${GSF_PATH}/lib/protobuf/lib
	)
endif()
if(APPLE)
endif()

FILE(GLOB_RECURSE SRC "*.cpp" "*.h" "*.cc")
ADD_EXECUTABLE(entitys_db ${SRC})

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

        TARGET_LINK_LIBRARIES(entitys_db libevent.lib libevent_core.lib libevent_extras.lib network.lib core.lib ws2_32.lib libprotobufd.lib glog.lib log.lib fmt.lib lua53.lib LuaAdapter.lib distributed.lib timer.lib libmysql.lib Win32_Interop.lib hiredis.lib dbProxy.lib)
endif()
if(UNIX)
	
	if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 3.1)
		target_link_libraries(entitys_db ${CMAKE_THREAD_LIBS_INIT})
	else()
		target_link_libraries(entitys_db Threads::Threads)
	endif()
	
	target_link_libraries(entitys_db -lpthread libgsf_core.a libnetwork.a libevent.a libevent_core.a libevent_extra.a libtimer.a libprotobuf.a liblog.a libglog.a)

endif()
if(APPLE)

endif()
