# Uolevi

How to use Uolevi:
1. To PLAY a song, you can press the reset button in Uolevi's arm.
2. To STOP a song, you can hold the mode switch closer to the shoulder of the same arm until the eye LEDs turn on.
3. To CHANGE the song, you can keep holding the mode switch until the LEDs turn off, after which the number of beeps will indicate which song is selected. To select the next song, you can repeat this process. If the song selection is not indicated, you can retry changing the song, or reset Uolevi and retry. When you hear the correct number of beeps, you need to wait until the song is copied onto onboard memory and Uolevi plays the song. The eye LEDs indicate a loading is in progress. At most the loading time is approximately 6 minutes for the longest duration of song programmable (9 min 21 s).
4. To ADD new songs, you can take out the micro SD card in Uolevi's back, to the left of the battery compartment, and follow the programming instructions in the "Programming" directory.
---

Project documentation is split into the following directory structure:
- 3D Models: 3D-printable mechanical components
- Electronics: Schematics and bill of materials for PCB design
- Firmware: Code for microcontroller firmware
- Programming: Software and instructions for programming new songs onto Uolevi

Estimated current draw of Uolevi Rev 1.1 for functionalities and modes of operation at 4.5 V:
- Mouth motor: 0.9 A
- Legs motor: 0.4 A
- Eye LEDs: 2 x 20 mA
- Speaker: 40 mA
- Loading song from SD card onto external flash: 40 mA
- Standby mode (software power off): 4 mA
- Power off (timer chip 3.3 V cutoff): 50 nA
