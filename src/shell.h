#ifndef SHELL_H
#define SHELL_H

int parseInput(char *ui);
int remove_backing_store(const char *path);


#if defined(X) && defined(Y)
    int frameSize = X;
    int varMemSize = Y;
#else
    int frameSize = 500;
    int varMemSize = 500;
#endif


#endif