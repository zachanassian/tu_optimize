#Tyrant Unleashed Optimizer
Deck Simulator and Optimizer for Tyrant Unleashed!

##Version 0.9
* Beta Release
* ownedcards.txt allows cards without id
* added simple UI: SimpleTUOptimizeStarter.exe
* added tu_optimize icon credit to http://openiconlibrary.sourceforge.net/gallery2/?./Icons/apps/preferences-web-browser-cache-2.png
* the zip file now contains cards.xml and missions.xml

##Version 0.8
* Prepare Alpha Release
* cardabbrs.txt, cards.xml, customdecks.txt, missions.xml, ownedcards.txt are now read from folder data/
* [FIX] no poison damage on dead card
* Added GNU GENERAL PUBLIC LICENSE Version 3
* [FIX] mission commander gets max level

##Version 0.7
* Evade logic adjusted to TU rules
* [Fix] basic TU Factions are now recognized correctly
* Added Enhance Evade
<pre>
tu_optimize.exe "Alaric, Revolver(3)" "Halcyon, Barrage Tank(3)" sim 100000
Your Deck: [REH3+j] Alaric, Revolver, Revolver, Revolver
Enemy's Deck: [QEFj+j] Halcyon, Barrage Tank, Barrage Tank, Barrage Tank
win%: 99.572 (99572 / 100000)
stall%: 0 (0 / 100000)
loss%: 0.428 (428 / 100000)
</pre>
* Allow more then one legendary in deck
<pre>
tu_optimize.exe "Alaric, Omega(2)" "Halcyon, Barrage Tank(3)" -o=exclude/ownedcards_omega.txt climb 1000
Your Deck: [REBA+i] Alaric, Omega, Omega
Enemy's Deck: [QEFj+j] Halcyon, Barrage Tank, Barrage Tank, Barrage Tank
0 (0 / 1000)
0: Alaric, Omega #2
Deck improved: REBA+j -void- -> [64] Omega: 99.6 (996 / 1000)
99.6: Alaric, Omega #3
Deck improved: REBA+k -void- -> [64] Omega: 100 (1000 / 1000)
100: Alaric, Omega #4
Evaluated 4 decks (4000 + 2000 simulations).
Optimized Deck: 100: Alaric, Omega #4
</pre>
* Progenitor are now targeted by faction specific skills
<pre>
tu_optimize.exe "Nex, Tikal(3)" "Halcyon, Barrage Tank(3)" +v sim 1
Your Deck: Deck: S0PS+j
Nexor hp:52 legendary xeno, Enhance Leech 2, Protect xeno 2, Enfeeble all 1
  Tikal 2/15/2 legendary progenitor, armored 3, leech 4, Rally 3
  Tikal 2/15/2 legendary progenitor, armored 3, leech 4, Rally 3
  Tikal 2/15/2 legendary progenitor, armored 3, leech 4, Rally 3
Enemy's Deck: Deck: QEFj+j
Halcyon hp:47 legendary imperial, Enhance Armored all 1, Heal all imperial 2, Strike 2
  Barrage Tank 3/7/3 imperial, armored 1, Strike 2
  Barrage Tank 3/7/3 imperial, armored 1, Strike 2
  Barrage Tank 3/7/3 imperial, armored 1, Strike 2
------------------------------------------------------------------------
TURN 1 begins for Commander [Nexor hp:52]
Commander [Nexor hp:52] plays Assault 0 [Tikal 2/15/2 legendary progenitor, armored 3, leech 4, Rally 3]
Commander [Nexor hp:52] Protect (2) on Assault 0 [Tikal att:2 hp:15 cd:2]
TURN 1 ends for Commander [Nexor hp:52]
...
</pre>
* Jam logic adjusted to TU rules
<pre>
tu_optimize.exe "Nex, Tazerecca(4)" "Halcyon, Tikalan(3)" sim 100000
Your Deck: [S0-LB+k] Nexor, Tazerecca, Tazerecca, Tazerecca, Tazerecca
Enemy's Deck: [QEPY+j] Halcyon, Tikalan, Tikalan, Tikalan
win%: 67.752 (67752 / 100000)
stall%: 0.213 (213 / 100000)
loss%: 32.035 (32035 / 100000)
</pre>

##Version 0.6
* Added Enhance Leech
<pre>
tu_optimize.exe "Nex, Apex(10)" "Obama, SF(5)" sim 100000
Recognize abbreviation Nex: Nexor-6
Recognize abbreviation Apex: Apex-6
Recognize abbreviation Obama: Barracus-6
Recognize abbreviation SF: Starformer-6
Your Deck: Deck: S0CC+q
Enemy's Deck: Deck: SoO6+l
win%: 24.193 (24193 / 100000)
stall%: 0.745 (745 / 100000)
loss%: 75.062 (75062 / 100000)
</pre>
* Maxed Version of a Card is now recognized without -level => Barracus-6 => Barracus. However Barracus-6 will still be recognized as a build in abbreviation
* Replaced "Recognize abbreviation" as to verbose with Your Deck: [S0CB+q] Nexor, Apex-5, Apex-5, Apex-5, Apex-5, Apex-5, Apex-5, Apex-5, Apex-5, Apex-5, Apex-5
* Added Enhance Counter
<pre>
tu_optimize.exe "Obama, Rabid Corruptor(10)" "Yurich,Dread Panzer-1(3)" sim 100000
Your Deck: [SoFt+q] Barracus, Rabid Corruptor, Rabid Corruptor, Rabid Corruptor, Rabid Corruptor, ...
Enemy's Deck: [QiAm+j] Yurich, Dread Panzer-1, Dread Panzer-1, Dread Panzer-1
win%: 2.168 (2168 / 100000)
stall%: 97.832 (97832 / 100000)
loss%: 0 (0 / 100000)
</pre>

##Version 0.5
* Changed name from tyrant_optimize to tu_optimize
* [FIX] cardabbrs.txt can be used again
* Added Enhance Berserk
<pre>
tu_optimize.exe "Pet,HA(10)" "Obama, SF(5)" sim 100000
Recognize abbreviation Pet: Petrisis-6
Recognize abbreviation HA: Havoc Alpha-6
Recognize abbreviation Obama: Barracus-6
Recognize abbreviation SF: Starformer-6
Your Deck: Deck: SuLQ+q
Enemy's Deck: Deck: SoO6+l
win%: 21.368 (21368 / 100000)
stall%: 21.348 (21348 / 100000)
loss%: 57.284 (57284 / 100000)
</pre>

##Version 0.4
* Added new skill Enhance Poison
<pre>
tyrant_optimize.exe "Barracus-6, Starformer-6" "Constantine-6, Pylon-3(6)" sim 100000
Your Deck: Deck: SoO6
Enemy's Deck: Deck: S6CI+m
win%: 43.463 (43463 / 100000)
stall%: 43.914 (43914 / 100000)
loss%: 12.623 (12623 / 100000)
&nbsp;
tyrant_optimize.exe "Barracus-6,Rabid Corruptor-3(3)" "Cyrus-1,Havoc-5(3)" sim 1000000
Your Deck: Deck: SoFt+j
Enemy's Deck: Deck: PoA5+j
win%: 72.9008 (729008 / 1000000)
stall%: 27.0992 (270992 / 1000000)
loss%: 0 (0 / 1000000)
</pre>

##Version 0.3
* Added new skill Enhance Armored
<pre>
tyrant_optimize.exe "Cyrus-1, Starformer-6" "Constantine-6, Pylon-3(2)" sim 100000
Your Deck: Deck: PoO6
Enemy's Deck: Deck: S6CI+i
win%: 68.75 (68750 / 100000)
stall%: 31.25 (31250 / 100000)
loss%: 0 (0 / 100000)
</pre>

##Version 0.2
* Poison is now triggered at end of turn and protect reduces poison damage.
<pre>
tyrant_optimize.exe "Cyrus-1, Starformer-6" "Constantine-6, Pylon-3(2)" debug
</pre>

##Version 0.1
* TU: cards.xml and mission.xml are now parsed
* Mission decks will always refer to mission level 10
* Card level has to be indicated -level eg. Barracus-6 => Barracus Level 6
* This is working at the moment
<pre>
tyrant_optimize.exe "Barracus-6, Starformer-6, Tartarus Brood-6" "94. Heart of Tartarus" debug
</pre>
* Compared this to simXX 1.000.000 sims. This: 14,43 sec simXX: 102,78 sec.  
* What is still missing:
..* Poison is triggered at start instead of end of turn 
..* Enhance Skill not working
..* Jam Skill not working as in TU
..* Evade Skill works differently
..* Unit Type 6 Progenitor can be targeted by any faction skill
..* only one legendary in a deck
