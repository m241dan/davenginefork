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

-- design the duration/frequency parts of the damage object
function onMeleeAttack( attacker, damage )
end

-- analyze the damage object sent to the defender
-- calculator any the actual damage that the object does
-- note that any elemental damage will be handled by C
function onReceiveDamage( defender, damage )
end

-- return order attacker -> defender -> room
function combatMessage( attacker, defender, damage, status )
end

