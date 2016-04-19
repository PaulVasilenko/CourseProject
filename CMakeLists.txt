cmake_minimum_required(VERSION 3.3)
project(CourseProjectServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES main.cpp lib/UnixSocket/UnixSocket.cpp lib/UnixSocket/UnixSocket.h lib/Connection/Connection.cpp lib/Connection/Connection.h lib/Thread/Thread.cpp lib/Thread/Thread.h lib/Thread/Runnable.h lib/RequestParser/RequestParser.cpp lib/RequestParser/RequestParser.h lib/Utils/Utils.cpp lib/Utils/Utils.h)
add_executable(CourseProjectServer ${SOURCE_FILES} lib/Connection/Connection.cpp lib/Connection/Connection.h)