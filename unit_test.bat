@echo on
tu_optimize So-Iq-SI-MJ-Ss-LBO6-GV-LT example_group -r sim 10000
@echo off
echo === Expected ===
echo win%: 75.538 (561 9302 / 10000)
echo stall%: 0.188 (94 0 / 10000)
echo loss%: 24.274 (9345 698 / 10000)
echo.

@echo on
tu_optimize.exe "Nex, Apex(10)" "Obama, SF(5)" -e "Strike 2" sim 10000
@echo off
echo === Expected ===
echo Your Deck: [S0CC+q] Nexor, Apex, Apex, Apex, Apex, Apex, Apex, Apex, Apex, Apex, Apex
echo Enemy's Deck: [SoO6+l] Barracus, Starformer, Starformer, Starformer, Starformer, Starformer
echo Effect: Strike 2
echo win%: 20.07 (2007 / 10000)
echo stall%: 0.04 (4 / 10000)
echo loss%: 79.89 (7989 / 10000)
echo.

@echo on
tu_optimize So-Iq-SI-MJ-Ss-LBO6-GV-LT GT125k -r sim 1000
@echo off
echo === Expected ===
echo Filtering user defined decks with regular expression: ^GT12Sk\d\d$
echo Match: Custom Deck #61 "GT12Sk01": Barracus-6, Noxious Tank-6, Spiteful Raptor-6, Deranged Malgoth-6, Spiteful Raptor-6, Tazerecca-6, Entrails Bog-6, Omniphet-6, Xeno Suzerain-6, Spiteful Raptor-6, Sculpted Aegis-6
echo Match: Custom Deck #62 "GT12Sk02": Halcyon-6, Indestructible Troop-6, Indestructible Troop-6, Neocyte Resonator-6, Neocyte Resonator-6, Omniphet-6, Garrison Fortifier-6, Agrius Maximus-6, Galereaver-6, Flux Station-6, Arthurian Monarch-6
echo Match: Custom Deck #63 "GT12Sk03": Barracus-6, Narscious the Eerie-6, Warp Alchemist-6, Narscious the Eerie-6, Warp Alchemist-6, Bolt Crag-6, Sacred Sanctuary-6, Lord Hades-6, Mawnage-6, Noble Defiance-6
echo Match: Custom Deck #64 "GT12Sk04": Halcyon-6, Galereaver-6, Galereaver-6, Neocyte Resonator-6, Galereaver-6, Neocyte Resonator-6, Galereaver-6, Agrius Maximus-6, Galereaver-6, Agrius Maximus-6, Galereaver-6
echo Match: Custom Deck #65 "GT12Sk05": Barracus-6, Charincinerator-6, Heart Devourer-6, Heart Devourer-6, Heart Devourer-6, Remote Rig-6, Heart Devourer-6, Loaded Warehouse-6, Heart Devourer-6, Heart Devourer-6, Heart Devourer-6
echo Match: Custom Deck #66 "GT12Sk06": Barracus-6, Narscious the Eerie-6, Warp Alchemist-6, Narscious the Eerie-6, Warp Alchemist-6, Sacred Sanctuary-6, Heart Devourer-6, Loaded Warehouse-6, Charincinerator-6, Heart Devourer-6, Charincinerator-6
echo Match: Custom Deck #67 "GT12Sk07": Alaric-6, Flash Deity-6, Bolt Crag-6, Flash Deity-6, Heart Devourer-6, Noble Defiance-6, Sacred Sanctuary-6, Noble Defiance-6, Pure Benediction-6, Pure Benediction-6, Shining Sanctuary-6
echo Match: Custom Deck #68 "GT12Sk08": Barracus-6, Narscious the Eerie-6, Warp Alchemist-6, Narscious the Eerie-6, Warp Alchemist-6, Narscious the Eerie-6, Vicious Borg-6, Charincinerator-6, Vicious Borg-6, Charincinerator-6, Sacred Sanctuary-6
echo Match: Custom Deck #69 "GT12Sk09": Barracus-6, Narscious the Eerie-6, Charincinerator-6, Omega Nexus-6, Agrius Maximus-6, Agrius Maximus-6, Sacred Sanctuary-6, Valence-6, Sacred Sanctuary-6, Tazerecca-6, Ayrkrane Vik-6
echo Match: Custom Deck #70 "GT12Sk10": Alaric-6, Charincinerator-6, Charincinerator-6, Heart Devourer-6, Flash Deity-6, Noble Defiance-6, Noble Defiance-6, Grand Templar-6, Noble Defiance-6, Noble Defiance-6, Mawnage-6
echo Match: Custom Deck #71 "GT12Sk11": Nexor-6, Heart Devourer-6, Noxious Tank-6, Tartarus Obelisk-6, Rift Awakener-6, Xeno Suzerain-6, Tazerecca-6, Lord Hades-6, Noble Defiance-6, Sacred Sanctuary-6, Grand Templar-6
echo Match: Custom Deck #72 "GT12Sk12": Barracus-6, Narscious the Eerie-6, Warp Alchemist-6, Bolt Crag-6, Noxious Tank-6, Noble Defiance-6, Noble Defiance-6, Tazerecca-6, Xeno Suzerain-6, Sacred Sanctuary-6, Sculpted Aegis-6
echo Your Deck: [So-Iq-SI-MJ-Ss-LBO6-GV-LT] Barracus, Noble Defiance, Auger Bore, Xeno Suzerain, Grizrael the Reaper, Tazerecca, Starformer, Ayrkrane Vik, Mach Jet
echo Enemy's Deck: [SokU-DV-Dn-IA-Iw+j-LB-MJ-Mf] Barracus, Noxious Tank, Spiteful Raptor, Deranged Malgoth, Spiteful Raptor, Tazerecca, Entrails Bog, Omniphet, Xeno Suzerain, Spiteful Raptor, Sculpted Aegis
echo Enemy's Deck: [QEis+il2-CG-Cr-Ez+i-FT-Mf-QB] Halcyon, Indestructible Troop, Indestructible Troop, Neocyte Resonator, Neocyte Resonator, Omniphet, Garrison Fortifier, Agrius Maximus, Galereaver, Flux Station, Arthurian Monarch
echo Enemy's Deck: [SokO-B6-Ck-F9-Iq-KF+i-Of+i] Barracus, Narscious the Eerie, Warp Alchemist, Narscious the Eerie, Warp Alchemist, Bolt Crag, Sacred Sanctuary, Lord Hades, Mawnage, Noble Defiance
echo Enemy's Deck: [QEis+i-Cr+i-QB+m] Halcyon, Galereaver, Galereaver, Neocyte Resonator, Galereaver, Neocyte Resonator, Galereaver, Agrius Maximus, Galereaver, Agrius Maximus, Galereaver
echo Enemy's Deck: [Soksmt-Ce-FZ+n] Barracus, Charincinerator, Heart Devourer, Heart Devourer, Heart Devourer, Remote Rig, Heart Devourer, Loaded Warehouse, Heart Devourer, Heart Devourer, Heart Devourer
echo Enemy's Deck: [SokOmt-Ce+i-FZ+i-KF+i-Of+i] Barracus, Narscious the Eerie, Warp Alchemist, Narscious the Eerie, Warp Alchemist, Sacred Sanctuary, Heart Devourer, Loaded Warehouse, Charincinerator, Heart Devourer, Charincinerator
echo Enemy's Deck: [REiCkO-C3+i-Dh+i-FZ-F9-Iq+i] Alaric, Flash Deity, Bolt Crag, Flash Deity, Heart Devourer, Noble Defiance, Sacred Sanctuary, Noble Defiance, Pure Benediction, Pure Benediction, Shining Sanctuary
echo Enemy's Deck: [SokO-CA+i-Ce+i-KF+j-Of+i] Barracus, Narscious the Eerie, Warp Alchemist, Narscious the Eerie, Warp Alchemist, Narscious the Eerie, Vicious Borg, Charincinerator, Vicious Borg, Charincinerator, Sacred Sanctuary
echo Enemy's Deck: [SoPGkO+i-Ce-Cr+i-DP-GV-KF-LB] Barracus, Narscious the Eerie, Charincinerator, Omega Nexus, Agrius Maximus, Agrius Maximus, Sacred Sanctuary, Valence, Sacred Sanctuary, Tazerecca, Ayrkrane Vik
echo Enemy's Deck: [RE-B0-Ce+i-Ck-C3-FZ-Iq+k] Alaric, Charincinerator, Charincinerator, Heart Devourer, Flash Deity, Noble Defiance, Noble Defiance, Grand Templar, Noble Defiance, Noble Defiance, Mawnage
echo Enemy's Deck: [S0iakO-B0-B6-Cx-FZ-IA-Iq-LB-MJ] Nexor, Heart Devourer, Noxious Tank, Tartarus Obelisk, Rift Awakener, Xeno Suzerain, Tazerecca, Lord Hades, Noble Defiance, Sacred Sanctuary, Grand Templar
echo Enemy's Deck: [SokO-Dn-F9-IA-Iq+i-KF-LB-MJ-Of] Barracus, Narscious the Eerie, Warp Alchemist, Bolt Crag, Noxious Tank, Noble Defiance, Noble Defiance, Tazerecca, Xeno Suzerain, Sacred Sanctuary, Sculpted Aegis
echo win%: 72.9083 (992 911 549 852 725 439 793 202 753 843 935 755 / 1000)
echo stall%: 0.1 (1 0 2 2 0 1 0 3 1 1 0 1 / 1000)
echo loss%: 26.9917 (7 89 449 146 275 560 207 795 246 156 65 244 / 1000)
echo.

@echo on
tu_optimize.exe "Halcyon-1, Nimbus-4" "Halcyon, Nimbus-4" sim 10000
@echo off
echo === Expected ===
echo Your Deck: [P/Ad] Halcyon-1, Nimbus-4
echo Enemy's Deck: [QEAd] Halcyon, Nimbus-4
echo win%: 0 (0 / 10)
echo stall%: 0 (0 / 10)
echo loss%: 100 (10000 / 10000)
echo.

@echo on
tu_optimize "Constantine, Eviscerator Noz(4)" "Alaric, Noble Defiance(2)" sim 10000
@echo off
echo === Expected ===
echo win%: 9.13 (913 / 10000)
echo stall%: 2.29 (229 / 10000)
echo loss%: 88.58 (8858 / 10000)
echo.

@echo on
tu_optimize.exe "Barracus, Starformer(2)" "Constantine, Vigil(10)" -e "Poison 2" sim 10000
@echo off
echo === Expected ===
echo win%: 99.75 (9975 / 10000)
echo stall%: 0.07 (7 / 10000)
echo loss%: 0.18 (18 / 10000)
echo.

@echo on
tu_optimize.exe "Nex, Apex(10)" "Obama, SF(5)" -e "Leech 3" sim 10000
@echo off
echo === Expected ===
echo win%: 22.30 (2230 / 10000)
echo stall%: 1.05 (105 / 10000)
echo loss%: 76.64 (7664 / 10000)
echo.

@echo on
tu_optimize "Alaric, Vigil(5)" "Obama, Terraformer(5)" -e "Heal 1" sim 10000
@echo off
echo === Expected ===
echo win%: 98.55 (9855 / 10000)
echo stall%: 1.39 (139 / 10000)
echo loss%: 0.06 (6 / 10000)
echo.

@echo on
tu_optimize "Alaric, Blitz(6)" "Barracus, Apex" -e "Evade 1" sim 10000
@echo off
echo === Expected ===
echo win%: 100 (10000 / 10000)
echo stall%: 0 (0 / 10000)
echo loss%: 0 (0 / 10000)
echo.

@echo on
tu_optimize "Alaric, Vigil(4)" "Halcyon, Windreaver(5)" -e "Counter 1" sim 10000
@echo off
echo === Expected ===
echo win%: 25.08 (2508 / 10000)
echo stall%: 1.29 (129 / 10000)
echo loss%: 73.63 (7363 / 10000)
echo.

@echo on
tu_optimize "Vex, HA(5)" "Halcyon, Galereaver(3)" -e "Berserk 3" sim 10000
@echo off
echo === Expected ===
echo win%: 11.92 (1192 / 10000)
echo stall%: 15.05 (1505 / 10000)
echo loss%: 73.03 (7303 / 10000)
echo.

@echo on
tu_optimize "Alaric, Vigil(3)" "Obama, Apex(5)" -e "Armor 2" sim 10000
@echo off
echo === Expected ===
echo win%: 73.12 (7312 / 10000)
echo stall%: 25.31 (2531 / 10000)
echo loss%: 1.57 (157 / 10000)
echo.

@echo on
tu_optimize "Barracus, Noble Defiance, Xeno Suzerain, Starformer" Heimdal -r sim 10000
@echo off
echo === Expected ===
echo win%: 59.73 (5973 / 10000)
echo stall%: 17.21 (1721 / 10000)
echo loss%: 23.06 (2306 / 10000)
echo.

@echo on
tu_optimize.exe "Alaric, Revolver(3)" "Halcyon, Barrage Tank(3)" sim 10000
@echo off
echo === Expected ===
echo win%: 99.572 (9957 / 10000)
echo stall%: 0 (0 / 100000)
echo loss%: 0.428 (43 / 10000)
echo.

@echo on
tu_optimize.exe "Nex, Tazerecca(4)" "Halcyon, Tikalan(3)" sim 10000
@echo off
echo === Expected ===
echo win%: 67.752 (67752 / 100000)
echo stall%: 0.213 (213 / 100000)
echo loss%: 32.035 (32035 / 100000)
echo.

@echo on
tu_optimize.exe "Nex, Apex(10)" "Obama, SF(5)" sim 10000
@echo off
echo echo === Expected ===
echo win%: 24.19 (2419 / 10000)
echo stall%: 0.75 (75 / 10000)
echo loss%: 75.06 (7506 / 10000)
echo.

@echo on
tu_optimize.exe "Obama, Rabid Corruptor(10)" "Yurich,Dread Panzer-1(3)" sim 10000
@echo off
echo === Expected ===
echo win%: 2.168 (2168 / 100000)
echo stall%: 97.832 (97832 / 100000)
echo loss%: 0 (0 / 100000)
echo.

@echo on
tu_optimize.exe "Pet,HA(10)" "Obama, SF(5)" sim 10000
@echo off
echo === Expected ===
echo win%: 21.37 (2137 / 10000)
echo stall%: 21.35 (2135 / 10000)
echo loss%: 57.28 (5728 / 10000)
echo.

@echo on
tu_optimize.exe "Barracus-6, Starformer-6" "Constantine-6, Pylon-3(6)" sim 10000
@echo off
echo === Expected ===
echo win%: 43.463 (43463 / 100000)
echo stall%: 43.914 (43914 / 100000)
echo loss%: 12.623 (12623 / 100000)
echo.

@echo on
tu_optimize.exe "Cyrus-1, Starformer-6" "Constantine-6, Pylon-3(2)" sim 10000
@echo off
echo === Expected ===
echo win%: 68.75 (6875 / 10000)
echo stall%: 31.25 (3125 / 10000)
echo loss%: 0 (0 / 10000)
echo.
