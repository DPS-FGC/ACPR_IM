# ACPR Improvement Mod
This is a port of the [BBCF-Improvement-Mod AI edition](https://github.com/KDing0/BBCF-Improvement-Mod) by [KDing0](https://github.com/KDing0/BBCF-Improvement-Mod/commits?author=KDing0).
The purpose of this mod is to add features to the game both for fun and for practice.

Features of this mod:

1. A trainer.  
2. Custom palettes.  
3. Game overlay (hitbox viewer + frame meter) - based on [GGXXACPROverlay](https://github.com/YouKnow232/ggxxacpr_overlay/tree/main) by [YouKnow232](https://github.com/YouKnow232/ggxxacpr_overlay/commits?author=YouKnow232).  
4. Custom AI based on case-based-reasoning - based on implementation by [KDing0](https://github.com/KDing0/BBCF-Improvement-Mod/commits?author=KDing0) from the original BBCF mod.  

## Installation
Unpack the .7z file (ACPR_IM-V1.0.0-Online.7z) from the [releases](https://github.com/DPS-FGC/ACPR_IM/releases/latest) section and extract the contents.
dinput8.dll and all accompanying files should be placed in the folder where GGXXACPR_Win.exe is located.

The mod was tested under Windows 11, Linux (wine and proton) and Android (winlator) using both original steam_api.dll and
[Goldberg's Steam emulator](https://mr_goldberg.gitlab.io/goldberg_emulator/).

**IMPORTANT**: If using Proton or Goldberg's Steam emulator, set SteamDLLType = 1 in settings.ini, otherwise the game will crash
when entering an online lobby.

To run under wine / winlator:  
    1. Open winecfg either from the terminal (linux) of from the start menu (winlator).  
	2. Navigate to the Libraries tab.  
	3. Search for dinput8.dll under Existing overrides. If it's not there, add it manually.  
	4. Select dinput8 and click Edit.  
	5. Select "Native the Builtin".  

To run under proton:  
	1. Open steam.  
	2. Right click on the game and choose properties.  
	3. Under launch options, set WINEDLLOVERRIDES="dinput8.dll=n,b" %command%.  
	4. Run the game.  
	5. If the game crashes with the mod, setting PROTON_USE_WINED3D=1 and/or changing proton versions might help.  

## Usage
The usage of the mod is identical to the [original BBCF mod](https://github.com/libreofficecalc/BBCF-Improvement-Mod).  
Press F1 in game to open the mod menu (the toggle button can be changed in settings.ini file).  
Click on the different headers to expand/collapse them. Hovering the mouse over most user interface (UI) elements will show help text for that UI element.  

Some basic settings can be changed using settings.ini file. Default palette settings can be changed using palettes.ini file.  

Further details on each mod feature are listed below:  

### Trainer
The trainer is used mainly for fun and to enhance the single player experience.  
***It can not be used for cheating online.***  

The trainer features are separated into multiple categories:  
- General: Contains features such as infinite health, tension, burst, etc...  
- Defence: Allows adding automatic defence, IB, slashback, auto throw techs, etc... Custom probabilities can be set for each type of defence (see more in the AI section).
- Character specific: Shows character specific features only relevant to the currently selected characters.
- Extra menu: Allows changing values of extra menu variables outside of versus/training modes.
- Game modes: Features related to specific game modes. Currently allows selecting custom arcade mode level and custom survival boss. These features will be visible only after selecting the relevant game mode.

Custom settings can be saved to and loaded from trainer configuration files.  
The trainer is disabled in online/replay modes as it causes desyncs.

### Custom palettes
Allows changing color palettes without external file modifications (e.g. palmod).
Click on the `Player1/2` buttons to choose from available custom palettes.

To create a custom palette, click on the `Palette editor` button.  
In the palette editor, use `Highlight mode` to see which pixels are affected by each color.

Custom palette files are saved under ACPR_IM/Palettes/_character name_

**NOTE**: Currently only basic character palettes are supported. Special effects / UI edits / unlinked sprites still require palmod.

### Game overlay
Adds an overlay to the game which displays hitboxes and a frame meter.  
The entire logic for the overlay is taken directly from [GGXXACPROverlay by YouKnow232](https://github.com/YouKnow232/ggxxacpr_overlay/tree/main) ver 1.3.1-beta.

The frame meter works in replay mode in two ways:  
By default, it is reset every time the replay is rewinded.  
Checking the `Allow rewinding` checkbox will enable rewinding the frame meter when the replay is rewinded. It requires slightly more memory and is therefore disabled by default.

### CBR AI
Allows recording and training custom AI based on matches with real players.  
The logic for the AI is mostly retained from the original [BBCF mod](https://github.com/KDing0/BBCF-Improvement-Mod) with small changes made for compatibility with +R.

A quick video tutorial on the mod is available by Kding0 on [youtube](https://youtu.be/74DYdpBzIOs) and a more detailed guide on his [github repository](https://github.com/KDing0/BBCF-Improvement-Mod).

To train the AI from matches against other players, select the `Auto Record Myself`, `Auto Record Opponents` and `Auto Saving in Lobby` checkboxes **before** entering a match and enter your player name in the textbox.

When automatic recording is on, the AI will automatically learn from players in Online, VS 2P and Replay modes.  

It is possible to perform automatic training from a folder containing replays by pressing on the `Auto Scan Replay Folder` button.  

The scan range can be limited by entering a start index and end index for the replays.  
Setting the end index to zero will scan up to the end of the folder.  
**NOTE**: The first replay starts at index = 2 since the first item in the folder is the \<back> folder.  
To stop a replay scan after it was started, click on `Stop replay auto scan` in the CBR AI menu during the replay. The replay can then be exited or played until it ends.

When recording in VS 2P mode or from replays recorded for local matches, the players' names can be set manually in the relevant textboxes.

To play against the AI:
- Enter training mode.
- Set the second player to controller.
- It is recommended to set Health, Tension and Burst settings to normal (not infinite).
- Click on `Load` and select from the available list in the opened window. Alternatively, enter the player's name in `Player Name` and click `Load By Name`.
- Then click `Replaying`.

**NOTE**: If the AI gets hit a lot in neutral / wakeup, using the automatic defence options with custom probabilities from the trainer can be used to make it more/less challenging.

### Online features (EXPERIMENTAL!)
Clicking on the `Online` button (can be toggled with F2) while in an online game room (private room or quick match), will show avilable improvement
mod users.
Custom palettes will be shared online between improvement mod users (whether players or spectators).
In the `Custom palettes` section or the `Online` window, palette selection buttons will appear next to the players.
You can change your palette mid match, but not the opponents. The opponents palette can be disabled by clicking the X
button or checking the `Disable` checkbox. While the `Disable` checkbox is ticked, custom palettes will not be loaded for
that player.

Sharing your palettes can be disabled from the main window under `Custom Palettes` by removing `Enable palette sharing` checkmark.

Custom palettes from other players are automatically saved under ACPR_IM/Download/Character Name.
If they do not appear in the `Custom palettes` window, click on `Reload custom palettes`.

Palette information for both players is also stored under ACPR_IM/Replays/_replay name_.prep. These files are used
by the mod to enable custom palettes in replay mode. They should have the same name as the corresponding replay (.ggr) file to be
loaded correctly.
The .prep files can be organized into subfolders inside ACPR_IM/Replays.

**IMPORTANT**: This mode is still experimental and might occasionally cause desyncs and, in rare cases, wrong palette colors.
An older offline-only build is available in the releases section. Use it if the online version has noticable effects on the gameplay.

**Note**: Custom palette sharing is not supported in Team vs modes (offline and online). Automatic spectating is also not
currently supported.

**Note**: If an online palette shares the same filename as a local palette it will not be shown. Rename one of the files for
it to appear in the custom palette menu.

**Note**: Palettes modified with palmod will be shared online as well under _OpponentName_-_PaletteType_, where _PaletteType_
refers to the button used to get that palette (e.g. K = Kick, ExS = Ex palette + slash, etc...). The mod determines whether the
opponents palette was modified by comparing it with the users default palette. This means that if the opponent uses an unmodified
palette, while the user uses a modified palette, the unmodified palette will be saved as well.

## Removing the mod
To disable the mod simply rename or delete dinput8.dll.

## Troubleshooting
If the game crashes, or any other weird behavior occurs in game, try removing the mod. If the problem persists then it is
unrelated to the mod.
If the game crashes when entering an online lobby, make sure the correct SteamDLLType is chosen in `settings.ini` (see Installation section).

## Compiling
This mod was compiled using Microsoft Visual Studio 2022 (toolset v143).  
Additional libraries used were [boost (x86)](https://sourceforge.net/projects/boost/files/boost-binaries/1.87.0/) and [openssl (32-bit)](https://slproweb.com/products/Win32OpenSSL.html).

## Reporting bugs
Please use the reporting issues section on github to report bugs.  
I do not have a lot of time to work on this project, so it might take time until issues are resolved.

## Acknowledgements
This mod would not be possible without the original creators of BBCF-Improvement-Mod, KDing0 for his implementation of custom AI and YouKnow232 for the GGXXACPROverlay mod.  
The user interface of the mod is based on [Dear ImGui](https://github.com/ocornut/imgui).

Special thanks to Lightfat for help with development and testing.

## Legal
ACPR Improvement Mod is NOT associated with Arc System Works or any of its partners / affiliates.  
ACPR Improvement Mod is NOT intended for malicious use.  
ACPR Improvement Mod is NOT intended to give players unfair advantages in online matches.  
ACPR Improvement Mod should only be used on the official version that you legally purchased and own.  
ACPR Improvement Mod is intended for entertainment and educational purposes and not for commercial use.

Use ACPR Improvement Mod at your own risk.  
I am not responsible for what happens while using ACPR Improvement Mod. You take full reponsibility for any outcome that happens to you while using this application.
