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

-- design the damage object to send to the defender
-- deal with any attacker specific factors and set the amount
-- note that elemental damage and damage types will be handled by C
function onMeleeAttack( attacker, damage )
end

-- analyze the damage object sent to the defender
-- deal with any mitigating factors and adjust the damage amount as necessary
-- note that elemental damage and damage types will be handle by C
function onReceiveDamage( defender, damage )
end

-- return order attacker -> defender -> room
function combatMessage( attacker, defender, damage, status )
end

