function nannyInterp( nanny, argument )
   local state = nanny:getState()
 
   if( state == 0 ) then
      for index, race in pairs( race_table ) do
         if( race[1]:lower() == argument:lower() ) then
            local framework = EntityFramework.getFramework( race[2] )
            local players_framework = EntityFramework.newInheritedFrame( framework )
            nanny:setContent( players_framework )
            nanny:setState( 1 )
            nanny:echoAt( "Please note, we allow for casing in names. What this means if you type, \"davenge\" your name will be lowercase davenge. If you want dAvenge, that could be your name. For a regular name, make sure you capitalize Davenge when you input your name.\nEnter the characters name: " )
            return
         end
      end
      nanny:echoAt( "Invalid race.\n" )
   elseif( state == 1 ) then
      if( argument == "" or argument:len() < 4 or argument:len() > 18 ) then
         nanny:echoAt( "Invalid name.\n" )
         return
      end

      local account = nanny:getControl():getAccount()
      local character = account:getCharacter( argument )
      if( character ~= nil ) then
         nanny:echoAt( "You already have a character with that name.\n" )
         return
      end
    
      local framework = nanny:getContent()
      local base_framework = framework:getInheritance()
      local id = base_framework:getID()

      -- prepare the name --
      framework:setName( argument )
      framework:setShort( argument )
      framework:setLong( string.format( "%s the %s is here.", framework:getName(), getRaceName( id ) ) )

      character = EntityInstance.new( framework )
      character:setLevel( 1 )
      character:togglePlayer()
      account:addCharacter( character )
      nanny:finish()
      account:echoAt( "Character created.\n" )
   end
end


function getRaceName( id )
   for index, race in pairs( race_table ) do
      if( race[2] == id ) then return race[1] end
   end
end
