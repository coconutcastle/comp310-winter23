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

#if defined(X)
int frameSize = X;
#else
int frameSize = 500;
#endif

#if defined(Y)
int varMemSize = Y;
#else
int varMemSize = 500;
#endif

int MAX_USER_INPUT = 1000;
int parseInput(char ui[]);

int main(int argc, char *argv[])
{
	printf("%s\n", "Shell version 1.2 Created January 2023\n");

	char prompt = '$';				// Shell prompt
	char userInput[MAX_USER_INPUT]; // user's input stored here
	int errorCode = 0;				// zero means no error, default

	// init user input
	for (int i = 0; i < MAX_USER_INPUT; i++)
		userInput[i] = '\0';

	// init shell memory
	mem_init(frameSize, varMemSize);
	printf("Frame Store Size = %d; Variable Store Size = %d\n", frameSize, varMemSize);

	// create backing store
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
	if (opendir("backing_store") != NULL)
	{
		remove_backing_store();
	}

	if (mkdir("backing_store", 0777))
	{
		return 1;
	}
	return -1;
}

int copy_file(char *filename, char *new_filename)
{

	char filepath[200];
	// strcpy(filepath, "../testcases/assignment3/");
	// strcat(filepath, filename);

	snprintf(filepath, 200, "../testcases/assignment3/%s", filename);

	// open program file
	FILE *program = fopen(filepath, "rt");

	if (program == NULL)
	{
		return -1;
	}

	FILE *file = fopen(new_filename, "w");

	int line_count = 0;
	char prog_line[1000];

	// go through file line by line, then each line character by character - insert new line if semicolon
	while (fgets(prog_line, sizeof(prog_line), program) != NULL)
	{
		for (int i = 0; i < strlen(prog_line); i++)
		{
			if (prog_line[i] == ';')
			{
				fputs("\n", file);
				line_count++;
			}
			else
			{
				fputc(prog_line[i], file);
			}
		}
		line_count++;
	}
	fclose(file);
	fclose(program);

	return line_count;
}

// Code taken from: https://stackoverflow.com/questions/2256945/removing-a-non-empty-directory-programmatically-in-c-or-c
int remove_backing_store()
{
	const char *path = "./backing_store";
	DIR *d = opendir(path);
	size_t path_len = strlen(path);
	int r = -1;

	if (d)
	{
		struct dirent *p;

		r = 0;
		while (!r && (p = readdir(d)))
		{
			int r2 = -1;
			char *buf;
			size_t len;

			/* Skip the names "." and ".." as we don't want to recurse on them. */
			if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
				continue;

			len = path_len + strlen(p->d_name) + 2;
			buf = malloc(len);

			if (buf)
			{
				struct stat statbuf;

				snprintf(buf, len, "%s/%s", path, p->d_name);
				if (!stat(buf, &statbuf))
				{
					if (S_ISDIR(statbuf.st_mode))
						r2 = remove_backing_store(buf);
					else
						r2 = unlink(buf);
				}
				free(buf);
			}
			r = r2;
		}
		closedir(d);
	}

	if (!r)
	{
		r = rmdir(path);
	}

	return r;
}