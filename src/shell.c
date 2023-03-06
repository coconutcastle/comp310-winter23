#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "interpreter.h"
#include "shellmemory.h"
#include "processControlBlock.h"
#include "ready_queue.h"

int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);

// ./mysh < ../testcases/assignment1/run.txt
// ./mysh < ../testcases/assignment2/echo.txt

// gcc -g -c shell.c interpreter.c shellmemory.c processControlBlock.c ready_queue.c
// gcc -o mysh shell.o interpreter.o shellmemory.o processControlBlock.o ready_queue.o
// gcc -g -01 -c shell.c interpreter.c shellmemory.c processControlBlock.c ready_queue.c;gcc -o mysh shell.o interpreter.o shellmemory.o processControlBlock.o ready_queue.o
// valgrind --leak-check=yes --log-file=valgrind.rpt ./mysh

// Start of everything
// code for one-liners inspiration taken from https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
int main(int argc, char *argv[])
{
  printf("%s\n", "Shell version 1.2 Created January 2023");
  help();

  char prompt = '$';              // Shell prompt
  char userInput[MAX_USER_INPUT]; // user's input stored here
  int errorCode = 0;              // zero means no error, default

  // for one-liners
  const char *semicolon = ";";
  char *input_token; // pointer to first token (semicolon-deliminated) in user input

  for (int i = 0; i < MAX_USER_INPUT; i++) // init user input
    userInput[i] = '\0';

  mem_init(); // init shell memory

  while (1)
  {
    // check if commands are from file (batch) or terminal (interactive)
    if (isatty(fileno(stdin)))
    {
      printf("%c ", prompt);
    }

    // file stream input ends
    // currently does not reopen terminal after file ends, but it does prevent an infinite loop
    if (feof(stdin))
    {
      break;
    }

    // when stdin is written to userinput, userinput is filled with the entered command line
    fgets(userInput, MAX_USER_INPUT - 1, stdin);

    // split user input by semicolons and execute each as a separate command
    input_token = strtok(userInput, semicolon);
    while (input_token != NULL)
    {
      // printf("eyy 1\n");
      errorCode = parseInput(input_token);
      if (errorCode == -1)
        exit(99); // ignore all other errors
      memset(input_token, 0, sizeof(input_token));
      input_token = strtok(NULL, semicolon);
    }
  }

  return 0;
}

int parseInput(char ui[])
{
  // this is really not working...

  // printf("in parse\n");
  // char tmp[200];
  // char *words[100];
  // int a = 0;
  // int b;
  // int w = 0; // wordID
  // int errorCode;
  // printf("parse 1");
  // for (a = 0; ui[a] == ' ' && a < 1000; a++)
  // 	; // skip white spaces
  // while (ui[a] != '\n' && ui[a] != '\0' && a < 1000)
  // {
  // 	for (b = 0; ui[a] != ';' && ui[a] != '\0' && ui[a] != '\n' && ui[a] != ' ' && a < 1000; a++, b++)
  // 	{
  //     printf("parse 2");
  // 		tmp[b] = ui[a];
  // 		// extract a word
  // 	}
  //   printf("parse 3");
  // 	tmp[b] = '\0';
  // 	words[w] = strdup(tmp);
  // 	w++;
  // 	if (ui[a] == '\0')
  // 		break;
  // 	a++;
  // }
  // printf("going to interpreter\n");
  // errorCode = interpreter(words, w);
  // return errorCode;

  char tmp[200];
  char *words[100];
  int a = 0;
  int b;
  int w = 0; // wordID
  int errorCode;
  for (a = 0; ui[a] == ' ' && a < 1000; a++)
    ; // skip white spaces

  while (ui[a] != '\n' && ui[a] != '\0' && a < 1000)
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
