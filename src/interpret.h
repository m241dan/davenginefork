/* interpret.h written by Davenge */

extern struct typCmd account_commands[];
extern struct typCmd settings_sub_commands[];

int account_handle_cmd( ACCOUNT_DATA *account, char *arg );
COMMAND *find_loaded_command( LLIST *loaded_list, const char *command );
int load_commands( LLIST *command_list, COMMAND command_table[], int level_compare );
int copy_command( COMMAND *to_copy, COMMAND *command );
int free_command_list( LLIST *com_list );
int free_command( COMMAND *command );
