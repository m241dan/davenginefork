function onCall( account, argument )
   local nanny = Nanny.new( "../scripts/nannys/nannycharcreate.lua" )
   local races = "[ "
   nanny:setState( 0 )
   nanny:setControl( account )
   account:echoAt( "You begin character creation...\n" )
   account:echoAt( "What race would you like?\n" )
   for index, race in pairs( race_table ) do
      races = races .. race[1] .. " "
   end
   races = races .. "]\n"
   account:echoAt( races )
   nanny:start()
end
