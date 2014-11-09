function onDodge( attacker, dodger )
   local dodgechance = mud.getGlobal( "BonusDodge" )

   dodgechance = dodgechance + dodger:getStatValue( "dodge" )
  
   return dodgechance
end

function onMeleeAttack( attacker, damage ) -- design the damage object to send to the defender --
end

function onMeleeDefend( defender, damage ) -- analyze the damage object sent to defender and return how much dmg it actually does --
end
