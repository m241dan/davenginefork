require( "../scripts/settings/combat_vars" )

function dodgeChance( attacker, dodger )
   return 0
end

function parryChance( attacker, parryr )
   return 0
end

function missChance( attacker, victim )
   return 0
end

function onMeleeAttack( attacker, damage ) -- design the damage object to send to the defender --
end

function onReceiveDamage( defender, damage ) -- analyze the damage object sent to defender and return how much dmg it actually does --
end

-- return order attacker -> defender -> room
function combatMessage( attacker, defender, damage, status )
   local atk_msg
   local def_msg
   local room_msg

   if( status == HIT_SUCCESS ) then
      atk_msg = string.format( "You attack %s doing %d damage.", defender:getShort(), damage:getAmount() )
      def_msg = string.format( "%s attacks you doing %d damage.", attacker:getShort(), damage:getAmount() )
      room_msg = string.format( "%s attacks %s.", attackere:getShort(), defender:getShort() )
   elseif( status == HITDODGED ) then
   elseif( status == HITPARRIED ) then
   elseif( status = HITMISSED ) then
   end
end
