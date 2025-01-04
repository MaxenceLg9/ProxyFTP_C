cmake_minimum_required(VERSION 3.30)
project(proxy C)

# Set the C standard to C23
set(CMAKE_C_STANDARD 23)

# Create a library for simpleSocketAPI
add_library(apisocket simpleSocketAPI.c)

# Create an executable target for the proxy
add_executable(proxy proxy.c
        proxyapi.c
        proxyapi.h
        simpleSocketAPI.c
        simpleSocketAPI.h)

# Link the apisocket library to the proxy executable
#target_link_libraries(proxy PRIVATE apisocket)
