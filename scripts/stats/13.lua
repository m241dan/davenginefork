function onStatGain( mob, amount )
   local current_pl = mob:getStatValue( "PowerLevel" )
   local increased_pl = current_pl + amount
   local set_index = 1

   for index,pl in pairs( tier_table ) do
      if( increased_pl >= pl ) then
         set_index = index
      else
         break
      end
   end
   mob:setStatMod( "Tier", set_index )

end

function onStatLose( mob, amount )
   local current_pl = mob:getStatValue( "PowerLevel" )
   local decreased_pl = current_pl - amount
   local set_index = 1

   for index,pl in pairs( tier_table ) do
      if( decreased_pl >= pl ) then
         set_index = index
      else
         break
      end
   end
   mob:setStatMod( "Tier", set_index )
end
