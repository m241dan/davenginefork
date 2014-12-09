-- NOT NOISE ---
-- ONLY HARD SCRIPTS IN THIS SPACE --
-- END NOT NOISE --
function onInstanceInit( frame, instance )
   frame:inherits( "onInstanceInit", "fi", frame:getInheritance(), instance )
   --- autowrite init ---
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
   enterer:echoAt( "%s says, 'I am %s!'", enterer:getShort(), enterer:getShort() )
end

function onEntityLeave( room, leaver )
end

function onEntering( room, enterer )
   enterer:interp( "say I am the first Icer!" )
   room:echo( "The First Icer flexes his muscles" )
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
end

function parryChance( attacker, parryr )
-- need to ifcheck for a shield for 15% chance to parry blunt plus str/10
-- otherwise 5% chance + str/10
   return 0
end

function missChance( attacker, victim )
--- duplicate dodgechance, but with dexterity vs dexterity
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
   entity:echoAt( string.format( "<HP: %d/%d KI: %d/%d MV: %d/%d\nPL: %d/%d>\n", entity:getStatMod( "Health" ), entity:getStatPerm( "Health" ), entity:getStatMod( "Energy" ), entity:getStatPerm( "Energy" ), entity:getStatMod( "Stamina" ), entity:getStatPerm( "Stamina" ), entity:getStatPerm ( "PowerLevel" ), entity:getStatValue ( "PowerLevel" ) ) )
   local target = entity:getTarget()

   if( target ~= nil ) then
      entity:echoAt( string.format( "Target: %s\n", target:getName() ) )
   end
end



function uiLook( looker, looking_at )
   local account = Account.getAccount( looker ) -- get the account, so we can get the pagewidth
   local room = looker:getContainer() -- get the container of the looker
   if account == nil then -- if the account is nil (mob) default to 80 pagewidth
      pagewidth = 80 
   else
      pagewidth = account:getPagewidth() -- otherwise get the pagewidth from account
   end

   local bar = string.rep( "-=", ( pagewidth / 2 ) ) -- so, "-" x 80 equals 80 spaces, however, "-=" x 80 = 160 spaces. so pagewidth / number_of_character, -= is two characters... NAILED IT

   looker:echoAt( string.format( "%s\n", room:getShort() ) ) 
   looker:echoAt( string.format( "%s\n", bar ) )
   looker:echoAt( string.format( "%s\n", room:getDesc() ) )
   looker:echoAt( string.format( "%s\n", bar )  )
   looker:echoAt( " \tExits: \n#n" )
   for exit in room:eachInventory() do 
      if( exit:isExit() == true) then 
         looker:echoAt( string.format( "%s :", exit:getShort() ) )
         local exit_to = exit:getExitTo()
         if( exit_to ~= nil ) then
            looker:echoAt( string.format( " %s\n", exit_to:getShort() ) )
         else
         looker:echoAt( " nowhere... \n" )
         end
      end
   end

   for stuff in room:eachInventory() do
      if( stuff:isExit() == false ) then
        looker:echoAt( string.format( "%s\n", stuff:getLong() ) )
      end
   end
   looker:echoAt( "\n" )
end

function uiInventory( entity )
   local exits = "" 
   local mobs = ""
   local items = ""

   entity:echoAt( " - Your Inventory -\n" )
   for item in entity:eachInventory() do
      if( item:isExit() == true ) then
         exits = exits .. item:getShort() .. "\n"
      elseif( item:isObj() == true ) then
         items = items .. item:getShort() .. "\n"
      elseif( item:isMob() == true ) then
         mobs = mobs .. item:getShort() .. "\n"
      end
   end
   entity:echoAt( string.format( "Exits:\n %s", exits ) )
   entity:echoAt( string.format( "Mobs:\n %s", mobs ) )
   entity:echoAt( string.format( "Items:\n %s", items ) )
end

function uiScore( entity )
   local firstspacing = entity:getStatMod( "Spirit" )
   local secondspacing = entity:getStatPerm( "Luck" )
   local thirdspacing = entity:getStatPerm( "Damroll" )
   local fourthspacing = entity:getStatPerm( "Hitroll" )
   local fifthspacing = entity:getStatPerm( "Evasiveness" )
   local sixthspacing = entity:getStatPerm( "Accuracy" )

   entity:echoAt( string.format( "You currently have %d/%d health, %d/%d energy, and %d/%d stamina.\n", entity:getStatMod( "Health" ), entity:getStatPerm( "Health" ), entity:getStatMod( "Energy" ), entity:getStatPerm( "Energy" ), entity:getStatMod( "Stamina" ), entity:getStatPerm( "Stamina" ) ) )
   entity:echoAt( string.format( "Str: %d#y (#w%d) \t Dex: %d#y (#w%d) \t Con: %d#y (#w%d) \n", entity:getStatPerm( "Strength" ), entity:getStatValue( "Strength" ), entity:getStatPerm( "Dexterity" ), entity:getStatValue( "Dexterity" ), entity:getStatPerm( "Constitution" ), entity:getStatValue( "Constitution") ) )
   entity:echoAt( string.format( "Int: %d#y (#w%d) \t Wis: %d#y (#w%d) \t Chr: %d#y (#w%d) \n", entity:getStatPerm( "Intelligence" ), entity:getStatValue( "Intelligence" ), entity:getStatPerm( "Wisdom" ), entity:getStatValue( "Wisdom" ), entity:getStatPerm( "Charisma" ), entity:getStatValue( "Charisma" ) ) )

   if firstspacing <= 9 and secondspacing <= 9 then
      entity:echoAt( string.format( "Spi: %d#y  (#w%d) \t Lck: %d#y  (#w%d) \t\n", entity:getStatMod( "Spirit" ), entity:getStatPerm( "Spirit" ), entity:getStatPerm( "Luck" ), entity:getStatValue( "Luck" ) ) )  
   elseif secondspacing <= 9 and firstspacing >= 10 then
      entity:echoAt( string.format( "Spi: %d#y (#w%d) \t Lck: %d#y  (#w%d) \t\n", entity:getStatPerm( "Spirit" ), entity:getStatValue( "Spirit" ), entity:getStatPerm( "Luck" ), entity:getStatValue( "Luck" ) ) )  
   elseif firstspacing <= 9 and secondspacing >= 10 then
      entity:echoAt( string.format( "Spi: %d#y  (#w%d) \t Lck: %d#y (#w%d) \t\n", entity:getStatPerm( "Spirit" ), entity:getStatValue( "Spirit" ), entity:getStatPerm( "Luck" ), entity:getStatValue( "Luck" ) ) )  
   else
      entity:echoAt( string.format( "Spi: %d#y (#w%d) \t Lck: %d#y (#w%d) \t\n", entity:getStatPerm( "Spirit" ), entity:getStatValue( "Spirit" ), entity:getStatPerm( "Luck" ), entity:getStatValue( "Luck" ) ) )  
   end

   if thirdspacing <= 9 and fourthspacing <= 9 then
      entity:echoAt( string.format( "Dam: %d#y  (#w%d) \t Hit: %d#y  (#w%d) \t\n", entity:getStatPerm( "Damroll" ), entity:getStatValue( "Damroll" ), entity:getStatPerm( "Hitroll" ), entity:getStatValue( "Hitroll" ) ) )
   elseif fourthspacing <= 9 and thirdspacing >= 10 then
      entity:echoAt( string.format( "Dam: %d#y (#w%d) \t Hit: %d#y  (#w%d) \t\n", entity:getStatPerm( "Damroll" ), entity:getStatValue( "Damroll" ), entity:getStatPerm( "Hitroll" ), entity:getStatValue( "Hitroll" ) ) )
   elseif thirdspacing <= 9 and fourthspacing >= 10 then
      entity:echoAt( string.format( "Dam: %d#y  (#w%d) \t Hit: %d#y (#w%d) \t\n", entity:getStatPerm( "Damroll" ), entity:getStatValue( "Damroll" ), entity:getStatPerm( "Hitroll" ), entity:getStatValue( "Hitroll" ) ) )
   else
      entity:echoAt( string.format( "Dam: %d#y (#w%d) \t Hit: %d#y (#w%d) \t\n", entity:getStatPerm( "Damroll" ), entity:getStatValue( "Damroll" ), entity:getStatPerm( "Hitroll" ), entity:getStatValue( "Hitroll" ) ) )
   end

   if fifthspacing <= 9 and sixthspacing <= 9 then
      entity:echoAt( string.format( "Eva: %d#y  (#w%d) \t Acc: %d#y  (#w%d) \t\n\n", entity:getStatPerm( "Evasiveness" ), entity:getStatValue( "Evasiveness" ), entity:getStatPerm( "Accuracy" ), entity:getStatValue( "Accuracy" ) ) )
   elseif sixthspacing <= 9 and fifthspacing >= 10 then
      entity:echoAt( string.format( "Eva: %d#y (#w%d) \t Acc: %d#y  #w%d) \t\n\n", entity:getStatPerm( "Evasiveness" ), entity:getStatValue( "Evasiveness" ), entity:getStatPerm( "Accuracy" ), entity:getStatValue( "Accuracy" ) ) )
   elseif fifthspacing <= 9 and sixthspacing >= 10 then
      entity:echoAt( string.format( "Eva: %d#y  (#w%d) \t Acc: %d#y (#w%d) \t\n\n", entity:getStatPerm( "Evasiveness" ), entity:getStatValue( "Evasiveness" ), entity:getStatPerm( "Accuracy" ), entity:getStatValue( "Accuracy" ) ) )
   else
      entity:echoAt( string.format( "Eva: %d#y (#w%d) \t Acc: %d#y (#w%d) \t\n\n", entity:getStatPerm( "Evasiveness" ), entity:getStatValue( "Evasiveness" ), entity:getStatPerm( "Accuracy" ), entity:getStatValue( "Accuracy" ) ) )
   end
end

