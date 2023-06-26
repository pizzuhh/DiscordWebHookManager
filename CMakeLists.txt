cmake_minimum_required(VERSION 3.25)
project(DiscordWebhookManager C)

set(CMAKE_C_STANDARD 11)

add_executable(DiscordWebhookManager main.c)
target_link_libraries(${PROJECT_NAME} curl)
target_link_libraries(${PROJECT_NAME} cjson)
