#############################################
# Common game params for TurboRaketti games #
#############################################

# game
resurrect_time               200      
end_time                     200      
immortal_time                200      
minplayers                   2
maxplayers                   2
include                      trship.ship
star_spread_speed            1
empty_gas_die_time           600

# environment damage
background_damage            100
particle_damage              7

# physics
particle_drag                1000
particle_mass                1
weapon_a_mass                1
weapon_b_mass                32
gas_mass                     0.1
gravity                      0.003

# platform
gas_refill_rate              4        
strength_refill_rate         1        
weapon_a_refill_rate         4
weapon_b_refill_rate         50
platform_friction            0.005

# generic weapon params
autofire_rate                4
weapon_b_max_repetition_rate 37
bullet_speed                 2.2      

# allowed weapons
streamgun
spraygun
doublegun
tripplegun
single
buck
seeker
mine

# specific weapon params
oneshot_damage               100      
buckshot_spread_speed        0.1
seek_radius                  130
heatseeker_accel             0.1      
asm_accel                    0.2         
heatseeker_armtime           50       
heatseeker_lifetime          200      
mine_lifetime                4800     
mine_drag                    50
mine_shot_speed              1
mine_bounce_speed            3
singleshot_armtime           100       
buckshot_armtime             22       
spraystream_spread           25      

gunstation_rate              200

# zone params
liquid_drag                  0.93

