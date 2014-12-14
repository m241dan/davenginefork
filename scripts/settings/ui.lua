-- UI Stuff --
function uiPrompt( entity )
--   entity:echoAt( string.format( "<Life: %d/%d Energy: %d/%d Move: %d/%d>\n" ) ) )
   entity:echoAt( string.format( "<PowerLevel: %d/%d>\n", entity:getStatPerm( "Powerlevel" ), entity:getStatValue( "PowerLevel" ) ) )
end

function uiLook( looker, looking_at )
   if( looking_at == nil ) then
      local room = looker:getContainer()
      if( room == nil ) then return end
      looker:echoAt( string.format( "%s\n", room:getShort() ) )
      looker:echoAt( string.format( "%s\n\n", room:getDesc() ) )

      looker:echoAt( "#RExits:\n#n" )
      for exit in room:eachInventory() do
         if( exit:isExit() == true) then
            looker:echoAt( string.format( "%s :", exit:getShort() ) )
            local exit_to = exit:getExitTo()
            if( exit_to ~= nil ) then
               looker:echoAt( string.format( " %s\n", exit_to:getShort() ) )
            else
               looker:echoAt( " nowhere...\n" )
            end
         end
      end

      looker:echoAt( "\nEverything else:\n" )
      for stuff in room:eachInventory() do
         if( stuff:isExit() == false ) then
            looker:echoAt( string.format( "%s\n", stuff:getLong() ) )
         end         
      end
   end
end

function uiInventory( entity )
   entity:echoAt( "Inventory:\n" )
   for item in entity:eachInventory() do 
      entity:echoAt( string.format( " - %s\n", item:getShort() ) )
   end
   entity:echoAt( "\n" )
end

function uiScore( entity )
local pagewidth = Account.getAccount( entity ):getPagewidth()
local bar = string.rep( "+-", pagewidth / 2 )

entity:echoAt( string.format( "\n%s, The Elite Warrior\n", entity:getShort() ) )
entity:echoAt( string.format( "%s has no title!\n", entity:getShort() ) )
entity:echoAt( "+-----------------------+-----------------------+-------------------------------\n" )
entity:echoAt( string.format( "| Str: %d (%d)\t\t| Dam: %d (%d)\t\t| Zenni: \t100435\n", entity:getStatValue( "Strength" ), entity:getStatPerm( "Strength" ), entity:getStatValue( "Damroll" ), entity:getStatPerm( "Damroll" ) ) )
entity:echoAt( string.format( "| Dex: %d (%d)\t\t| Hit: %d (%d)\t\t| Items: \t1/15\n", entity:getStatValue( "Dexterity" ), entity:getStatPerm( "Dexterity"), entity:getStatValue( "Hitroll" ), entity:getStatPerm( "Hitroll" ) ) )
entity:echoAt( string.format( "| Con: %d (%d)\t\t| Eva: %d (%d)\t\t| Weight: \t210/1000\n", entity:getStatValue( "Constitution" ), entity:getStatPerm( "Constitution" ), entity:getStatValue( "Evasiveness" ), entity:getStatPerm( "Evasiveness" ) ) )
entity:echoAt( string.format( "| Int: %d (%d)\t\t| Acc: %d (%d)\t\t| Trains: \t10/110\n", entity:getStatValue( "Intelligence" ), entity:getStatPerm( "Intelligence" ), entity:getStatValue( "Accuracy" ), entity:getStatPerm( "Accuracy" ) ) )
entity:echoAt( string.format( "| Wis: %d (%d)\t\t| Spi: %d (%d)\t\t| Practices: \t2/44\n", entity:getStatValue( "Wisdom" ), entity:getStatPerm( "Wisdom" ), entity:getStatValue( "Spirit" ), entity:getStatPerm( "Spirit" ) ) )
entity:echoAt( "+-----------------------+-----------------------+-------------------------------\n" )
entity:echoAt( "| Age: 19\t\t| Pos: sitting\t\t| RPP: \t\t10\n" )
entity:echoAt( string.format( "| Race: Icer\t\t| Tier: %d\t\t| Max RPP: \t25\n", entity:getStatPerm( "Tier" ) ) )
entity:echoAt( "| Align: -999\t\t| Wimpy: 10%\t\t| RP Level: \t5\n" )
entity:echoAt( "| Gender: Male\t\t| Statlimit: 0\t\t| RP Posts: \t20\n" )
entity:echoAt( "| Income: 50k\t\t| Play Time: 95\t\t| RP Power: \t50000\n" )
entity:echoAt( "+-------------------------------------------------------------------------------\n" )
entity:echoAt( "| MKills: \t[100]\t\t    | MDeaths: \t\t [5]\n" )
entity:echoAt( "+-------------------------------------------------------------------------------\n" )
entity:echoAt( "| PKills: \t[15]\t\t    | PDeaths: \t\t [3]\n" )
entity:echoAt( "+-------------------------------------------------------------------------------\n" )
entity:echoAt( "| HBTC Count: \t[0/2]\t\t    | Spar Count: \t [0/5] \n" )
entity:echoAt( "+-------------------------------------------------------------------------------\n" )
entity:echoAt( "| Char Created: 12/01/2015 5:45PM   | Last Login: \t 12/14/2015 8:45AM\n" )
entity:echoAt( "+-------------------------------------------------------------------------------\n" )
entity:echoAt( "| System Time:  1:36PM 12/14/2015   | (UTC-05:00)Eastern Time(US & Canada)\n" )
entity:echoAt( "+-------------------------------------------------------------------------------\n" )
end
