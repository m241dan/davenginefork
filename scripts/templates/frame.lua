function onInstanceInit( frame, instance )
   frame:inherits( "onInstanceInit", "fi", frame:getInheritance(), instance )
   --- autowrite init ---
   --- end autowrite init ---
end

function onEntityEnter( room, entity )
end

function onEntityLeave( room, entity )
end

function onEntityEntering( room, entity )
end

function onEntityLeaving( room, entity )
end

function onGreetEntity( greeter, entity )
end

function onFarewellEntity( farewell, entity )
end

function onSay( entity, sayer )
end

function onGive( entity, giver )
end

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
end

