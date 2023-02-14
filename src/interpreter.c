#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 7; // assuming you won't get more than 7 arguments

int badcommand()
{
	printf("%s\n", "Unknown Command");
	return 1;
}

// For run command only
int badcommandFileDoesNotExist()
{
	printf("%s\n", "Bad command: File not found");
	return 3;
}

int badCommandTooManyTokens()
{
	printf("%s\n", "Bad command: Too many tokens");
	return 4; // return int 4 for now
}

int badcommand_my_mkdir()
{
	printf("%s\n", "Bad command: my_mkdir");
	return 5;
}

int badcommand_my_cd()
{
	printf("%s\n", "Bad command: my_cd");
	return 6;
}

int help();
int quit();

int set(char *command_args[], int args_size);
int echo(char *varStr);
int my_ls();
int my_mkdir(char *dirname);
int my_touch(char *filename);
int my_cd(char *rel_path);
int dirname_comp(const void *a, const void *b);

int print(char *var);
int run(char *script);

int badcommandFileDoesNotExist();
int badCommandTooManyTokens();
int badcommand_my_mkdir();
int badcommand_my_cd();

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size)
{
	int i;

	if (args_size < 1)
	{
		return badcommand();
	}
	if (args_size > MAX_ARGS_SIZE)
	{
		return badCommandTooManyTokens();
	}

	for (i = 0; i < args_size; i++)
	{ // strip spaces new line etc
		command_args[i][strcspn(command_args[i], "\r\n")] = 0;
	}

	if (strcmp(command_args[0], "help") == 0)
	{
		// help
		if (args_size != 1)
			return badcommand();
		return help();
	}
	else if (strcmp(command_args[0], "quit") == 0)
	{
		// quit
		if (args_size != 1)
			return badcommand();
		return quit();
	}
	else if (strcmp(command_args[0], "set") == 0)
	{
		// set
		if (args_size < 3)
			return badcommand();
		return set(command_args, args_size);
	}
	else if (strcmp(command_args[0], "echo") == 0)
	{
		// echo
		if (args_size != 2)
			return badcommand();
		return echo(command_args[1]);
	}
	else if (strcmp(command_args[0], "my_ls") == 0)
	{
		// my_ls
		if (args_size != 1)
			return badcommand();
		return my_ls();
	}
	else if (strcmp(command_args[0], "my_mkdir") == 0)
	{
		// my_mkdir
		if (args_size != 2)
			return badcommand_my_mkdir();
		return my_mkdir(command_args[1]);
	}
	else if (strcmp(command_args[0], "my_touch") == 0)
	{
		// my_touch
		if (args_size != 2)
			return badcommand();
		return my_touch(command_args[1]);
	}
	else if (strcmp(command_args[0], "my_cd") == 0)
	{
		// my_cd
		if (args_size != 2)
			return badcommand_my_cd();
		return my_cd(command_args[1]);
	}
	else if (strcmp(command_args[0], "print") == 0)
	{
		if (args_size != 2)
			return badcommand();
		return print(command_args[1]);
	}
	else if (strcmp(command_args[0], "run") == 0)
	{
		if (args_size != 2)
			return badcommand();
		return run(command_args[1]);
	}
	else
		return badcommand();
}

int help()
{

	char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
run SCRIPT.TXT		Executes the file SCRIPT.TXT\n ";
	printf("%s\n", help_string);
	return 0;
}

int quit()
{
	printf("%s\n", "Bye!");
	exit(0);
}

// assigns a value to shell memory
int set(char *command_args[], int args_size)
{
	// maximum 5 tokens, tokens maximum 100 chars
	char *buffer = (char *)malloc(100 * 5 * sizeof(char));
	char *space = " ";

	for (int i = 2; i < args_size; i++)
	{
		if (i != 2)
		{
			strcat(buffer, space);
		}
		strcat(buffer, command_args[i]);
	}

	mem_set_value(command_args[1], buffer);

	// free(buffer);

	return 0;
}

int echo(char *varStr)
{
	if (varStr[0] == '$')
	{
		char *memValue = mem_get_value(varStr + 1);

		if (strcmp(memValue, "Variable does not exist") != 0)
		{
			printf("%s\n", memValue);
		}
		else
		{
			printf("%s", "\n");
		}
	}
	else
	{
		printf("%s\n", varStr);
	}
	return 0;
}

int dirname_comp(const void *a, const void *b)
{
	// numbers have lower ascii codes than letters
	// and uppercase letters have lower ascii codes than lowercase
	int return_cmp = strcmp(*(const char **)a, *(const char **)b);

	if (isalpha((*(const char **)a)[0]) || isalpha((*(const char **)b)[0]))
	{
		// if starting with different letters, sort by lowercase
		int lowercase_comp = tolower((*(const char **)a)[0]) - tolower((*(const char **)b)[0]);
		if (lowercase_comp != 0)
		{
			return_cmp = lowercase_comp;
		}

		// if starting with the same letters, lowercase first
		int same_letter_case_comp = (*(const char **)a)[0] - (*(const char **)b)[0];
		if ((lowercase_comp == 0) && (same_letter_case_comp != 0))
		{
			return_cmp = same_letter_case_comp;
		}
	}

	return return_cmp;
}

// lists all files and folders in current working directory
// code inspiration taken from https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
int my_ls()
{
	DIR *curr_directory = opendir(".");
	struct dirent *dir;

	if (curr_directory)
	{
		// assumes that there will be no more than 100 files/folders per directory
		// assumes that each file/folder name is < 100 characters
		char *dir_names[100 * 100];
		int num_dir_children = 0;

		while ((dir = readdir(curr_directory)) != NULL)
		{
			if (num_dir_children == 99)
			{
				break;
			}
			if ((strcmp(dir->d_name, ".") == 0) || (strcmp(dir->d_name, "..") == 0))
			{
				continue; // ignore . and ..
			}
			// add all directory file/folder names to string array
			dir_names[num_dir_children] = dir->d_name;
			num_dir_children++;
		}
		qsort(dir_names, num_dir_children, sizeof(char *), dirname_comp);

		for (int n = 0; n < num_dir_children; n++)
		{
			printf("%s\n", dir_names[n]);
		}

		closedir(curr_directory); // array accesses stop working if directory closed
	}

	return 0;
}

int sort_ls(char *dir_names[], int num_dir_children)
{
	// bubble sort
	for (int i = 0; i < num_dir_children; i++)
	{
		for (int j = 0; j < num_dir_children; j++)
		{
		}
	}
}

int my_mkdir(char *dirname)
{
	if (dirname[0] == '$') // argument is variable
	{
		char *memDirname = mem_get_value(dirname + 1);
		if ((strcmp(memDirname, "Variable does not exist") != 0) && (strchr(memDirname, ' ') == NULL)) // check that var exists and is single token
		{
			mkdir(memDirname, 0700);
		}
		else
		{
			return badcommand_my_mkdir();
		}
	}
	else // argument is string, use as new directory name
	{
		mkdir(dirname, 0700);
	}
	return 0;
}

// create a new file in the current working directory with the given filename
int my_touch(char *filename)
{
	FILE *newfile = fopen(filename, "w");
	fclose(newfile);
	return 0;
}

// navigate directory with relative paths
int my_cd(char *rel_path)
{
	if (chdir(rel_path) != 0)
	{
		return badcommand_my_cd();
	}
	return 0;
}

int print(char *var)
{
	printf("%s\n", mem_get_value(var));
	return 0;
}

int run(char *script)
{
	int errCode = 0;
	char line[1000];
	FILE *p = fopen(script, "rt"); // the program is in a file

	if (p == NULL)
	{
		return badcommandFileDoesNotExist();
	}

	fgets(line, 999, p);
	while (1)
	{
		errCode = parseInput(line); // which calls interpreter()
		memset(line, 0, sizeof(line));

		if (feof(p))
		{
			break;
		}
		fgets(line, 999, p);
	}

	fclose(p);

	return errCode;
}
