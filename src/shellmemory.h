#ifndef SHELLMEMORY_H
#define SHELLMEMORY_H
void mem_init(int frameSize, int varMemSize);
char *mem_get_value(char *var);
char *mem_get_command(char *var_in);
void mem_set_value(char *var, char *value);
int load_file(FILE* fp, int* pStart, int* pEnd, char* fileID);
char * mem_get_value_at_line(int index);
void mem_free_lines_between(int start, int end);
// mem_clean_out_block(int start, int num_instructions);
int get_free_page_frame();
int mem_set_by_index(int index, char *var, char *value);
void printShellMemory();
void show_var_section();
#endif