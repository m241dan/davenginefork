/* interpret.h written by Davenge */

extern struct typCmd account_commands[];
const char *settings_desc( void *extra );
const char *chat_desc( void *extra );

extern struct typCmd settings_sub_commands[];
const char *pagewidth_desc( void *extra );
const char *chatas_desc( void *extra );

extern struct typCmd olc_commands[];

extern struct typCmd workspace_sub_commands[];

extern struct typCmd frameworks_sub_commands[];

extern struct typCmd create_eFramework_commands[];

int account_handle_cmd( ACCOUNT_DATA *account, char *arg );
int olc_handle_cmd( INCEPTION *olc, char *arg );
int eFrame_editor_handle_command( INCEPTION *olc, char *arg );

void execute_command( ACCOUNT_DATA *account, COMMAND *com, void *passed, char *arg );
COMMAND *find_loaded_command( LLIST *loaded_list, const char *command );
int load_commands( LLIST *command_list, COMMAND command_table[], int level_compare );
int copy_command( COMMAND *to_copy, COMMAND *command );
int free_command_list( LLIST *com_list );
int free_command( COMMAND *command );
