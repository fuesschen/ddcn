
project(ddcn_service)
cmake_minimum_required(VERSION 2.4.0)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/../bin)

find_package(Boost REQUIRED COMPONENTS system)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Ariba REQUIRED)
find_package(MCPO REQUIRED)
find_package(Log4cxx REQUIRED)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter")

find_package(Qt4 COMPONENTS QtCore QtDBus REQUIRED)
include(${QT_USE_FILE})

set(SRC
	main.cpp
	CompilerNetwork.cpp
	CompilerNetworkAdaptor.cpp
	CompilerService.cpp
	CompilerServiceAdaptor.cpp
	InputOutputFilePair.cpp
	Job.cpp
	DBusStructs.cpp
	NetworkInterface.cpp
	TemporaryFile.cpp
	ToolChain.cpp
	BootstrapConfig.cpp
	NetworkNode.cpp
	ParameterParser.cpp
	LogWriter.cpp
)

set(MOC_H
	CompilerService.h
	CompilerServiceAdaptor.h
	CompilerNetwork.h
	CompilerNetworkAdaptor.h
	InputOutputFilePair.h
	Job.h
	NetworkInterface.h
	NetworkNode.h
)

QT4_WRAP_CPP(MOC_SRC ${MOC_H})

include_directories(${ARIBA_INCLUDE_DIR} ${MCPO_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${LOG4CXX_INCLUDE_DIR} ../ddcn_crypto)

add_executable(ddcn_service ${SRC} ${MOC_SRC})
target_link_libraries(ddcn_service ${QT_LIBRARIES} ${ARIBA_LIBRARY} ${PTHREAD_LIBRARY} ${MCPO_LIBRARY} ${Boost_LIBRARIES} ${LOG4CXX_LIBRARY} ddcn_crypto)

# Installation of files

install(TARGETS ddcn_service DESTINATION bin)

