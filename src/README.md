# ECSE 427 Assignment 3 Code

**Author:** Cecile Dai

**McGill ID:** 260923343

This code uses the starter code from [the project repository](https://gitlab.cs.mcgill.ca/balmau/comp310-winter23). This assignment is built off of the A3 solution code provided.

This code has never been successfully crawled by the Autograder due to an odd issue with an infinite loop - the source of which was only very recently discovered. Therefore, I cannot say for certain that there will be no compilation errors/warnings or issues that arise remotely but not locally. 

Some other things:
- Some code is taken from sources such as StackOverflow. These have been linked and credited in the comments.
- I am unable to properly delete the backing_store directory that is created each time it is run, though the files contained within them are. I am not clear on what the issue is, as I don't have anything other than an error code. I suspect it may have to do with permissions.
- There is something odd happening in the page replacement section.
- The command line arguments for framesize and varmemsize can be set using framesize=X and varmemsize=X.

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

To run batch mode with one of the test case text files, run `./mysh < ../testcases/assignment3/<filename>.txt` .