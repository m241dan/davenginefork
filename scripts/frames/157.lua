-- NOT NOISE ---
-- ONLY HARD SCRIPTS IN THIS SPACE --
-- END NOT NOISE --
function onInstanceInit( frame, instance )
   frame:inherits( "onInstanceInit", "fi", frame:getInheritance(), instance )
   --- autowrite init ---
   instance:setStatPerm( "Strength", 20 )
   instance:setStatPerm( "Dexterity", 20 )
   instance:setStatPerm( "Intelligence", 20 )
   instance:setStatPerm( "Constitution", 20 )
   instance:setStatPerm( "Wisdom", 20 )
   instance:setStatPerm( "Charisma", 20 )
   instance:setStatPerm( "Health", 500 )
   instance:setStatMod( "Health", 500 )
   instance:setStatPerm( "Luck", 20 )
   instance:setStatPerm( "Evasiveness", 5 )
   instance:setStatPerm( "Accuracy", 0 )
   instance:setStatPerm( "Energy", 10000 )
   instance:setStatMod( "Energy", 10000 )
   instance:setStatPerm( "Spirit", 20 )
   instance:setStatMod( "Spirit", 20 )
   instance:setStatPerm( "Stamina", 400 )
   instance:setStatMod( "Stamina", 400 )
   instance:setStatPerm( "PowerLevel", 500 )
   instance:setStatPerm( "Damroll", 4 )
   instance:setStatPerm( "Hitroll", 4 )
   instance:setStatPerm( "Experience", 1 )
   instance:setStatPerm( "HP_Regen", 100 )
   instance:setStatPerm( "Mana_Regen", 180 )
   instance:setStatPerm( "Stamina_Regen", 140 )
   instance:setStatPerm( "Tier", 0 )
   --- end autowrite init ---
end

function onDeath( frame, dead_person )
   frame:inherits( "onDeath", "fi", frame:getInheritance(), dead_person )
   -- write specific for this frame under here --
end

function onSpawn( frame, spawning_person )
   frame:inherits( "onSpawn", "fi", frame:getInheritance(), spawning_person )
   -- write specific for this frame under here --
end

-- NOT NOISE --
-- ONLY SPEC SCRIPTS AFTER THIS POINT --
-- END NOT NOISE --
function onEntityEnter( room, enterer )
end

function onEntityLeave( room, leaver )
end

function onEntering( room, enterer )
end

function onLeaving( room, leaver )
end

function onGreet( greeter, enterer )
end

function onFarewell( waver, leaver )
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

-- calculate any factors effecting melee cooldowns
function meleeCooldown( attacker )
   return automelee_delay
end

-- check to see if the attacker can atk the victim, ie same room? can see? etc etc --
-- return anything other than a string if you can attack, otherwise return the reason --
-- for example "%s is not out of range" or "%s is not here" whatever you want chummy --
function meleeCheck( attacker, victim )
end

-- design the duration/frequency parts of the damage object
function prepMeleeTimer( attacker, timer )
end

-- figure out the damage amount portion of the damage object
function prepMeleeDamage( attacker, damage ) 
end

-- analyze the damage object sent to the defender
-- calculator any the actual damage that the object does
-- note that any elemental damage will be handled by C
function onReceiveDamage( defender, damage )
end

-- return order attacker -> defender -> room
function combatMessage( attacker, defender, damage, status )
end

-- returns the amount of time it takes for this corpse to decay based on the instance it was passed
function corpseDecay( instance )
   return standard_corpse_decay
end

-- move the items from the instance to the corpse
-- the reason this is scripts is to handle any exceptions
-- for example, maybe unique items don't go to your corpse
function inventoryToCorpse( instance, corpse )
end

-- UI Stuff --
function uiPrompt( entity )
end

function uiLook( looker, looking_at )
end

function uiInventory( entity )
end

function uiScore( entity )
end

