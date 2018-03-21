PROJECT(test_mysql)

#引用头文件目录
if(WIN32)
	INCLUDE_DIRECTORIES(
		${GSF_PATH}/
		${GSF_PATH}/modules
		${GSF_PATH}/3rd/lua-5.3.4/src
		${GSF_PATH}/3rd/sol2
		${GSF_PATH}/3rd/fmt
		${GSF_PATH}/3rd/mysql_connect/include
		${GSF_PATH}/3rd/redis/deps/hiredis
	)
endif()
if(UNIX)
	INCLUDE_DIRECTORIES(
		${GSF_PATH}/
		${GSF_PATH}/modules
		${GSF_PATH}/lib/glog/include
		${GSF_PATH}/lib/lua
		${GSF_PATH}/3rd/sol2
		${GSF_PATH}/lib/fmt/include
	)
endif()
if(APPLE)
	INCLUDE_DIRECTORIES(
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
		${GSF_PATH}/lib/glog/lib
		${GSF_PATH}/lib/fmt/lib
	)
endif()
if(APPLE)
endif()

FILE(GLOB_RECURSE SRC "*.cpp" "*.h")
ADD_EXECUTABLE(test_mysql ${SRC})

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

        TARGET_LINK_LIBRARIES(test_mysql ws2_32.lib libevent.lib libevent_core.lib libevent_extras.lib core.lib timer.lib glog.lib log.lib lua53.lib luaProxy.lib fmt.lib libmysql.lib Win32_Interop.lib hiredis.lib dbProxy.lib)
elseif(UNIX)
	if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 3.1)
		target_link_libraries(test_mysql ${CMAKE_THREAD_LIBS_INIT})
	else()
		target_link_libraries(test_mysql Threads::Threads)
	endif()

    TARGET_LINK_LIBRARIES(test_mysql liblog.a libglog.a libgsf_core.a liblua_proxy.a liblua.a libtimer.a libfmt.a -ldl)
elseif(APPLE)
endif()