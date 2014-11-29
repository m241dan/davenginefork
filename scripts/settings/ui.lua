-- UI Stuff --
function uiPrompt( entity )
   entity:echoAt( "Prompt:=>\n" )
end

function uiLook( looker, looking_at )
end

function uiInventory( entity )
   entity:echoAt( "Inventory:\n" )
   for item in entity:eachInventory() do 
      entity:echoAt( string.format( " - %s\n", item:getShort() ) )
   end
   entity:echoAt( "\n" )
end

function uiScore( entity )
end


