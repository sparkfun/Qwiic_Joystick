# Qwiic_Joystiic
Thumbstick Breakout for the Qwiic System
<br/><br/>
Factory Default I2C Slave Address: 0x20<br/>
<br/>
<h3 style="text-decoration: underline;">I2C Registers</h3><br/>

| Address | Contents |
| ------- | -------- |
| 0x00-0x01 | Horizontal Position (MSB First) |
| 0x02-0x03 | Vertical Position (MSB First) |
| 0x04 | Button Position |
| 0x06 | Current I2C Slave Address from EEPROM |
<br/>
<h3 style="text-decoration: underline;">I2C Commands</h3><br/>

| Address | Command |
| ------- | ------- |
| 0x05 | Set I2C Slave Address |
