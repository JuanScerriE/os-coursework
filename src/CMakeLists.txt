cmake_minimum_required(VERSION 3.10)
project(assignment C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

add_executable(tish tish.c linenoise.c tokenizer.c token_vec.c
  peek_stream.c)
