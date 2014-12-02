command_table = {}

-- name, path, level --

-- account commands --
account_commands = {}
-- mobile commands --
mobile_commands = { { "test", "../scripts/commands/test.lua", 0 } }


-- assigning tables to appropriate state indexes --
command_table[1] = account_commands
command_table[9] = mobile_commands



