#ifndef SHELL_H
#define SHELL_H

int parseInput(char *ui);
int create_backing_store();
int remove_backing_store();
int copy_file(char *filename, char *new_filename);

#endif