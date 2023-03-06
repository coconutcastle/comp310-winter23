void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
int mem_find_space(int size);
int mem_set_command_value(int start, char *command, char *identifier);
char *mem_get_command_value(int start, int curr_instruction, char *identifier);
int mem_clean_out_block(int start, int num_instructions);
