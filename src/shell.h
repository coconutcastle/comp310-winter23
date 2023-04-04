#ifndef SHELL_H
#define SHELL_H

int parseInput(char *ui);
int create_backing_store();
int remove_backing_store(const char *path);
// int remove_backing_store(const char *const path);

#endif

// #if defined(X) && defined(Y)
//     int frameSize = X;
//     int varMemSize = Y;
// #else
//     int frameSize = 500;
//     int varMemSize = 500;
// #endif