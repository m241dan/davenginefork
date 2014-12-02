command_table = {}

-- name, path, level --

-- account commands --
account_commands = { { "test", "../scripts/commands/test.lua", 1 },
                        { "another test", "../scripts/commands/anothertest.lua", 1 }
                      }
-- mobile commands --
mobile_commands = {}


-- assigning tables to appropriate state indexes --
command_table[1] = account_commands
command_table[9] = mobile_commands



