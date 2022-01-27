#定义了LIST变量，包含编译所需的所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)

#指定生成的可执行文件
add_executable(chatserver ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})

#指定可执行文件需要链接的库文件
target_link_libraries(chatserver mymuduo pthread mysqlclient hiredis)