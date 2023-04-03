#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define SHELL_MEM_LENGTH 1000

struct memory_struct
{
  char *var;
  char *value;
};

struct memory_struct shellmemory[SHELL_MEM_LENGTH];
int var_store_start = 0;

// Helper functions
int match(char *model, char *var)
{
  int i, len = strlen(var), matchCount = 0;
  for (i = 0; i < len; i++)
    if (*(model + i) == *(var + i))
      matchCount++;
  if (matchCount == len)
    return 1;
  else
    return 0;
}

char *extract(char *model)
{
  char token = '='; // look for this to find value
  char value[1000]; // stores the extract value
  int i, j, len = strlen(model);
  for (i = 0; i < len && *(model + i) != token; i++)
    ; // loop till we get there
  // extract the value
  for (i = i + 1, j = 0; i < len; i++, j++)
    value[j] = *(model + i);
  value[j] = '\0';
  return strdup(value);
}

// Shell memory functions

void mem_init(int frameSize, int varMemSize)
{
  // for this assignment, each frame has 3 lines
  // for now, divide memory evenly between frame and variable store
  // shellmemory is still size 1000
  // assumes variable store starts directly after frame store
  var_store_start = frameSize;

  for (int i = 0; i < 1000; i++)
  {
    shellmemory[i].var = "none";
    shellmemory[i].value = "none";
  }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in)
{
  int i;

  // only set variables in the variable store
  for (i = var_store_start; i < 1000; i++)
  {
    if (strcmp(shellmemory[i].var, var_in) == 0)
    {
      shellmemory[i].value = strdup(value_in);
      return;
    }
  }

  // Value does not exist, need to find a free spot.
  for (i = var_store_start; i < 1000; i++)
  {
    if (strcmp(shellmemory[i].var, "none") == 0)
    {
      shellmemory[i].var = strdup(var_in);
      shellmemory[i].value = strdup(value_in);
      return;
    }
  }

  return;
}

// get value based on input key
char *mem_get_value(char *var_in)
{
  int i;

  // only search variable store
  for (i = var_store_start; i < 1000; i++)
  {
    if (strcmp(shellmemory[i].var, var_in) == 0)
    {
      return strdup(shellmemory[i].value);
    }
  }
  return NULL;
}

void printShellMemory()
{
  int count_empty = 0;
  for (int i = 0; i < SHELL_MEM_LENGTH; i++)
  {
    if (strcmp(shellmemory[i].var, "none") == 0)
    {
      count_empty++;
    }
    else
    {
      printf("\nline %d: key: %s\t\tvalue: %s\n", i, shellmemory[i].var, shellmemory[i].value);
    }
  }
  printf("\n\t%d lines in total, %d lines in use, %d lines free\n\n", SHELL_MEM_LENGTH, SHELL_MEM_LENGTH - count_empty, count_empty);
}

void show_var_section()
{

  printf("%s\n","------------------------");
  printf(" var_sectionStart = %d \n", var_store_start);
  int i;
  for (i = var_store_start; i < SHELL_MEM_LENGTH; i++)
  {
    if (strcmp(shellmemory[i].var, "none") != 0)
    {
      printf("index = %d/%d  %s=%s\n", i, SHELL_MEM_LENGTH, shellmemory[i].var, shellmemory[i].value);
    }
  }

  printf("%s\n","------------------------	");
}

/*
 * Function:  addFileToMem
 * 	Added in A2
 * --------------------
 * Load the source code of the file fp into the shell memory:
 * 		Loading format - var stores fileID, value stores a line
 *		Note that the first 100 lines are for set command, the rests are for run and exec command
 *
 *  pStart: This function will store the first line of the loaded file
 * 			in shell memory in here
 *	pEnd: This function will store the last line of the loaded file
      in shell memory in here
 *  fileID: Input that need to provide when calling the function,
      stores the ID of the file

    modification -- Load into frame memory at first available spot, three lines at a time
 *
 * returns: error code, 21: no space left
 */
// int load_file(FILE *fp, int *pStart, int *pEnd, char *filename)
// {
//   char *line;
//   size_t i;
//   int error_code = 0;
//   bool hasSpaceLeft = false;
//   bool flag = true;
//   i = 101;
//   size_t candidate;

//   while (flag)
//   {
//     flag = false;
//     for (i; i < var_store_start; i++)
//     {
//       if (strcmp(shellmemory[i].var, "none") == 0)
//       {
//         *pStart = (int)i;
//         hasSpaceLeft = true;
//         break;
//       }
//     }
//     candidate = i;
//     for (i; i < var_store_start; i++)
//     {
//       if (strcmp(shellmemory[i].var, "none") != 0)
//       {
//         flag = true;
//         break;
//       }
//     }
//   }
//   i = candidate;
//   // shell memory is full
//   if (hasSpaceLeft == 0)
//   {
//     error_code = 21;
//     return error_code;
//   }

//   for (size_t j = i; j < var_store_start; j++)
//   {
//     if (feof(fp))
//     {
//       *pEnd = (int)j - 1;
//       break;
//     }
//     else
//     {
//       line = calloc(1, var_store_start);
//       fgets(line, 999, fp);
//       shellmemory[j].var = strdup(filename);
//       shellmemory[j].value = strndup(line, strlen(line));
//       free(line);
//     }
//   }

//   // no space left to load the entire file into shell memory
//   if (!feof(fp))
//   {
//     error_code = 21;
//     // clean up the file in memory
//     for (int j = 1; i <= var_store_start; i++)
//     {
//       shellmemory[j].var = "none";
//       shellmemory[j].value = "none";
//     }
//     return error_code;
//   }
//   // printShellMemory();
//   return error_code;
// }

int load_file(FILE *fp, int *pStart, int *pEnd, char *filename)
{
  char *line;
  size_t i;
  int error_code = 0;
  bool hasSpaceLeft = false;
  bool flag = true;
  i = 101;
  size_t candidate;

  while (flag)
  {
    flag = false;
    for (i; i < var_store_start; i++)
    {
      if (strcmp(shellmemory[i].var, "none") == 0)
      {
        *pStart = (int)i;
        hasSpaceLeft = true;
        break;
      }
    }
    candidate = i;
    for (i; i < var_store_start; i++)
    {
      if (strcmp(shellmemory[i].var, "none") != 0)
      {
        flag = true;
        break;
      }
    }
  }
  i = candidate;
  // shell memory is full
  if (hasSpaceLeft == 0)
  {
    error_code = 21;
    return error_code;
  }

  for (size_t j = i; j < var_store_start; j++)
  {
    if (feof(fp))
    {
      *pEnd = (int)j - 1;
      break;
    }
    else
    {
      line = calloc(1, var_store_start);
      fgets(line, 999, fp);
      shellmemory[j].var = strdup(filename);
      shellmemory[j].value = strndup(line, strlen(line));
      free(line);
    }
  }

  // no space left to load the entire file into shell memory
  if (!feof(fp))
  {
    error_code = 21;
    // clean up the file in memory
    for (int j = 1; i <= var_store_start; i++)
    {
      shellmemory[j].var = "none";
      shellmemory[j].value = "none";
    }
    return error_code;
  }
  // printShellMemory();
  return error_code;
}

// returns first free slot for a page
int get_free_page_frame()
{
  int i;
  int free_index = -1;
  int free_count = 0; // needs to reach 3 to count as a free spot

  for (i = 0; i < var_store_start; i++)
  {
    if (strcmp(shellmemory[i].var, "none") == 0)
    {
      free_count++;
    }
    else
    {
      free_count = 0;   // reset if not enough room for 3 lines
    }
    if (free_count == 3)
    {
      free_index = i;
      break;
    }
  }
  return free_index;
}

int mem_set_by_index(int index, char *var, char *value) {
  if (strcmp(shellmemory[index].value, "none") == 0) {
    shellmemory[index].var = var;
    shellmemory[index].value = value;
  } else {
    return -1;
  }
}

char *mem_get_value_at_line(int index)
{
  printf("getting line at %d\n", index);
  if (index < 0 || index > SHELL_MEM_LENGTH)
    return NULL;
  return shellmemory[index].value;
}

void mem_free_lines_between(int start, int end)
{
  for (int i = start; i <= end && i < SHELL_MEM_LENGTH; i++)
  {
    if (shellmemory[i].var != NULL)
    {
      free(shellmemory[i].var);
    }
    if (shellmemory[i].value != NULL)
    {
      free(shellmemory[i].value);
    }
    shellmemory[i].var = "none";
    shellmemory[i].value = "none";
  }
}