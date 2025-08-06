cmake_minimum_required(VERSION 3.10)  # Use 3.10+; 3.31 is not widely available/stable
project(Server C)
set(CMAKE_C_STANDARD 11)

# Only include .c files in add_executable
add_executable(Server
        Server.c
        launch.c
        HTTPRequest.c
)

# Manually specify Homebrew paths for cJSON
target_include_directories(Server PRIVATE /opt/homebrew/include)
target_link_directories(Server PRIVATE /opt/homebrew/lib)

# Link cJSON library
target_link_libraries(Server cjson)