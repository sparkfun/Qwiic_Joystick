import smbus, time
bus = smbus.SMBus(1)
addr = 0x20

def main():
    global bus_data, X, Y
    while True:
        qwiicjoystick()


def qwiicjoystick():
    global bus_data, X, Y
    

    try:
        bus_data = bus.read_i2c_block_data(addr, 0x03, 5)
        #X_MSB = bus.read_byte_data(addr, 0x03) # Reads MSB for horizontal joystick position
        #X_LSB = bus.read_byte_data(addr, 0x04) # Reads LSB for horizontal joystick position
    
        #Y_MSB = bus.read_byte_data(addr, 0x05) # Reads MSB for vertical joystick position
        #Y_LSB = bus.read_byte_data(addr, 0x06) # Reads LSB for vertical joystick position

        #Select_Button = bus.read_byte_data(addr, 0x07) # Reads button position
    except Exception as e:
        print(e)

    X = (bus_data[0]<<8 | bus_data[1])>>6
    Y = (bus_data[2]<<8 | bus_data[3])>>6
    
    print(X, Y, " Button = ", bus_data[4])
    
    time.sleep(.05)
    #if X < 450:
        #direction = RIGHT
    #elif 575 < X:
        #direction = LEFT

    
    #if Y< 450:
        #direction = DOWN
    #elif 575 < Y:
        #direction = UP

    #if Select_Button == 1:
        #terminate()
        
if __name__ == '__main__':
    main()
