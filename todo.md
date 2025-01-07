
# TODO

01/2025 support V3 of Seplos BMS protocol is changed to MODBUS RTU
- Basic testing is active
- CRC need to be created and checkd on receive.

- Auto reconnect mqtt if disconneced? -> testing 08.10.2024
- Is rs485 ist lost. -> Show offline.

2025.01.06 22:58:48:767:Acquire PIA(Pack#0)
00 04 10 00 00 12 75 16 
BMS response OK
00 04 24 15 4B 02 8A 6F 95 71 48 00 00 03 D9 03 E8 00 00 0D 4E 0B 42 0D 54 0D 4C 0B 4A 0B 39 00 00 00 B4 00 B4 03 E8 E2 50 


2025.01.06 22:58:46:913:Acquire PIA(Pack#0)
00 04 10 00 00 12 75 16 
BMS response Error : Format error(CRC)!
3C 00 04 24 15 4B 02 8A 6F 95 71 48 00 00 03 D9 03 E8 00 00 0D 4E 0B 42 0D 54 0D 4C 0B 4A 0B 39 00 00 00 B4 00 B4 03 E8 E2 50 

2025.01.06 22:58:47:308:Acquire PIB(Pack#0)
00 04 11 00 00 1A 75 2C 
BMS response OK
00 04 34 0D 50 0D 4D 0D 4D 0D 4C 0D 4D 0D 4E 0D 50 0D 54 0D 54 0D 50 0D 4E 0D 4C 0D 4C 0D 4F 0D 4D 0D 51 0B 42 0B 39 0B 43 0B 4A 0A AB 0A AB 0A AB 0A AB 0B 2E 0B 14 A7 18 

2025.01.06 22:58:47:498:Acquire PIC(Pack#0)
00 01 12 00 00 90 38 CF 
BMS response OK
00 01 12 00 00 00 00 00 00 00 00 02 00 00 00 00 00 00 03 00 00 CA 7B
