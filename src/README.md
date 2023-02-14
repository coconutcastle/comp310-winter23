# ECSE 427 Assignment 1 Code

**Author:** Cecile Dai

**McGill ID:** 260923343

This code uses the starter code from [the project repository](https://gitlab.cs.mcgill.ca/balmau/comp310-winter23).

Note that due to using a separate project repository than the one originally submitted and having it be corrected very late, the autograder has never successfully found this repository. The issue was unfortunately not discovered until the last day. Therefore, while the local results seem correct, I cannot say for certain that there will be no compilation errors/warnings or issues that arise remotely but not locally. 

Some other things:
- There are some memory leakage issues due to uses of `strdup()` in the starter code.
- The current implementation closes the shell if a file does not end in "quit" in batch mode. It does not go to interactive mode, but it does prevent an infinite loop.

## To Run

### Compilation

Navigate to *src* from the root folder before compiling and running.

```
cd src
make clean
make mysh
```

### Interactive Mode

Run `./mysh` .

### Batch Mode

To run batch mode with one of the test case text files, run `./mysh < ../testcases/assignment1/<filename>.txt` .