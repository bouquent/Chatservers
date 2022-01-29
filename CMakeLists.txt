cmake_minimum_required(VERSION 2.5)
project(chat)

# 配置编译选项  -g
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 可执行文件最后的生成路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 配置头文件的搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)
include_directories(${PROJECT_SOURCE_DIR}/include/client)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)

# 加载子目录(执行子目录里的CMakeLists)
add_subdirectory(src)