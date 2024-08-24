To program a song to Uolevi, open the "Songs" directory and create a file called "<song_name>.txt". Copy the .wav song file to be programmed to the same directory.

Different programming parameters are separated by lines, and data values are separated by spaces. The first line should contain the song file name. The next 4 lines should contain toggle times in seconds for the leg motor, mouth motor, left eye LED, and right eye LED, respectively.

Run the programmer.py script in the "Python" directory and input the "<song_name>.txt" file name for programming. Finally copy the created "<song_name>.ulv" to the root directory of the SD card and rename to indicate order ("<0-9>.ulv") in songs to load to Uolevi.
A song is loaded into active memory by inserting the SD card into Uolevi and holding Uolevi's upper left hand button down until the eye LEDs have turned on and off. This can be repeated to select the desired song, indicated by the number of beeps (1-10). Wait until the song starts playing to make sure loading is finished.

Below is an example of a programming file.
---
example.wav
0.0 1.0 5.0 5.1
0.01 0.012 0.053 0.1 2.0 2.1

1.0 4.8 5.25
---

The .ulv file specification begins with a 4-byte header consisting of a 32-bit unsigned integer N, encoded as little endian. The header is then followed by N data bytes.
The first data byte contains the first 2 "mechanical" samples with the following bit assignment: 0 (LSB) - leg motors 1st sample, 1 - mouth motor 1st sample, 2 - left eye LED 1st sample, 3 - right eye LED 1st sample, 4 - leg 2nd sample, 5 - mouth 2nd sample, 6 - left eye 2nd sample, 7 (MSB) - right eye 2nd sample.
The next 1492 data bytes contain the first 1492 audio samples encoded as 8-bit unsigned integers. These are then followed by the next mechanical sample byte and the next 1492 data bytes, and so on, until the end of the file.