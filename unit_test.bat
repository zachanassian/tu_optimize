@echo on
tu_optimize.exe "Barracus, Starformer(2)" "Constantine, Vigil(10)" -e "Poison 2" sim 10000
@echo off
echo === Expected ===
echo win%: 99.75 (9975 / 10000)
echo stall%: 0.07 (7 / 10000)
echo loss%: 0.18 (18 / 10000)
echo

@echo on
tu_optimize.exe "Nex, Apex(10)" "Obama, SF(5)" -e "Leech 3" sim 10000
@echo off
echo === Expected ===
echo win%: 22.30 (2230 / 10000)
echo stall%: 1.05 (105 / 10000)
echo loss%: 76.64 (7664 / 10000)
echo

@echo on
tu_optimize "Alaric, Vigil(5)" "Obama, Terraformer(5)" -e "Heal 1" sim 10000
@echo off
echo === Expected ===
echo win%: 98.55 (9855 / 10000)
echo stall%: 1.39 (139 / 10000)
echo loss%: 0.06 (6 / 10000)
echo

@echo on
tu_optimize "Alaric, Blitz(6)" "Barracus, Apex" -e "Evade 1" sim 10000
@echo off
echo === Expected ===
echo win%: 100 (10000 / 10000)
echo stall%: 0 (0 / 10000)
echo loss%: 0 (0 / 10000)
echo

@echo on
tu_optimize "Alaric, Vigil(4)" "Halcyon, Windreaver(5)" -e "Counter 1" sim 10000
@echo off
echo === Expected ===
echo win%: 25.08 (2508 / 10000)
echo stall%: 1.29 (129 / 10000)
echo loss%: 73.63 (7363 / 10000)
echo

@echo on
tu_optimize "Vex, HA(5)" "Halcyon, Galereaver(3)" -e "Berserk 3" sim 10000
@echo off
echo === Expected ===
echo win%: 11.92 (1192 / 10000)
echo stall%: 15.05 (1505 / 10000)
echo loss%: 73.03 (7303 / 10000)
echo

@echo on
tu_optimize "Alaric, Vigil(3)" "Obama, Apex(5)" -e "Armor 2" sim 10000
@echo off
echo === Expected ===
echo win%: 73.12 (7312 / 10000)
echo stall%: 25.31 (2531 / 10000)
echo loss%: 1.57 (157 / 10000)
echo

@echo on
tu_optimize "Barracus, Noble Defiance, Xeno Suzerain, Starformer" Heimdal -r sim 10000
@echo off
echo === Expected ===
echo win%: 59.73 (5973 / 10000)
echo stall%: 17.21 (1721 / 10000)
echo loss%: 23.06 (2306 / 10000)
echo

@echo on
tu_optimize.exe "Alaric, Revolver(3)" "Halcyon, Barrage Tank(3)" sim 10000
@echo off
echo === Expected ===
echo win%: 99.572 (9957 / 10000)
echo stall%: 0 (0 / 100000)
echo loss%: 0.428 (43 / 10000)
echo

@echo on
tu_optimize.exe "Nex, Tazerecca(4)" "Halcyon, Tikalan(3)" sim 10000
@echo off
echo === Expected ===
echo win%: 67.752 (67752 / 100000)
echo stall%: 0.213 (213 / 100000)
echo loss%: 32.035 (32035 / 100000)
echo

@echo on
tu_optimize.exe "Nex, Apex(10)" "Obama, SF(5)" sim 10000
@echo off
echo echo === Expected ===
echo win%: 24.19 (2419 / 10000)
echo stall%: 0.75 (75 / 10000)
echo loss%: 75.06 (7506 / 10000)
echo

@echo on
tu_optimize.exe "Obama, Rabid Corruptor(10)" "Yurich,Dread Panzer-1(3)" sim 10000
@echo off
echo === Expected ===
echo win%: 2.168 (2168 / 100000)
echo stall%: 97.832 (97832 / 100000)
echo loss%: 0 (0 / 100000)
echo

@echo on
tu_optimize.exe "Pet,HA(10)" "Obama, SF(5)" sim 10000
@echo off
echo === Expected ===
echo win%: 21.37 (2137 / 10000)
echo stall%: 21.35 (2135 / 10000)
echo loss%: 57.28 (5728 / 10000)
echo

@echo on
tu_optimize.exe "Barracus-6, Starformer-6" "Constantine-6, Pylon-3(6)" sim 10000
@echo off
echo === Expected ===
echo win%: 43.463 (43463 / 100000)
echo stall%: 43.914 (43914 / 100000)
echo loss%: 12.623 (12623 / 100000)
echo

@echo on
tu_optimize.exe "Cyrus-1, Starformer-6" "Constantine-6, Pylon-3(2)" sim 10000
@echo off
echo === Expected ===
echo win%: 68.75 (6875 / 10000)
echo stall%: 31.25 (3125 / 10000)
echo loss%: 0 (0 / 10000)
echo
