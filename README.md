# CPS1012 Assignment

This repository contains the source code for all the tasks which
had to be completed for the CPS1012 assignment. CPS1012 is a
compulsory unit at the University of Malta which has to be
taken by first year Computer Science and Mathematics undergraduates
during the second semester.

## The Structure of the Assignment

```
|- .clang-format
|- .gitignore
|- README.md
|- src
|    |- task1
|    |    |- .clang-format
|    |    |- qa.c
|    |    |- qb.c
|    |    |- qc.c
|    |    |- qd.c
|    |    `- qe.c
|    |- task2
|    |    `- qab.c
|    |- task3and4
|    |    |- builtin.c
|    |    |- builtin.h
|    |    |- external.c
|    |    |- external.h
|    |    |- linenoise.c
|    |    |- linenoise.h
|    |    |- parse_execute.c
|    |    |- parse_execute.h
|    |    |- string_t.c
|    |    |- string_t.h
|    |    |- tish.c
|    |    |- tokeniser.c
|    |    |- tokeniser.h
|    |    |- token_t.c
|    |    |- token_t.h
|    |    |- token_vec_t.c
|    |    |- token_vec_t.h
|    |    `- util.h
|    |- util
|    |    |- linenoise.c
|    |    |- linenoise.h
|    |    |- peek_stream.c
|    |    |- peek_stream.h
|    |    |- string_vec.c
|    |    |- string_vec.h
|    |    |- tokeniser.c
|    |    |- tokeniser.h
|    |    `- util.h
|    |- .clang-format
|    `- CMakeLists.txt
```

`.gitignore` is being used to make sure that all the files being generated by
`cmake` during testing are not pushed to the repository.

## Build Instructions

The minimum required version for `cmake` is `3.10`.

Starting from the root of the directory, execute the following commands.

```
$ cmake -S src -B build ; cd build ; make
```

This generates 7 executables. Which are the following:

```
Task 1  Task2   Task 3

./q1a   ./q2ab  ./tish
./q1c
./q1d
./q1e
```

To execute the following binaries you must be in the build
directory and type the following command where you must
substitute `{name_of_binary}` with one of the binaries described
above.

```
build $ ./{name_of_binary}
```

E.g.

```
build $ ./q2ab
build $ ./tish
```

## Testing

Each binary has a main method which allows it to be run and
tested with random user to test if the expected behaviour is
observed.

Moreover, `valgrind` was used for profiling to ensure that no
memory is leaked.

## References

For this assignment I used the suggested line editing library
`linenoise`. The repository can be found at
[antirez/linenoise](https://github.com/antirez/linenoise).
