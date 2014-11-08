function onDodge( attacker, dodger )
   local dodgechance = mud.getGlobal( "BonusDodge" )

   dodgechance = dodgechance + dodger:getStatValue( "dodge" )
  
   return dodgechance
end

function onMelee( attack, defender )
end
