function dodgeChance( attacker, dodger )
   return 0
end

function parryChance( attacker, parryr )
   return 0
end

function missChance( attacker, victim )
   return 0
end

-- calculate any factors effecting melee cooldowns
function meleeCooldown( attacker )
   return automelee_delay
end

-- check to see if the attacker can atk the victim, ie same room? can see? etc etc --
-- return anything other than a string if you can attack, otherwise return the reason --
-- for example "%s is not out of range" or "%s is not here" whatever you want chummy --
function meleeCheck( attacker, victim )
   if( attacker:isSameRoom( victim ) == true ) then
      return true
   else
      local ret = string.format( "%s is not here.", vicitm:getShort() )
      return ret
   end
end

-- design the duration/frequency parts of the damage object
function prepMeleeTimer( attacker, timer )
   timer:setDuration( 1 )
   timer:setFrequency( 1 )
   timer:setCounter( 0 )
end

-- figure out the damage amount portion of the damage object
function prepMeleeDamage( attacker, damage ) 
   if( attacker:isBuilder() ) then damage:setAmount( 100 ) return end
   damage:setAmount( 1 )
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
      atk_msg = string.format( "You attack %s doing %d damage.", defender:getShort():lower(), damage:getAmount() )
      def_msg = string.format( "%s attacks you doing %d damage.", attacker:getShort():lower(), damage:getAmount() )
      room_msg = string.format( "%s attacks %s.", attacker:getShort(), defender:getShort() )
   elseif( status == HIT_DODGED ) then
   elseif( status == HIT_PARRIED ) then
   elseif( status == HIT_MISSED ) then
   end
   return atk_msg, def_msg, room_msg
end
