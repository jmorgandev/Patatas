# Patatas

An emulator for the CHIP-8 Virtual Console, originally developed by Joseph Weisbecker for the COSMAC VIP and Telmac 1800.

CHIP-8 emulators tend to differ between implementations, which typically results in undefined behaviour for any ROMs that were developed for a specific type of implementation.

This emulator attempts to solve this by providing a way to "toggle" certain opcode implementations so that existing ROMs can run smoothly. It also provides a way to adjust features of the VM such as the sound, colors, speed, and keybinds.

These are in no way unique ideas, as many modern implementations of CHIP-8 emulators make sure to include different opcode implementations or at least declare which ones are in use.

What sets this emulator apart is that the CHIP-8 ROM themselves will be checked for specific settings to change opcode implementations, colors, and speed, without prompting the user in any way (Such as on screen or in a readme.txt). This way, ROMs can be programmed with it's own settings in mind, without relying on any particular standard or any overly-defensive programming.