# ULV File specification

ULV 1.0
- The ULV 1.0 file specification begins with a 4-byte header consisting of a 32-bit unsigned integer N, encoded as little endian. The header is then followed by N data bytes.
- The first data byte contains the first 2 "mechanical" samples with the following bit assignment: 0 (LSB) - leg motors 1st sample, 1 - mouth motor 1st sample, 2 - left eye LED 1st sample, 3 - right eye LED 1st sample, 4 - leg 2nd sample, 5 - mouth 2nd sample, 6 - left eye 2nd sample, 7 (MSB) - right eye 2nd sample.
- The next 1492 data bytes contain the first 1492 audio samples encoded as 8-bit unsigned integers. These are then followed by the next mechanical sample byte and the next 1492 data bytes, and so on, until the end of the file.
