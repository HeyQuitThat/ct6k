/* hw.h - definitions of built-in hardware */

#ifndef __HW_H__
#define __HW_H__
/*
Hardware
Serial Port:
Control reg at 0xFFFFFFF0
  0: Enable serial port
  1: Enable interrupt on rx
TX register at 0xFFFFFFF1 - low byte is sent (MSB first, but only theoretically)
RX register at 0xFFFFFFF2 - low byte is RX data. Bit 31 indicates valid data. Read clears.
Incoming bytes are buffered; new bytes won't appear until previous byte is read.
*/

#endif /* !__HW_H__ */
