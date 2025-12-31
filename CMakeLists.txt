cmake_minimum_required(VERSION 3.16)
project(uukOJ)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定源码
aux_source_directory(src SOURCES)

# 可执行文件
add_executable(uukOJ ${SOURCES})

# MariaDB Connector/C++ 库路径
set(MARIADB_CPP_INCLUDE_DIR "/usr/local/include/mariadb")
set(MARIADB_CPP_LIBRARY_DIR "/usr/local/lib/mariadb")

# 包含头文件
target_include_directories(uukOJ PRIVATE
    ${MARIADB_CPP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/entities
    ${CMAKE_CURRENT_SOURCE_DIR}/mapper
)

# 链接库
target_link_directories(uukOJ PRIVATE ${MARIADB_CPP_LIBRARY_DIR})
target_link_libraries(uukOJ PRIVATE mariadbcpp)

set_target_properties(uukOJ PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "${MARIADB_CPP_LIBRARY_DIR}"
)
