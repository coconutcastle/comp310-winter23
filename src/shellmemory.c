#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct memory_struct
{
  char *var;
  char *value;
};

struct memory_struct shellmemory[1000];

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

void mem_init()
{

  int i;
  for (i = 0; i < 1000; i++)
  {
    shellmemory[i].var = "none";
    shellmemory[i].value = "none";
  }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in)
{

  int i;

  for (i = 0; i < 1000; i++)
  {
    if (strcmp(shellmemory[i].var, var_in) == 0)
    {
      shellmemory[i].value = strdup(value_in);
      return;
    }
  }

  // Value does not exist, need to find a free spot.
  for (i = 0; i < 1000; i++)
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

  for (i = 0; i < 1000; i++)
  {
    if (strcmp(shellmemory[i].var, var_in) == 0)
    {

      return strdup(shellmemory[i].value);
    }
  }
  return "Variable does not exist";
}

// find space for a size of given script
int mem_find_space(int size)
{
  int i;
  int consecutive_empty_slots = size;
  int slot_start = 0;
  // move through memory until you find a block that's the right size
  for (i = 0; i < 1000; i++)
  {
    // empty slot
    if (strcmp(shellmemory[i].var, "none") == 0)
    {
      consecutive_empty_slots--;
    }
    // if encounter full slot before finding enough space, reset to size and start over
    else
    {
      consecutive_empty_slots = size;
      slot_start = i + 1;   // make it i + 1 because i isn't actually free
    }
    if (consecutive_empty_slots == 0)
    {
      return slot_start;
    }
  }
  return -1;
}

int mem_set_command_value(int start, char *command, char *identifier)
{
  shellmemory[start].var = strdup(identifier);
  shellmemory[start].value = strdup(command);
  return 0;
}

// specifically to locate a command, use location in memory
char *mem_get_command_value(int start, int curr_instruction, char *identifier)
{
  int location = start + curr_instruction;
  if (location >= 1000)
  {
    return "Out of bounds";
  }
  if (strcmp(shellmemory[location].var, identifier) != 0)
  {
    return "Nothing for that instruction";
  }
  
  else 
  {
    return shellmemory[location].value;
  }
}

// reset a chunk of memory
int mem_clean_out_block(int start, int num_instructions)
{
  int i;
  for (i = 0; i < num_instructions; i++)
  {
    if (strcmp(shellmemory[start + i].var, "none") == 0)
    {
      printf("Missing instructions");
      return -1;
    }
    shellmemory[start + i].var = "none";
    shellmemory[start + i].value = "none";
  }
}
