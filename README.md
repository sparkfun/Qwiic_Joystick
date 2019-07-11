SparkFun Qwiic Joystick
========================================

![SparkFun Qwiic Joystick](https://cdn.sparkfun.com/assets/parts/1/3/5/5/8/15168-SparkFun_Qwiic_Joystick-01.jpg)

[*SparkFun Qwiic Joystick (COM-15168)*](https://www.sparkfun.com/products/15168)

Now, you can easily add an HID/controller to your project! The [SparkFun Qwiic Joystick](https://www.sparkfun.com/products/15168) combines the convenience of the Qwiic connection system and an analog joystick that feels reminiscent of the _thumbstick_ from a PlayStation 2 controller.

Thanks to an ATtiny85 on the [Qwiic Joystick](https://www.sparkfun.com/products/15168), all the necessary bits are taken care of and your microcontroller only needs to look for your inputs in the registers of the I<sup>2</sup>C device.

<h3 style="text-decoration: underline;">Factory Default I2C Slave Address: 0x20</h3>

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

`uint_16t full10bitvalue = (MSB | LSB)>>6;`

You could potentially only look at the MSB and get an 8-bit (256 position) reading. The firmware was intentionally written this way in the hopes that it would be useful for customers who don't need the full resolution of the joystick position.

Repository Contents
-------------------

* **/Firmware** - Firmware for ATtiny85 and Python Example Code 
* **/Hardware** - Eagle design files (.brd, .sch)

Documentation
--------------
* **[Library](https://github.com/sparkfun/SparkFun_Qwiic_Joystick_Arduino_Library)** - Arduino library for the Qwiic Joystick.
* **[Hookup Guide](https://learn.sparkfun.com/tutorials/qwiic-joystick-hookup-guide)** - Basic hookup guide for the Qwiic Joystick.

Product Versions
----------------
* [COM-15168](https://www.sparkfun.com/products/15168)- Retail packaging of standard description here

Version History
---------------
* [SparkFun Qwiic Joystick COM-15168](https://www.sparkfun.com/products/15168) - Firmware was update so that the board acts like a *normal* I2C slave device, the silk screen was changed to follow retail product guidelines, and the ISP pins were reconfigured for programming on a test bed.
* [SparkX Qwiic Joystiic v1.1 SPX-14656](https://www.sparkfun.com/products/14656) - Version 1.1 is a minor revision which adds a power LED and corrects the wide IC footprint. It's functionally identical to the previous version.
* [SparkX Qwiic Joystiic SPX-14619](https://www.sparkfun.com/products/retired/14619) - Original SparkX Joystiic.

License Information
-------------------

This product is _**open source**_! 

Please review the LICENSE.md file for license information. 

If you have any questions or concerns on licensing, please visit the [SparkFun Forum](https://forum.sparkfun.com/index.php) and post a topic. For more general questions related to our qwiic system, please visit this section of the forum: [SparkFun Forums: QWIIC SYSTEMS](https://forum.sparkfun.com/viewforum.php?f=105)

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
