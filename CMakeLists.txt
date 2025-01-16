# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(PDM)

# 检测 Buildroot 环境并设置工具链
if(DEFINED ENV{BR2_EXTERNAL} OR DEFINED ENV{BR2_EXTERNAL_PATH})
    set(BUILDROOT_ENV TRUE)
endif()

if(BUILDROOT_ENV)
    # 使用 Buildroot 提供的工具链文件
    if(DEFINED ENV{BR2_TOOLCHAIN_EXTERNAL_PATH})
        set(CMAKE_TOOLCHAIN_FILE $ENV{BR2_TOOLCHAIN_EXTERNAL_PATH}/toolchain.cmake CACHE FILEPATH "Buildroot toolchain file" FORCE)
    endif()
    
    # 设置 sysroot 和其他必要的变量
    set(CMAKE_SYSROOT $ENV{STAGING_DIR_TARGET})
    set(CMAKE_FIND_ROOT_PATH $ENV{STAGING_DIR_TARGET})
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
endif()

# 如果不在 Buildroot 环境中，按照原来的逻辑设置 CROSS_COMPILE
if(NOT BUILDROOT_ENV)
    if(DEFINED CROSS_COMPILE AND NOT CROSS_COMPILE STREQUAL "")
        set(CROSS_COMPILE ${CROSS_COMPILE})
    elseif(DEFINED ENV{CROSS_COMPILE} AND NOT "$ENV{CROSS_COMPILE}" STREQUAL "")
        set(CROSS_COMPILE $ENV{CROSS_COMPILE})
    else()
        message(FATAL_ERROR "Neither CROSS_COMPILE nor environment variable CROSS_COMPILE is set.")
    endif()

    # 设置交叉编译工具链
    set(CMAKE_C_COMPILER   ${CROSS_COMPILE}gcc)
    set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
    set(CMAKE_AR           ${CROSS_COMPILE}ar CACHE FILEPATH "Archiver")
    set(CMAKE_RANLIB       ${CROSS_COMPILE}ranlib CACHE FILEPATH "Ranlib")
endif()

# 设置 C 标准
set(CMAKE_C_STANDARD 99)

# 设置输出目录（Buildroot 通常有自己的规则）
if(NOT BUILDROOT_ENV)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

# 设置安装时的 RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# 禁止安装过程中清除 RPATH
set(CMAKE_SKIP_INSTALL_RPATH FALSE)
set(CMAKE_SKIP_RPATH FALSE)

# 添加子目录
add_subdirectory(lib)
add_subdirectory(app/pdm_test)
add_subdirectory(app/websocket_server)
add_subdirectory(web)

# 移除自定义清理目标（由 Buildroot 处理）
#add_custom_target(distclean
#    COMMAND /bin/sh -c "rm -rf ${CMAKE_BINARY_DIR}/*"
#    COMMENT "Removing all files in the build directory"
#)
