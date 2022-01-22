aux_source_directory(. SRC_LIST)
aux_source_directory(./model SRC_LIST1)
aux_source_directory(./db SRC_LIST2)

add_executable(chatserver ${SRC_LIST} ${SRC_LIST1} ${SRC_LIST2})

target_link_libraries(chatserver mymuduo pthread mysqlclient)