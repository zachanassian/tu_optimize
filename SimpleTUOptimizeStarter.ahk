#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
;#Warn  ; Recommended for catching common errors.
#SingleInstance off
SendMode Input  ; Recommended for new scripts due to its superior speed and reliability.
SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.

Menu, MyMenu, Add, ownedcards.txt, MenuOwnedcards
Menu, MyMenu, Add, customdecks.txt, MenuCustomdecks
Menu, MyMenu, Add, cardabbrs.txt, MenuCardabbrs
Menu, MyMenu, Add, Update XMLs, MenuUpdate
Menu, MyMenu, Add, Help, MenuHelp
Menu, MyMenu, Add, Web, MenuWeb
Gui, Menu, MyMenu
Gui, Add, Text, r5, My Deck:
Gui, Add, Text, r1, My Fortress:
Gui, Add, Text, r5, Enemy Deck(s):
Gui, Add, Text, r1, Enemy Fortress:
Gui, Add, Text, r1, Effect:
Gui, Add, Text, r1, Mode:
Gui, Add, Text, r1, Order:
Gui, Add, Text, r1, Operation:
Gui, Add, Text, r1, Flags:
Gui, Add, Edit, vMyDeck ym w600 r5, Cyrus, Medic, Revolver, Imperial APC, Medic, Imperial APC
Gui, Add, Edit, vMySiege w600 r1,
Gui, Add, Edit, vEnemiesDeck w600 r5, 94. Heart of Tartarus
Gui, Add, Edit, vEnemySiege w600 r1,
Gui, Add, DropDownList, vEffect Choose1, none|Armor 1|Armor 2|Armor 3|Berserk 1|Berserk 2|Berserk 3|Corrosive 1|Corrosive 2|Corrosive 3|Counter 1|Counter 2|Counter 3|Enfeeble 1|Enfeeble 2|Enfeeble 3|Evade 1|Evade 2|Evade 3|Heal 1|Heal 2|Heal 3|Leech 1|Leech 2|Leech 3|Poison 1|Poison 2|Poison 3|Rally 1|Rally 2|Rally 3|Strike 1|Strike 2|Strike 3
Gui, Add, Radio, vMode r1 Checked section, PVP
Gui, Add, Radio, ys, PVP (defense)
Gui, Add, Radio, ys, Guildwar
Gui, Add, Radio, ys, Guildwar (defense)
Gui, Add, Radio, vOrder r2 xs Group Checked section, Random
Gui, Add, Radio, r2 ys, Ordered
Gui, Add, Radio, vOperation r1 xs Group Checked section, Climb
Gui, Add, Radio, r1 ys, Sim
Gui, Add, Radio, r1 ys, Reorder
Gui, Add, Text, r1 ys, Iterations:
Gui, Add, Edit, vIterations w100 r1 ys-3, 10000
Gui, Add, Text, r1 ys, Threads:
Gui, Add, DropDownList, vThreads ys-3 w50 Choose4, 1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16
Gui, Add, Edit, vSimOptions r1 xs w600,
Gui, Add, Button, default r2 w100 x100 y+15 section, Simulate
Gui, Add, Button, r2 w100 ys xs+200, Exit
Gui, Show,, Simple Tyrant Unleashed Optimize Starter - Copyright (C) 2014 zachanassian
return  

ButtonSimulate:
Gui, Submit
selMode :=  ( Mode == 1 ? "pvp" : Mode == 2 ? "pvp-defense" : Mode == 3 ? "gw" : "gw-defense" )
selOrder :=  ( Order == 1 ? "random" : "ordered" )
selOperation :=  ( Operation == 1 ? "climb" : Operation == 2 ? "sim" : "reorder" )
selMySiege := ( MySiege == "" ? "" : "yfort """ MySiege """ ")
selEnemySiege := ( EnemySiege == "" ? "" : "efort """ EnemySiege """ ")
selEffect := ( Effect == "none" ? "" : "-e """ Effect """ ")
selThreads := ( Threads == "4" ? "" : "-t " Threads " ")
selSimOptions := ( SimOptions == "" ? "" : SimOptions " ")
execString = tu_optimize "%MyDeck%" "%EnemiesDeck%" %selMode% %selOrder% %selMySiege%%selEnemySiege%%selEffect%%selThreads%%selSimOptions%%selOperation% %Iterations% 
Run, cmd.exe /c title TUOptimizeOutput && echo %execString% && %execString% && pause
Gui, Show
return

MenuHelp:
Gui, Submit
Run, cmd.exe /c title TUOptimizeOutput && echo tu_optimize && tu_optimize && pause
Gui, Show
return

MenuWeb:
Gui, Submit
Run http://zachanassian.github.io/tu_optimize/
Gui, Show
return

MenuUpdate:
MsgBox, 0, Update started, Updating cards.xml and missions.xml.`nPlease wait at least 10 seconds. A new window should open soon.`nThis Window will auto close in 2 seconds. , 2
UrlDownloadToFile, http://mobile.tyrantonline.com/assets/cards.xml, data\cards.xml
had_error := false
if ErrorLevel
{
    MsgBox, Error downloading cards.xml.
    had_error := true
}
UrlDownloadToFile, http://mobile.tyrantonline.com/assets/missions.xml, data\missions.xml
if ErrorLevel
{
    MsgBox, Error downloading missions.xml.
    had_error := true
}
if !had_error
    MsgBox, 0, Update finished, cards.xml and missions.xml successfully updated.`nThis Window will auto close in 2 seconds., 2
Gui, Show
return

MenuOwnedcards:
Gui, Submit
Run, Notepad.exe data\ownedcards.txt
Gui, Show
return

MenuCustomdecks:
Gui, Submit
Run, Notepad.exe data\customdecks.txt
Gui, Show
return

MenuCardabbrs:
Gui, Submit
Run, Notepad.exe data\cardabbrs.txt
Gui, Show
return

GuiClose:
ButtonExit:
while true
{
  IfWinExist, TUOptimizeOutput
      WinClose ; use the window found above
  else
      break
}  
ExitApp
