#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
#Warn  ; Recommended for catching common errors.
#SingleInstance off
SendMode Input  ; Recommended for new scripts due to its superior speed and reliability.
SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.

Gui, Add, Text, r5, My Deck:
Gui, Add, Text, r5, Enemie's Deck:
Gui, Add, Text, r1, Options:
Gui, Add, Edit, vMyDeck ym w600 r5, Cyrus, Medic, Revolver, Imperial APC, Medic, Imperial APC
Gui, Add, Edit, vEnemiesDeck w600 r5, 94. Heart of Tartarus
Gui, Add, Edit, vSimOptions w600, -r -o climb 10000
Gui, Add, Button, default r2 w100 x100 y+15 section, Simulate
Gui, Add, Button, r2 w100 ys xs+200, Exit
Gui, Show,, Simple Tyrant Unleashed Optimize Starter - Copyright (C) 2014 zachanassian
return  

ButtonSimulate:
Gui, Submit
Run, cmd.exe /c title TUOptimizeOutput && echo tu_optimize "%MyDeck%" "%EnemiesDeck%" %SimOptions% && tu_optimize "%MyDeck%" "%EnemiesDeck%" %SimOptions% && pause
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
