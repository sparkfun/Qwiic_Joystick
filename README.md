# Qwiic_Joystiic
Thumbstick Breakout for the Qwiic System
<br/><br/>
Factory Default I2C Slave Address: 0x20<br/>
<br/>
<h3 style="text-decoration: underline;">I2C Registers</h3><br/>

| Address | Contents |
| ------- | -------- |
| 0x00 | Default I2C Address |
| 0x01-0x02 | Firmware Version |
| 0x03-0x04 | Horizontal Position (MSB First) |
| 0x05-0x06 | Vertical Position (MSB First) |
| 0x07 | Button Position |
| 0x08 | Button Status: Indicates if button was pressed since last read of button state. Clears after read. |
| 0x09 | Lock Register for I2C Address Change |
| 0x0A | Current I2C Slave Address. Can only be changed once Lock Register is set to 0x13, then it clears the Lock Register. |

In the registers for the joystick position, the MSB contains the first 8 bits of the 10-bit ADC value and the LSB contains the last two bits. As an example, this is how the library converts the two registers back to a 10-bit value.

`uint_16t full10bitvalue = MSB << 2 | LSB;`

You could potentially only look at the MSB and get an 8-bit (256 position) reading. The firmware was intentionally written this way in the hopes that it would be useful for customers who don't need the full resolution of the joystick position.
