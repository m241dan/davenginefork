-- master table --
elements_table = { flesh_element, fire_element, water_element, earth_element, lightning_element, wind_element, light_element, dark_element }

-- element frameworks --
flesh_element = {}
flesh_element[name] = "Flesh"
flesh_element[strong_against] = {}
flesh_element[weak_against] = {}
flesh_element[composition] = { { flesh_element, 100 } } 

fire_element = {}
fire_element[name] = "Fire"
fire_element[strong_againt] = { wind_element }
fire_element[weak_against] = { water_element }
fire_element[composition] = { { fire_element, 100 } }

water_element = {}
water_element[name] = "Water"
water_element[strong_against] = { fire_element }
water_element[weak_against] = { earth_element }
water_element[composition] = { { water_element, 100 } }

earth_element = {}
earth_element[name] = "Earth"
earth_element[strong_against] = { water_element }
earth_element[weak_against] = { lightning_element }
earth_element[composition] = { { earth_element, 100 } }

lightning_element = {}
lightning_element[name] = "Lightning"
lightning_element[strong_against] = { earth_element }
lightning_element[weak_against] = { wind_element }
lightning_element[composition] = { { lightning_element, 100 } }

wind_element = {}
wind_element[name] = "Wind"
wind_element[strong_against] = { lightning_element }
wind_element[weak_against] = { fire_element }
wind_element[composition] = { { wind_element, 100 } }

light_element = {}
light_element[name] = "Light"
light_element[strong_against] = {}
light_element[weak_against] = { dark_element }
light_element[composition] = { { light_element, 100 } }

dark_element = {}
dark_element[name] = "Dark"
dark_element[strong_against] = {}
dark_element[weak_against] = { light_element }
dark_element[composition] = { { dark_element, 100 } }

function getElementalIndex( element_name )
   for index, name in pairs( elements_table ) do
      if( name[name] == element_name ) then return index end
   end
   return -1
end

function getElementalName( index )
   return elements[index][name]
end
