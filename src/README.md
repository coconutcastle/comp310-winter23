# ECSE 427 Assignment 3 Code

**Author:** Cecile Dai

**McGill ID:** 260923343

This code uses the starter code from [the project repository](https://gitlab.cs.mcgill.ca/balmau/comp310-winter23). This assignment is built off of the A3 solution code provided.

Some other things:
- Some code is taken from sources such as StackOverflow. These have been linked and credited in the comments.
- The backing_store directory is occasionally not properly deleted, though the files contained within them are. I am not clear on what the issue is, as I don't have anything other than an error code. I suspect it may have to do with permissions.
- The command line arguments for framesize and varmemsize can be set using framesize=X and varmemsize=X.
- There are some memory leaks due to use of strdup() in the provided starter code. According to the comments on [this Ed post](https://edstem.org/us/courses/32167/discussion/2902283), so long as the program passes the tests and does not crash, we should not be getting penalized for memory leaks. I am working under this assumption.

## To Run

### Compilation

Navigate to *src* from the root folder before compiling and running.

```
cd src
make clean
make mysh
```

### Batch Mode

To run batch mode with one of the test case text files, run `./mysh < ../testcases/assignment3/<filename>.txt` .

## Testing

All the test cases were run and checked using the following commands:

make clean;make mysh framesize=18 varmemsize=10;./mysh < ../testcases/assignment3/T_tc1.txt > tc1.txt;diff -w tc1.txt ../testcases/assignment3/T_tc1_result.txt

make clean;make mysh framesize=18 varmemsize=10;./mysh < ../testcases/assignment3/T_tc2.txt > tc2.txt;diff -w tc2.txt ../testcases/assignment3/T_tc2_result.txt

make clean;make mysh framesize=21 varmemsize=10;./mysh < ../testcases/assignment3/T_tc3.txt > tc3.txt;diff -w tc3.txt ../testcases/assignment3/T_tc3_result.txt

make clean;make mysh framesize=18 varmemsize=10;./mysh < ../testcases/assignment3/T_tc4.txt > tc4.txt;diff -w tc4.txt ../testcases/assignment3/T_tc4_result.txt

make clean;make mysh framesize=6 varmemsize=10;./mysh < ../testcases/assignment3/T_tc5.txt > tc5.txt;diff -w tc5.txt ../testcases/assignment3/T_tc5_result.txt

make clean;make mysh framesize=18 varmemsize=10;./mysh < ../testcases/assignment3/T_tc6.txt > tc6.txt;diff -w tc6.txt ../testcases/assignment3/T_tc6_result.txt

make clean;make mysh framesize=18 varmemsize=10;./mysh < ../testcases/assignment3/T_tc7.txt > tc7.txt;diff -w tc7.txt ../testcases/assignment3/T_tc7_result.txt

make clean;make mysh framesize=21 varmemsize=10;./mysh < ../testcases/assignment3/T_tc8.txt > tc8.txt;diff -w tc8.txt ../testcases/assignment3/T_tc8_result.txt

make clean;make mysh framesize=18 varmemsize=10;./mysh < ../testcases/assignment3/T_tc9.txt > tc9.txt;diff -w tc9.txt ../testcases/assignment3/T_tc9_result.txt

make clean;make mysh framesize=6 varmemsize=10;./mysh < ../testcases/assignment3/T_tc10.txt > tc10.txt;diff -w tc10.txt ../testcases/assignment3/T_tc10_result.txt

The tests all pass locally using the above commands. The files in in the src folder with names `tc<test_num>.txt` contain the results of the most recent runs of the batched test commands. Since Autograder has stopped working, I cannot say whether or not this will run on the Mimi servers, or that it will compile there either. Please contact me if there are problems in this regard.

Memory leaks were checked using the command `valgrind --leak-check=yes  --show-leak-kinds=all -s ./mysh < ../testcases/assignment3/T_tc<test_number>.txt `.