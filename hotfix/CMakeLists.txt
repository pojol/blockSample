PROJECT(hotfix)

#引用头文件目录
if(WIN32)
	INCLUDE_DIRECTORIES(
		${BLOCK_PATH}/
		${BLOCK_PATH}/modules
		${BLOCK_PATH}/3rd/lua-5.3.4/src
		${BLOCK_PATH}/3rd/sol2
		${BLOCK_PATH}/3rd/fmt
		${BLOCK_PATH}/3rd/mysql_connect/include
		${BLOCK_PATH}/3rd/redis/deps/hiredis
	)
endif()
if(UNIX)
	INCLUDE_DIRECTORIES(
		${BLOCK_PATH}/
		${BLOCK_PATH}/modules
		${BLOCK_PATH}/lib/glog/include
		${BLOCK_PATH}/lib/lua
		${BLOCK_PATH}/3rd/sol2
		${BLOCK_PATH}/lib/fmt/include
	)
endif()
if(APPLE)
	INCLUDE_DIRECTORIES(
		${BLOCK_PATH}/
		${BLOCK_PATH}/modules
    )
endif()


#引用库目录
if(WIN32)
	LINK_DIRECTORIES(
	${BLOCK_PATH}/lib/win/Debug
	)
endif()
if(UNIX)
	find_package(Threads)
	LINK_DIRECTORIES(
		${BLOCK_PATH}/lib
		${BLOCK_PATH}/lib/lua
		${BLOCK_PATH}/lib/glog/lib
		${BLOCK_PATH}/lib/fmt/lib
	)
endif()
if(APPLE)
endif()

FILE(GLOB_RECURSE SRC "*.cpp" "*.h")
ADD_EXECUTABLE(hotfix ${SRC})

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

        TARGET_LINK_LIBRARIES(hotfix ws2_32.lib libevent.lib libevent_core.lib libevent_extras.lib core.lib timer.lib glog.lib log.lib lua53.lib luaAdapter.lib fmt.lib libmysql.lib Win32_Interop.lib hiredis.lib dbProxy.lib)
elseif(UNIX)
	if ("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 3.1)
		target_link_libraries(hotfix ${CMAKE_THREAD_LIBS_INIT})
	else()
		target_link_libraries(hotfix Threads::Threads)
	endif()

    TARGET_LINK_LIBRARIES(hotfix liblog.a libglog.a libgsf_core.a liblua_proxy.a liblua.a libtimer.a libfmt.a -ldl)
elseif(APPLE)
endif()
