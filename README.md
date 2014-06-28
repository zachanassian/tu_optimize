#Tyrant Unleashed Optimizer
Deck Simulator and Optimizer for Tyrant Unleashed!

##Usage
<pre>
usage: tu_optimize.exe Your_Deck Enemy_Deck [Flags] [Operations]

Your_Deck:
  the name/hash/cards of a custom deck.

Enemy_Deck:
  1) semicolon separated list of defense decks, syntax:
     deck1[:factor1];deck2[:factor2];...
     where deck is the name/hash/cards of a mission or custom deck, and factor is optional. The default factor is 1.
     example: "94. Heart of Tartarus" is the deck of mission 94. Heart of Tartarus at Level 10.
     example: "eerie-spam:0.2;nbd-spam:0.8" means eerie-spam is the defense deck 20% of the time, while nbd-spam is the defense deck 80% of
the time.
  2) a regular expression surrounded by /.
     regular expression will be used to search all custom decks for matching keys.
     example: "/^GT/" will select all custom decks starting with the letters GT.

Mode:
  pvp: attacker goes first. Simulate/optimize for win rate. Normally used for missions or pvp. [default]
  pvp-defense: attacker goes second. Simulate/optimize for win rate + stall rate. Normally used for pvp defense.
  gw: attacker goes second. Simulate/optimize for win rate. Normally used for guild wars.
  gw-defense: attacker goes first. Simulate/optimize for win rate + stall rate. Normally used for gw defense.
Order:
  random: the attack deck is played randomly. [default]
  ordered: the attack deck is played in order instead of randomly (respects the 3 cards drawn limit).
Flags:
  yfort &lt;your_fortress_cards&gt;: your fortress structures. your_fortress_cards: the name/hash/cards of one or two fortress structures.
  efort &lt;enemy_fortress_cards&gt;: enemy fortress structures. enemy_fortress_cards: the name/hash/cards of one or two fortress structures.
  -e &lt;effect&gt;: set the battleground effect.
               use "tu_optimize Po Po -e list" to get a list of all available effects.
  -t &lt;num&gt;: set the number of threads, default is 4.
  -turnlimit &lt;num&gt;: set the number of turns in a battle, default is 50.
  -v: less verbose output. Omits output about your and enemy's deck and fortress
Flags for climb:
  -c: don't try to optimize the commander.
  -L &lt;min&gt; &lt;max&gt;: restrict deck size between &lt;min&gt; and &lt;max&gt;.
  -o: restrict to the owned cards listed in "data/ownedcards.txt".
  -o=&lt;filename&gt;: restrict to the owned cards listed in &lt;filename&gt;.
                       example: -o=data/mycards.txt.
  -o=&lt;cards&gt;: restrict to the owned cards specified.
                 example: -o="Sacred Equalizer#2, Infantry".
  target &lt;num&gt;: stop as soon as the score reaches &lt;num&gt;.

Operations:
  sim &lt;num&gt;: simulate &lt;num&gt; battles to evaluate a deck.
  climb &lt;num&gt;: perform hill-climbing starting from the given attack deck, using up to &lt;num&gt; battles to evaluate a deck.
  reorder &lt;num&gt;: optimize the order for given attack deck, using up to &lt;num&gt; battles to evaluate an order.
</pre>

Remark: Due to html character escaping this might read awkward in readme.txt. 
Open https://github.com/zachanassian/tu_optimize/blob/master/README.md to get latest version in formatted view.

If you receive virus warnings for <code>SimpleTUOptimizeStarter.exe</code> please read [here](http://zachanassian.github.io/tu_optimize/faq.html#ahk).

##Changelog

##Version 3.3.0
* Added enhance rally battle ground effect: - [Issue#42](https://github.com/zachanassian/tu_optimize/issues/42) credits [grani](https://github.com/zachanassian/tu_optimize/pull/43)
<pre>
>tu_optimize "Constantine, Zodiac Harbinger" "Constantine, Bolt Crag" -e "Rally 2" sim 1000     
Your Deck: Deck: S6-MP
Enemy's Deck: Deck: S6-F9
Effect: Rally 2
win%: 0 (0 / 1000)
stall%: 100 (1000 / 1000)
loss%: 0 (0 / 1000)
</pre>
<pre>
>tu_optimize "Constantine, Zodiac Harbinger" "Constantine, Bolt Crag" -e "Rally 3" sim 1000
Your Deck: Deck: S6-MP
Enemy's Deck: Deck: S6-F9
Effect: Rally 3
win%: 100 (1000 / 1000)
stall%: 0 (0 / 1000)
loss%: 0 (0 / 1000)
</pre>

##Version 3.2.0
* <code>SimpleTUOptimizeStarter</code> allows to update <code>cards.xml</code> and <code>missions.xml</code>.
* <code>SimpleTUOptimizeStarter</code> allows to launch editor for <code>ownedcards.txt</code>, <code>customdeck.txt</code> and <code>cardabbrs.txt</code>.

##Version 3.1.0
* <code>SimpleTUOptimizeStarter</code> allows to configure number of threads used for simulations.
* Option -o= can now be placed behind operations
* [FIX] last version of <code>SimpleTUOptimizeStarter</code> required to enter a SPACE after specifying <code>Flags:</code>. No longer needed.
* Added link to virus warning explanation for <code>SimpleTUOptimizeStarter</code> to readme.txt
* Added current <code>cards.xml</code> - 5 new basic epics eg. Sinew Feeder.

##Version 3.0.0
* Got a mail from Alex Reeve CEO of synapse-games that tu_optimize does not violate any IP from synapse-games. Thank you! Big credits to synapse-games for creating this game. [Details](http://www.kongregate.com/forums/338-tyrant-unleashed/topics/407952)
* Changed the command line syntax to allow easier commands for new users. New modes are pvp, pvp-defense, gw and gw-defense. Please check the usage info from above. The old commands will mostly (see below) still work so you have time to adapt to the new command. However old commands can be removed in next version without further notice.
* Incompatible Change: The option -o is now used by default if operation climb is used.
* If attack deck contains more the commander plus 10 cards a new warning message is issued.
* <code>SimpleTUOptimizeStarter</code> now allows to configure most use cases without setting additional flags. Try it.
* Added current <code>cards.xml</code> and <code>missions.xml</code> - mission Albatross.
* [FIX] remove crash caused by new skill flurry.

##Version 2.5.0
* Added missions Razogoth Mutant-1 to Razogoth Mutant-5 to <code>data/customdecks_template.txt</code>. Eg. Razogoth Mutant-5 contains all Razogoth Mutant cards at level 5
* Added option to specify owned cards at command line using -o=<cards>  - credits [draquila](https://github.com/zachanassian/tu_optimize/pull/31)
<pre>
>tu_optimize "Constantine, Bolt Crag#5, Tempest Citadel#2" "Razogoth Mutant" -o="Barracus, Insanitius, Arch Nova, Omega Nexus, Sacred Sanctuary, Tazerious, Ayrkrane Vik, Necropocalypse, Mawcor" -r climb 10000
Your Deck: [S6-F9+loS+i] Constantine, Bolt Crag, Bolt Crag, Bolt Crag, Bolt Crag, Bolt Crag, Tempest Citadel, Tempest Citadel
Enemy's Deck: [U+-lE+i-lK+i-lQ+i-lW+i-lc+i] Razogoth Mutant, Shadow Mutant, Shadow Mutant, Razoling Mutant, Razoling Mutant, Gloom Mutant, Gloom Mutant, Typhon's Mutant, Typhon's Mutant, Dream Mutator, Dream Mutator
2.01 (201 / 10000)
2.01: Constantine, Bolt Crag #5, Tempest Citadel #2
Deck improved: S6-F9-cD-F9+joS+i 0 [4381] Bolt Crag -> 1 [5795] Tazerious: 2.05 (205 / 10000)
2.05: Constantine, Bolt Crag, Tazerious, Bolt Crag #3, Tempest Citadel #2
Deck improved: S6-F9+koS-F9oS 1 [5795] Tazerious -> 5 [4381] Bolt Crag: 2.12 (212 / 10000)
2.12: Constantine, Bolt Crag #4, Tempest Citadel, Bolt Crag, Tempest Citadel
Deck improved: S6-F9+koS+i-F9 5 [4381] Bolt Crag -> 6 [4381] Bolt Crag: 2.82 (282 / 10000)
2.82: Constantine, Bolt Crag #4, Tempest Citadel #2, Bolt Crag
Deck improved: S6-F9+i-c9-F9+ioS+i-F9 7 -void- -> 2 [5853] Arch Nova: 2.88 (288 / 10000)
2.88: Constantine, Bolt Crag #2, Arch Nova, Bolt Crag #2, Tempest Citadel #2, Bolt Crag
Deck improved: S6-F9-cD-F9-c9-F9+ioS+i-F9 8 -void- -> 1 [5795] Tazerious: 3.21 (321 / 10000)
3.21: Constantine, Bolt Crag, Tazerious, Bolt Crag, Arch Nova, Bolt Crag #2, Tempest Citadel #2, Bolt Crag
Deck improved: S6-F9-cD-F9-c9-F9+ioS+i-fN-F9 9 -void- -> 8 [5997] Insanitius: 3.47 (347 / 10000)
3.47: Constantine, Bolt Crag, Tazerious, Bolt Crag, Arch Nova, Bolt Crag #2, Tempest Citadel #2, Insanitius, Bolt Crag
Deck improved: S6-F9-cD-F9-c9-F9+ioS+i-F9-fN 9 [4381] Bolt Crag -> 8 [4381] Bolt Crag: 3.84 (384 / 10000)
3.84: Constantine, Bolt Crag, Tazerious, Bolt Crag, Arch Nova, Bolt Crag #2, Tempest Citadel #2, Bolt Crag, Insanitius
Deck improved: S6-F9+i-c9-F9+ikOoS+i-F9-fN 1 [5795] Tazerious -> 5 [2318] Sacred Sanctuary: 4.21 (421 / 10000)
4.21: Constantine, Bolt Crag #2, Arch Nova, Bolt Crag #2, Sacred Sanctuary, Tempest Citadel #2, Bolt Crag, Insanitius
Deck improved: S6-F9+kkOoS+i-GV-F9-fN 2 [5853] Arch Nova -> 7 [4405] Ayrkrane Vik: 5.97 (597 / 10000)
5.97: Constantine, Bolt Crag #4, Sacred Sanctuary, Tempest Citadel #2, Ayrkrane Vik, Bolt Crag, Insanitius
Deck improved: S6-F9+jkOoS-F9oS-GV-F9-fN 2 [4381] Bolt Crag -> 5 [4381] Bolt Crag: 5.98 (598 / 10000)
5.98: Constantine, Bolt Crag #3, Sacred Sanctuary, Tempest Citadel, Bolt Crag, Tempest Citadel, Ayrkrane Vik, Bolt Crag, Insanitius
Deck improved: S6-F9+joS-F9oSkO-GV-F9-fN 3 [2318] Sacred Sanctuary -> 6 [2318] Sacred Sanctuary: 6.1 (610 / 10000)
6.1: Constantine, Bolt Crag #3, Tempest Citadel, Bolt Crag, Tempest Citadel, Sacred Sanctuary, Ayrkrane Vik, Bolt Crag, Insanitius
Deck improved: S6-F9+joS+ikO-F9-GV-F9-fN 4 [4381] Bolt Crag -> 6 [4381] Bolt Crag: 6.31 (631 / 10000)
6.31: Constantine, Bolt Crag #3, Tempest Citadel #2, Sacred Sanctuary, Bolt Crag, Ayrkrane Vik, Bolt Crag, Insanitius
Deck improved: S6-F9+joS+ikO-F9-c9-F9-fN 7 [4405] Ayrkrane Vik -> 7 [5853] Arch Nova: 6.5 (650 / 10000)
6.5: Constantine, Bolt Crag #3, Tempest Citadel #2, Sacred Sanctuary, Bolt Crag, Arch Nova, Bolt Crag, Insanitius
Deck improved: S6-F9+joS+ikO-F9+i-c9-fN 7 [5853] Arch Nova -> 8 [5853] Arch Nova: 6.77 (677 / 10000)
6.77: Constantine, Bolt Crag #3, Tempest Citadel #2, Sacred Sanctuary, Bolt Crag #2, Arch Nova, Insanitius
Deck improved: S6-F9+koS+ikO-F9-c9-fN 7 [4381] Bolt Crag -> 0 [4381] Bolt Crag: 6.92 (692 / 10000)
6.92: Constantine, Bolt Crag #4, Tempest Citadel #2, Sacred Sanctuary, Bolt Crag, Arch Nova, Insanitius
Deck improved: S6-F9+koS+ikO-F9-cD-fN 8 [5853] Arch Nova -> 8 [5795] Tazerious: 7.38 (738 / 10000)
7.38: Constantine, Bolt Crag #4, Tempest Citadel #2, Sacred Sanctuary, Bolt Crag, Tazerious, Insanitius
Deck improved: S6-F9+koS+ikO-F9-c9-cD 9 [5997] Insanitius -> 8 [5853] Arch Nova: 7.54 (754 / 10000)
7.54: Constantine, Bolt Crag #4, Tempest Citadel #2, Sacred Sanctuary, Bolt Crag, Arch Nova, Tazerious
Evaluated 1250 decks (1681634 + 1362939 simulations).
Optimized Deck: 7.54: Constantine, Bolt Crag #4, Tempest Citadel #2, Sacred Sanctuary, Bolt Crag, Arch Nova, Tazerious
</pre>

##Version 2.4.0
* [FIX] jam all - [Issue#30](https://github.com/zachanassian/tu_optimize/issues/30)
* Added current <code>cards.xml</code> and <code>missions.xml</code> - mission Infested Rastax

##Version 2.3.0
* Added support for cards with id greater then 8046. New maximum card_id is 24000.

##Version 2.2.0
* Added skill Enhance Corrosive
* Added battleground effect Corrosive 1-3 - [Issue#27](https://github.com/zachanassian/tu_optimize/issues/27)
* Added example for -o= - [Issue#28](https://github.com/zachanassian/tu_optimize/issues/28)
* Added switch -v to removed output of enemy's (and your) deck - [Issue#29](https://github.com/zachanassian/tu_optimize/issues/29)
* Added current <code>cards.xml</code> and <code>missions.xml</code> - mission Blightbloom
<pre>
tu_optimize "Cyrus, Bolt Crag(5)" "Barracus, Mephalus Gorge(2), Noble Defiance" -e "Corrosive 3" sim 10000
Your Deck: [RW-F9+l] Cyrus, Bolt Crag, Bolt Crag, Bolt Crag, Bolt Crag, Bolt Crag
Enemy's Deck: [So-Iq-hY+i] Barracus, Mephalus Gorge, Mephalus Gorge, Noble Defiance
Effect: Corrosive 3
win%: 33.08 (3308 / 10000)
stall%: 0 (0 / 10000)
loss%: 66.92 (6692 / 10000)
</pre>

##Version 2.1.0
* Added GT126k - credits [YWNM](http://www.kongregate.com//forums/338/topics/400905?page=1#posts-8034438)
* Added skill corrosive
<pre>
tu_optimize "Cyrus, Vigil" "Test Oracle, Medic" +v debug
...
TURN 21 begins for Commander [Cyrus hp:14]
Evaluating Commander [Cyrus hp:14] skill Heal imperial 2
Evaluating Commander [Cyrus hp:14] skill Siege 1
Evaluating Assault 0 [Vigil att:5 hp:12, corroded 5 [speed:+1]] skill Heal all 1
Evaluating Assault 0 [Vigil att:5 hp:12, corroded 5 [speed:+1]] skill Rally all righteous 1
Possible targets of Rally:
\+ Assault 0 [Vigil att:5 hp:12, corroded 5 [speed:+1]]
Assault 0 [Vigil att:5 hp:12, corroded 5 [speed:+1]] Rally (1) on Assault 0 [Vigil att:5 hp:12, corroded 5 [speed:+1]]
Assault 0 [Vigil att:5+1(rallied)=1 hp:12, corroded 5 [speed:+1]] attacks Commander [Test Oracle hp:24] for 1 damage
Commander [Test Oracle hp:24] takes 1 damage
TURN 21 ends for Commander [Cyrus hp:14]
...
Stall after 50 turns.
win%: 0 (0 / 1)
stall%: 100 (1 / 1)
loss%: 0 (0 / 1)
...
tu_optimize "Cyrus, Dread Panzer" "Test Oracle, Medic" +v debug
...
TURN 19 begins for Commander [Cyrus hp:14]
Evaluating Commander [Cyrus hp:14] skill Heal imperial 2
Evaluating Commander [Cyrus hp:14] skill Siege 1
Assault 0 [Dread Panzer att:5 hp:6, corroded 4 [speed:+1]] attacks Commander [Test Oracle hp:30] for 1 damage
Commander [Test Oracle hp:30] takes 1 damage
TURN 19 ends for Commander [Cyrus hp:14]
------------------------------------------------------------------------
TURN 20 begins for Commander [Test Oracle hp:29]
TURN 20 ends for Commander [Test Oracle hp:29]
------------------------------------------------------------------------
TURN 21 begins for Commander [Cyrus hp:14]
Evaluating Commander [Cyrus hp:14] skill Heal imperial 2
Evaluating Commander [Cyrus hp:14] skill Siege 1
Assault 0 [Dread Panzer att:5 hp:6, corroded 5 [speed:+1]] does not attack and looses corrosion
TURN 21 ends for Commander [Cyrus hp:14]
...
You win.
win%: 100 (1 / 1)
stall%: 0 (0 / 1)
loss%: 0 (0 / 1)
</pre>

##Version 2.0.1
* [FIX] if "your deck" is also part of "enemy deck" batch the efort structures were also used as yfort structures - https://github.com/zachanassian/tu_optimize/issues/24
* Added current <code>cards.xml</code> and <code>missions.xml</code> - New fusions (e.g. Honorable Samurai)

##Version 2.0.0
* [FIX] Battleground effects are now applied before commander skills
* [FIX] performance housekeeping: if jam is not charged no targets will be selected
* Removed debug option for an overall performance gain of 15-20%

##Version 1.2.1
* [FIX] Commanders jam was not recharged at start of turn

##Version 1.2.0
* Added support for Fortress Cards: - credits alessard(github)/AndyL161(kong)
They will be played out along with a players commander and reduce their timer already on first turn
Climb not supported if they are contained within a decks card list or <code>data/ownedcards.txt</code>
New commandline arguments (yfort <cardlist>, efort <cardlist>) added to set global fortress cards which are used for all simulations including climb
* Added current <code>cards.xml</code> and <code>missions.xml</code> - Rastax

##Version 1.1.3
* [FIX] Missing cards in <code>missions.xml</code> will no longer stop the program. Instead an error message like this will be displayed:
<code>Exception [While trying to find the card with id 5603: no such key in the cards_by_id map.] while loading deck [Interruption] from file missions.xml. Skip loading this mission.</code>
* Added last "good" version of <code>cards.xml</code> and <code>missions.xml</code> including 106. World's End
* [FIX] Corrected "Enemy Deck(s)" in <code>SimpleTUOptimizeStarter</code>

##Version 1.1.2
* [FIX] Corrected <code>GT04</code> to use <code>Bombardment Tank-3</code>

##Version 1.1.1
* [FIX] <code>example_group</code> syntax corrected in <code>customdecks_template.txt</code>
  
##Version 1.1
* deck groups can be defined in <code>customdecks.txt</code>. Example: <code>example_group: eerie-spam:0.2;nbd-spam:0.8</code>
* regular expresions can be used as Enemy_Deck to find matching custom decks from <code>customdecks.txt</code>
* regular expressions can be stored in <code>customdecks.txt</code> for easy reuse
* <code>customdecks_template.txt</code> contains data for famous gauntlets (GT125k, GT100k, Gauntlet) from [Excel Sim] Version 6.2 - credits rbwabd. If copied 
to your <code>customdecks.txt</code> you can run <code>tu_optimize mydeck GT125k sim 10000</code>
* Added battleground effect Strike 1-3
* Added new cards.xml and missions.xml - Shiva
   
##Version 1.0.2
* [FIX] Halycon-1 was treated as non player card Halcyon[1997] - https://github.com/zachanassian/tu_optimize/issues/10

##Version 1.0.1
* Added patch to avoid compiler errors on Mac OSX - credits ksvintsov

##Version 1.0
* Added new cards.xml and missions.xml - Evrane

##Version 1.0[RC]
* SimpleTUOptimizeStarter no longer needs quotes
* SimpleTUOptimizeStarter will close all his opened output windows on exit or window close
* SimpleTUOptimizeStarter Help to show build in help
* SimpleTUOptimizeStarter Web to open http://zachanassian.github.io/tu_optimize/
* [FIX] SimpleTUOptimizeStarter output window should no longer close immediatly
* ownedcards.txt single copy of a card no longer requires count (1)
* Added battleground effect Poison 1-3
<pre>
tu_optimize.exe "Barracus, Starformer(2)" "Constantine, Vigil(10)" -e "Poison 2" sim 10000
Your Deck: [SoO6+i] Barracus, Starformer, Starformer
Enemy's Deck: [S6Cd+q] Constantine, Vigil, Vigil, Vigil, Vigil, Vigil, Vigil, Vigil, Vigil, Vigil, Vigil
Effect: Poison 2
win%: 99.75 (9975 / 10000)
stall%: 0.07 (7 / 10000)
loss%: 0.18 (18 / 10000)
</pre>
* Added battleground effect Leech 1-3
<pre>
tu_optimize.exe "Nex, Apex(10)" "Obama, SF(5)" -e "Leech 3" sim 1000000
Your Deck: [S0CC+q] Nexor, Apex, Apex, Apex, Apex, Apex, Apex, Apex, Apex, Apex, Apex
Enemy's Deck: [SoO6+l] Barracus, Starformer, Starformer, Starformer, Starformer, Starformer
Effect: Leech 3
win%: 22.3022 (223022 / 1000000)
stall%: 1.0532 (10532 / 1000000)
loss%: 76.6446 (766446 / 1000000)
</pre>
* Added battleground effect Heal 1-3
<pre>
tu_optimize "Alaric, Vigil(5)" "Obama, Terraformer(5)" -e "Heal 1" sim 100000
Your Deck: [RECd+l] Alaric, Vigil, Vigil, Vigil, Vigil, Vigil
Enemy's Deck: [SoOe+l] Barracus, Terraformer, Terraformer, Terraformer, Terraformer, Terraformer
Effect: Heal 1
win%: 98.553 (98553 / 100000)
stall%: 1.39 (1390 / 100000)
loss%: 0.057 (57 / 100000)
</pre>
* Added battleground effect Evade 1-3
<pre>
tu_optimize "Alaric, Blitz(6)" "Barracus, Apex" -e "Evade 1" sim 10000
Your Deck: [REIJ+m] Alaric, Blitz, Blitz, Blitz, Blitz, Blitz, Blitz
Enemy's Deck: [SoCC] Barracus, Apex
Effect: Evade 1
win%: 100 (10000 / 10000)
stall%: 0 (0 / 10000)
loss%: 0 (0 / 10000)
</pre>
* Added battleground effect Counter 1-3
<pre>
tu_optimize "Alaric, Vigil(4)" "Halcyon, Windreaver(5)" -e "Counter 1" sim 10000
Your Deck: [RECd+k] Alaric, Vigil, Vigil, Vigil, Vigil
Enemy's Deck: [QEIP+l] Halcyon, Windreaver, Windreaver, Windreaver, Windreaver, Windreaver
Effect: Counter 1
win%: 25.08 (2508 / 10000)
stall%: 1.29 (129 / 10000)
loss%: 73.63 (7363 / 10000)
</pre>
* Added battleground effect Berserk 1-3
<pre>
tu_optimize "Vex, HA(5)" "Halcyon, Galereaver(3)" -e "Berserk 3" sim 10000
Your Deck: [QKLQ+l] Typhon Vex, Havoc Alpha, Havoc Alpha, Havoc Alpha, Havoc Alpha, Havoc Alpha
Enemy's Deck: [QE-QB+j] Halcyon, Galereaver, Galereaver, Galereaver
Effect: Berserk 3
win%: 11.92 (1192 / 10000)
stall%: 15.05 (1505 / 10000)
loss%: 73.03 (7303 / 10000)
</pre>
* Added battleground effect Armor 1-3
<pre>
tu_optimize "Alaric, Vigil(3)" "Obama, Apex(5)" -e "Armor 2" sim 10000
Your Deck: [RECd+j] Alaric, Vigil, Vigil, Vigil
Enemy's Deck: [SoCC+l] Barracus, Apex, Apex, Apex, Apex, Apex
Effect: Armored 2
win%: 73.12 (7312 / 10000)
stall%: 25.31 (2531 / 10000)
loss%: 1.57 (157 / 10000)
</pre>

##Version 0.10.1
* [FIX] Played cards started with evade_left 0
<pre>
tu_optimize "Barracus, Noble Defiance, Xeno Suzerain, Starformer" Heimdal -r sim 10000
win%: 59.73 (5973 / 10000)
stall%: 17.21 (1721 / 10000)
loss%: 23.06 (2306 / 10000)
</pre>

##Version 0.10
* Added Inhibit
<pre>
tu_optimize "Cyrus, Test Inhibit" "Cyrus, Rally Infantry" +v debug
...
TURN 6 begins for Commander [Cyrus hp:13]
Assault 0 [Rally Infantry att:2 hp:6 cd:1, inhibited 3] reduces its timer
Evaluating Commander [Cyrus hp:13] skill Heal imperial 2
Possible targets of Heal:
\+ Assault 0 [Rally Infantry att:2 hp:6, inhibited 3]
Commander [Cyrus hp:13] Heal (2) on Assault 0 [Rally Infantry att:2 hp:6, inhibited 3] but it is inhibited
Evaluating Commander [Cyrus hp:13] skill Siege 1
Evaluating Assault 0 [Rally Infantry att:2 hp:6, inhibited 2] skill Rally 1
Possible targets of Rally:
\+ Assault 0 [Rally Infantry att:2 hp:6, inhibited 2]
Assault 0 [Rally Infantry att:2 hp:6, inhibited 2] Rally (1) on Assault 0 [Rally Infantry att:2 hp:6, inhibited 2] but it is inhibited
Assault 0 [Rally Infantry att:2 hp:6, inhibited 1] attacks Assault 0 [Test Inhibit att:1 hp:10] for 2 damage
Assault 0 [Test Inhibit att:1 hp:10] takes 2 damage
TURN 6 ends for Commander [Cyrus hp:13]
...
</pre> 
* updated cards.xml and missons.xml

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
