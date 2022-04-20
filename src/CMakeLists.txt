cmake_minimum_required(VERSION 3.10)
project(assignment C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3 -Wall -Wextra -Werror")

set(DEPEND util/linenoise.c util/tokeniser.c util/string_vec.c
  util/peek_stream.c)

set(DEPEND_SHELL task3and4/linenoise.c task3and4/external.c
  task3and4/builtin.c task3and4/interpreter.c)

add_executable(q1a task1/qa.c ${DEPEND})
add_executable(q1b task1/qb.c ${DEPEND})
add_executable(q1c task1/qc.c ${DEPEND})
add_executable(q1d task1/qd.c ${DEPEND})
add_executable(q1e task1/qe.c ${DEPEND})
add_executable(q2ab task2/qab.c ${DEPEND})

add_executable(tish task3and4/tish.c ${DEPEND_SHELL})
