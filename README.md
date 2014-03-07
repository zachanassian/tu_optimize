#tyrant_optimize
This fork tries to adapt the code to be used with Tyrant Unleashed!
This is **not working** at the moment!

##Version 0.5
* Changed name from tyrant_optimize to tu_optimize
* [FIX] cardabbrs.txt can be used again

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
