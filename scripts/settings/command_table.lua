command_table = {}

-- name, path, level --

account_commands = { 
                       { "test", "../scripts/commands/test.lua", 1 } -- index 1
                       { "another test", "../scripts/commands/anothertest.lua", 1 } -- index 2  
                   }
mobile_commands = {}

command_table[1] = account_commands
command_table[9] = mobile_commands



