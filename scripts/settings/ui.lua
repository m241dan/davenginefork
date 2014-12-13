-- UI Stuff --
function uiPrompt( entity )
--   entity:echoAt( string.format( "<Life: %d/%d Energy: %d/%d Move: %d/%d>\n" ) ) )
   entity:echoAt( string.format( "<PowerLevel: %d/%d>\n", entity:getStatMod( "Powerlevel" ), entity:getStatPerm( "PowerLevel" ) ) )
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
entity:echoAt( string.format( "\n%s, The Elite Warrior\n", entity:getShort() ) )
entity:echoAt( string.format( "%s has no title!\n", entity:getShort() ) )
entity:echoAt( "-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n" ) 
entity:echoAt( string.format( "| Str: %d (%d)\t\t| Dam: %d (%d)\t\t| Zenni: 100435\n", entity:getStatMod( "Strength" ), entity:getStatPerm( "Strength" ), entity:getStatMod( "Damroll" ), entity:getStatPerm( "Damroll" ) ) )
entity:echoAt( "-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n" )
end


