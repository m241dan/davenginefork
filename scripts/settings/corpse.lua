-- returns the amount of time it takes for this corpse to decay based on the instance it was passed --
function corpseDecay( instance )
   return standard_corpse_decay
end

-- move the items from the instance to the corpse
-- the reason this is scripts is to handle any exceptions
-- for example, maybe unique items don't go to your corpse
function inventoryToCorpse( instance, corpse )
   local item
   for item in instance:eachInventory() do
      item:to( corpse )
   end
end


