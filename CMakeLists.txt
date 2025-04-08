cmake_minimum_required(VERSION 3.10)
project(AgentBasedMarketSimulator)

set(CMAKE_CXX_STANDARD 20)

# Compiler-specific flags
if (MSVC)
    # Use warning level 4 for MSVC
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

include_directories(include)

file(GLOB CORE_SRC "src/core/*.cpp")
file(GLOB AGENTS_SRC "src/agents/*.cpp")
file(GLOB UTILS_SRC "src/utils/*.cpp")

add_executable(adversarial_sim
    main.cpp
    ${CORE_SRC}
    ${AGENTS_SRC}
    ${UTILS_SRC}
)

