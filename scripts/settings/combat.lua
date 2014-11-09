function onDodge( attacker, dodger )
   local dodgechance = mud.getGlobal( "BonusDodge" )

   dodgechance = dodgechance + dodger:getStatValue( "dodge" )
  
   return dodgechance
end

function onMeleeAttack( attacker ) -- calculates the amount of melee damage the attacker does --
end

function onMeleeDefend( defender, damage ) -- calculates the amount of melee damage the defender receives based on the damage object passed --
end
