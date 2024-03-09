# MoonLander
Moon lander SDL2 game made in C

## Goal
Your goal is to safely land the probe on the moon's surface at one of the possible landing sites marked by red lines.  
To ensure that the lander remains intact upon touchdown, you need to align its rotation to the ground and limit both the vertical and horizontal speed of the module.  
If all landing criteria are met, 3 green dots will appear in the top right corner, indicating that the landing will be successful.  
Fuel is limited, so use it wisely.

## Controls
- Use the right and left arrows to rotate the probe.
- Hold the up arrow to turn on the thruster.
- Pause the game by pressing ESC.

## Settings
In the settings menu, you can change the map size and the difficulty.  
On harder difficulties there are fewer landing sites available and there is less fuel provided.

## Score and replays
Score is calculated from several factors such as time, remaining fuel, difficulty and map size.  
Successful and unsuccessful landings can be watched again in the replays section. By default, the file names are the Unix time of the landing, but you can rename any replay file in the "replays" folder created in the directory of the executable after the first finished landing.