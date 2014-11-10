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

-- design the damage object to send to the defender
-- deal with any attacker specific factors and set the amount
-- note that elemental damage and damage types will be handled by C
function onMeleeAttack( attacker, damage ) 
   if( attacker:isBuilder() ) then damage:setAmount( 100 ) return end
   damage:setAmount( 1 )
   return
end

-- analyze the damage object sent to the defender
-- deal with any mitigating factors and adjust the damage amount as necessary
-- note that elemental damage and damage types will be handle by C
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
      room_msg = string.format( "%s attacks %s.", attackere:getShort(), defender:getShort() )
   elseif( status == HITDODGED ) then
   elseif( status == HITPARRIED ) then
   elseif( status = HITMISSED ) then
   end
   return atk_msg, def_msg, room_msg
end
