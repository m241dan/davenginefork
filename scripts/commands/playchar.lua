function onCall( account, argument )
   if( argument ~= nil ) then
      argument = argument:lower()
      argument:gsub( "^%l", string.upper )
      local character = account:getCharacter( argument )
      if( character == nil ) then
         account:echoAt( string.format( "You don't have a character by the name %s.\n", argument ) )
         return
      end
      playerLogin( account, character )
      return
   end
   local nanny = Nanny.new( "../scripts/nannys/nannyplaychar.lua" )
   nanny:setControl( account )
   local characters = "Your Characters:\n"
   for sheet in account:getCharacters() do
      characters = characters .. string.format( " - %s\n", sheet )
   end
   account:echoAt( characters )
   nanny:start()
end
