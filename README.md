# Shadow Strike Story — 0.1.0 beta

Equips the Legends Assassin's Shadow Strike on Jin in the main campaign, replacing Dance of Wrath in the Ultimate slot. Use your normal Dance of Wrath input to activate it. The ability's own targeting and effects are handled by the game.

Default inputs: keyboard/mouse V + right mouse button; PlayStation controller L1 + R1; Xbox controller LB + RB. Custom bindings may differ. The skill requires sufficient resolve.

## Compatibility

Windows x64, Steam Ghost of Tsushima Director's Cut executable 1053.9.0623.1807, Steam build 23879181. The plugin checks the executable timestamp, image size and relevant code signatures before hooking. Other builds remain inactive and produce a log message. Epic and Proton are not validated.

Campaign only. Does not modify Legends gameplay or anti-cheat. Do not enable the Cheat Engine prototype or Master Thyself alongside this plugin: they can target the same hook or slot. Other mods which replace Ultimate or hook the same update function may conflict.

## Installation

Close the game. Copy `plugins/ShadowStrikeStory.asi` and `plugins/ShadowStrikeStory.ini` to the matching folder beside `GhostOfTsushima.exe`.

If you do not already use an ASI loader, copy the included `d3d12.dll` beside `GhostOfTsushima.exe`. It is the official x64 Ultimate ASI Loader 9.7.4, renamed to a supported proxy filename. This loading method has been documented by the GoT Native Loader proof of concept: https://www.nexusmods.com/ghostoftsushima/mods/845 .

If `d3d12.dll` already exists, do not overwrite it. Check whether your existing loader supports ASI plugins; otherwise resolve the loader conflict before installing this package. This package does not require the GoT Mod Launcher.

Start the game normally and load a campaign save. The plugin applies the ability automatically when the player, Ultimate slot and Shadow Strike resource are available. `plugins/ShadowStrikeStory.log` records successful application or reasons for staying inactive.

## Toggle and configuration

Press F8 to toggle. Disabling requests restoration of the original Ultimate on the game's update thread. The log confirms restoration after the slot is checked. F8 updates `Enabled` in the INI; you can also edit it manually. The plugin reloads the configuration once per second while the update function runs.

`ToggleKey=119` selects F8 using a Windows virtual-key code. Use `ToggleKey=0` to disable the shortcut. Settings are beside the ASI file, so the folder must permit writing to persist hotkey changes.

After death, fast travel or save reload, the plugin reads the current player and manager again; it does not reuse another player's backup. If a swap fails, it attempts restoration and stops repeated attempts for that entity.

## Uninstall

Disable with F8 while controlling Jin and check the restoration log. Close the game and remove this plugin's ASI, INI and log. Remove the included `d3d12.dll` only if you installed it solely for this mod and no other mods use it. Original game archives are not edited.

## Validation status

The Cheat Engine prototype's equip and restore path was tested in the campaign by the user, who reported functioning Shadow Strike without a crash, restoration, and additional gameplay checks. These are user reports for the prototype, not independent validation of this ASI binary.

The ASI compiled successfully, passed state-machine tests for application, restoration, Legends exclusion, player reload, external ownership, slot reset and failed-swap recovery, and loaded successfully in an independent Windows harness. Its game-process check left it inactive outside GhostOfTsushima.exe. Automatic loading in the actual game was confirmed by the plugin log, including successful application and a verified Ultimate slot. The player then confirmed that Shadow Strike worked in the campaign with this ASI installed and without Cheat Engine. Live F8 restoration and the full death/fast-travel/save-reload sequence have not been independently confirmed for this ASI build; the prototype had reported gameplay and restoration checks. Released as a beta with feedback welcome.

For feedback, include executable version, ASI version, your loader and other gameplay mods, exact reproduction steps, and `ShadowStrikeStory.log`.

## Credits and license

Musky / Musky689: Master Thyself 18.4, resource hash, asset-editor path and foundational research used for this plugin. https://www.nexusmods.com/ghostoftsushima/mods/735 . The author's mod permissions allow modification with credit and prohibit sale; keep this package free.

Destinate and cfemen: foundational Cheat Engine work credited by Master Thyself.

Tsuda Kageyu and contributors: MinHook 1.3.4, included in the plugin binary under its bundled license.

ThirteenAG and contributors: Ultimate ASI Loader 9.7.4, included under the MIT license. https://github.com/ThirteenAG/Ultimate-ASI-Loader .

New implementation generated with Codex for the user's mod project. Source and dependency licenses are included. Not affiliated with Sucker Punch, Nixxes, Sony or Nexus Mods.
