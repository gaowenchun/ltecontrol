
PROJECT(lte_weilan)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(CMAKE_SYSTEM_NAME Linux)

#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--no-as-needed -std=c++11")

#SET(CMAKE_CXX_FLAGS "-Wl,--no-as-needed -lpthread -std=c++11") 

add_definitions("-Wl,--no-as-needed -lpthread -std=c++11")

SET(CMAKE_FIND_ROOT_PATH "/opt/poky/2.5.1/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi/")

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)

SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


#SET(CMAKE_CXX_COMPILER "/opt/poky/1.5.1/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-g++")

AUX_SOURCE_DIRECTORY(. DIR_SRCS)

#find_library(PTHREAD names pthread paths /opt/poky/2.5.1/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi/)

ADD_EXECUTABLE(lte_weilan ${DIR_SRCS})
target_link_libraries(lte_weilan pthread)
target_link_libraries(lte_weilan dl)
target_link_libraries(lte_weilan log4cplus)

   
