function dodgeChance( attacker, dodger )
   return 0
end

function parryChance( attacker, parryr )
   return 0
end

function missChance( attacker, victim )
   return 0
end

-- design the duration/frequency parts of the damage object
function onMeleeAttack( attacker, damage ) 
   if( attacker:isBuilder() ) then damage:setAmount( 100 ) return end
   damage:setAmount( 1 )
   return
end

function onAutoMeleeAttack( attacker, damage )
   
end

-- analyze the damage object sent to the defender
-- calculator any the actual damage that the object does
-- note that any elemental damage will be handled by C
function onReceiveDamage( defender, damage )
   if( defender:isBuilder() ) then damage:setAmount( 1 ) return end
end

-- return order attacker -> defender -> room
function combatMessage( attacker, defender, damage, status )
   local atk_msg
   local def_msg
   local room_msg

   if( status == HIT_SUCCESS ) then
      atk_msg = string.format( "You attack %s doing %d damage.", defender:getShort(), damage:getAmount() )
      def_msg = string.format( "%s attacks you doing %d damage.", attacker:getShort(), damage:getAmount() )
      room_msg = string.format( "%s attacks %s.", attacker:getShort(), defender:getShort() )
   elseif( status == HIT_DODGED ) then
   elseif( status == HIT_PARRIED ) then
   elseif( status == HIT_MISSED ) then
   end
   return atk_msg, def_msg, room_msg
end
