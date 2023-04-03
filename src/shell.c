#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "interpreter.h"
#include "shellmemory.h"
#include "pcb.h"
#include "kernel.h"
#include "shell.h"

#if defined(X) && defined(Y)
    int frameSize = X;
    int varMemSize = Y;
#else
    int frameSize = 500;
    int varMemSize = 500;
#endif

int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);

// ./mysh < ../testcases/assignment3/T_tc1.txt
// make clean;make mysh fsize=500 vsize=500
// make clean;make mysh fsize=500 vsize=500;./mysh < ../testcases/assignment3/T_tc1.txt
// make clean;make mysh fsize=500 vsize=500;./mysh < ../testcases/assignment3/T_tc7.txt
// valgrind --leak-check=yes make mysh fsize=500 vsize=500
int main(int argc, char *argv[])
{
  printf("%s\n", "Shell version 1.2 Created January 2023\n");

  char prompt = '$';              // Shell prompt
  char userInput[MAX_USER_INPUT]; // user's input stored here
  int errorCode = 0;              // zero means no error, default

  // init user input
  for (int i = 0; i < MAX_USER_INPUT; i++)
    userInput[i] = '\0';

  // init shell memory
  mem_init(frameSize, varMemSize);
  printf("Frame Store Size = %d; Variable Store Size = %d\n", frameSize, varMemSize);

  // create backing store
  // printf("%s\n","done mem init");
  create_backing_store();

  while (1)
  {
    if (isatty(fileno(stdin)))
      printf("%c ", prompt);

    char *str = fgets(userInput, MAX_USER_INPUT - 1, stdin);
    if (feof(stdin))
    {
      freopen("/dev/tty", "r", stdin);
    }

    if (strlen(userInput) > 0)
    {
      errorCode = parseInput(userInput);
      if (errorCode == -1)
        exit(99); // ignore all other errors
      memset(userInput, 0, sizeof(userInput));
    }
  }

  return 0;
}

int parseInput(char *ui)
{
  char tmp[200];
  char *words[100];
  int a = 0;
  int b;
  int w = 0; // wordID
  int errorCode;
  for (a = 0; ui[a] == ' ' && a < 1000; a++)
    ; // skip white spaces

  while (ui[a] != '\n' && ui[a] != '\0' && a < 1000 && a < strlen(ui))
  {
    while (ui[a] == ' ')
      a++;
    if (ui[a] == '\0')
      break;
    for (b = 0; ui[a] != ';' && ui[a] != '\0' && ui[a] != '\n' && ui[a] != ' ' && a < 1000; a++, b++)
      tmp[b] = ui[a];
    tmp[b] = '\0';
    if (strlen(tmp) == 0)
      continue;
    words[w] = strdup(tmp);
    if (ui[a] == ';')
    {
      w++;
      errorCode = interpreter(words, w);
      if (errorCode == -1)
        return errorCode;
      a++;
      w = 0;
      for (; ui[a] == ' ' && a < 1000; a++)
        ; // skip white spaces
      continue;
    }
    w++;
    a++;
  }
  errorCode = interpreter(words, w);
  return errorCode;
}

int create_backing_store()
{
  // printf("%s\n", "in create b store");
  char *dir = "backing_store";

  if (opendir(dir) != NULL)
  {
    remove_backing_store(dir);
  }

  if (mkdir(dir, 0777))
  {
    return 1;
  }
  else {
    return -1;
  }
}

// Code adapted from: https://stackoverflow.com/questions/2256945/removing-a-non-empty-directory-programmatically-in-c-or-c
int remove_backing_store(const char *path)
{
  DIR *dir = opendir(path);

  if (!dir) {
    return -1;
  }

  struct dirent *dir_entry;
  int index = -1;

  while (!index && (dir_entry = readdir(dir))) {
    int i2 = -1;
    char *buffer;
    size_t length;

    if (!strcmp(dir_entry -> d_name, ".") || !strcmp(dir_entry->d_name, "..")) {
      continue;
    }

    length = strlen(path) + strlen(dir_entry -> d_name) + 2;
    buffer = malloc(length);

    if (buffer) {
      struct stat statBuffer;
      if (!stat(buffer, &statBuffer)) {
        if (S_ISDIR(statBuffer.st_mode)) {
          i2 = remove_backing_store(buffer);
        } else {
          i2 = unlink(buffer);
        }
      }
      free(buffer);
    }
    index = i2;
  }
  closedir(dir);
  return index;
}