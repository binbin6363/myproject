cmake_minimum_required(VERSION 2.6)

#INCLUDE(versioninfo.cmake)

#INCLUDE_DIRECTORIES(../../../imolib_scrpts/turtle_lib/trunk/src/imolib/socket/comm/include/)
INCLUDE_DIRECTORIES(../../../thirdparty/imo_net)
INCLUDE_DIRECTORIES(../../../thirdparty/net_manager/src)
INCLUDE_DIRECTORIES(../../../thirdparty/utils/src/)
INCLUDE_DIRECTORIES(../../../imolib_scrpts/turtle_lib/trunk/src/imolib/socket/comm/include/)
#INCLUDE_DIRECTORIES(../../../thirdparty/imo_net)

INCLUDE_DIRECTORIES(.)


FIND_LIBRARY(NETMANAGER_LIB net_manager  ../../../thirdparty/net_manager/src/ NO_DEFAULT_PATH)
FIND_LIBRARY(IMO_NET_LIB imo_net ../../../thirdparty/imo_net NO_DEFAULT_PATH)
FIND_LIBRARY(COMM_LIB comm ../../../imolib_scrpts/turtle_lib/trunk/src/imolib/socket/comm/lib NO_DEFAULT_PATH)
FIND_LIBRARY(UTILS_LIB utils ../../../thirdparty/utils/src/ NO_DEFAULT_PATH)
FIND_LIBRARY(PTHREAD_LIB pthread /usr/lib /usr/local/lib NO_DEFAULT_PATH)
FIND_LIBRARY(RUNTIME_LIB rt /usr/lib  /usr/local/lib NO_DEFAULT_PATH)

#link_libraries(${UTILS_LIB} ${NETMANAGER_LIB} ${LIBCONFIG} ${IMO_UTILS_LIB})
#link_libraries( ${NETMANAGER_LIB} ${UTILS_LIB} ${COMM_LIB} ${IMO_NET_LIB}
#    ${PTHREAD_LIB} ${RUNTIME_LIB})

link_libraries(${NETMANAGER_LIB} ${UTILS_LIB} ${COMM_LIB} ${IMO_NET_LIB}
           ${PTHREAD_LIB} ${RUNTIME_LIB} iconv)

ADD_DEFINITIONS(
-O3 -g -W -Wall
 -Wunused-variable -Wunused-parameter -Wunused-function -Wunused
 -Wno-deprecated -Woverloaded-virtual -Wwrite-strings
 -D__WUR= -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DTIXML_USE_STL
)



add_library(lib_send
	send_cmd.cpp
	codeconverter.cpp
    global.cpp
    md5.cpp
)

link_libraries(lib_send)
add_executable(mobi_send
	global.cpp
	)
