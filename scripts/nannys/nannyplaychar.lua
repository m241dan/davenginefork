function nannyInterp( nanny, argument )
   local account = nanny:getControl():getAccount()
   for sheet in account:getCharacters() do
      if( argument:lower() == sheet:lower() ) then
         nanny:finish()
         playerLogin( account, account:getCharacter( sheet ) )
         return;
      end
   end
   nanny:echoAt( "Invalid selection.\n" )
end
