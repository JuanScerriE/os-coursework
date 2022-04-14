cmake_minimum_required(VERSION 3.10)
project(assignment C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror")

set(DEPEND linenoise.c tokenizer.c string_vec.c peek_stream.c)

add_executable(tish tish.c ${DEPEND})
add_executable(q1a task1/qa.c ${DEPEND})
add_executable(q1b task1/qb.c ${DEPEND})
add_executable(q1c task1/qc.c ${DEPEND})
add_executable(q1d task1/qd.c ${DEPEND})
add_executable(q1e task1/qe.c ${DEPEND})
add_executable(q2ab task2/qab.c ${DEPEND})
