/******************************************************************************
Qwiic Starter Kit Game 
Owen Lyke @ SparkX
June 8th 2018

This sketch shows how Qwuiicly you can make a snake-like game (minus death!) 
All parts (BlackboardMini OLED Display, Acceleromenter, and Joystiic) are 
included in the Qwiic Starter Kit (available: https://www.sparkfun.com/products/14752)

Wiring:
The hardware setup is incredibly simple thanks to the magic of I2C! All you 
have to do is connect all the parts to one another with the Qwiic cables and
make sure that they are connected to the BlackBoard. The order in which the
parts are all strung together doesn't matter!

Playing the Game:
Make sure the libraries for the OLED and the Accelerometer (MMA8452Q) are
installed, then compile and upload the code to the BlackBoard. 
With the acclerometer flat use the Joystiic to move the ball toward the targets. 
For an additional challenge tilt the accelerometer - this will cause 'gravity' 
to affect the ball. 
You can even use the accelerometer as your controller!

Going Further:
Now you know how to use the Qwiic system and you have some sensors to play with.
Try using the VCNL4040 proximity sensor to reset the score when you wave your hand.
Try changing the code so that the ball bounces off walls!
The sky's the limit when it comes to making your ideas into reality. Check out
the host of other Qwiic parts like pressure sensors, MP3 players, thermal imagers, 
and lots more!
******************************************************************************/

#include <Wire.h>               // Qwiic parts use the I2C communication protocol, handled by Wire
#include <SFE_MicroOLED.h>      // Include the SFE_MicroOLED library                                    http://librarymanager/All#Sparkfun_Micro_OLED_Breakout
#include <SparkFun_MMA8452Q.h>  // Includes the SFE_MMA8452Q library                                    http://librarymanager/All#Sparkfun_MMA8452Q


//////////////////////////
// MicroOLED Definition //
//////////////////////////
#define PIN_RESET 9                     // The library assumes a reset pin is necessary. The Qwiic OLED has RST hard-wired, so pick an arbitrarty IO pin that is not being used
#define DC_JUMPER 1                     // Out-of-the-box the OLED's address jumper is open so use (1). If the jumper is closed then use 0 here
MicroOLED oled(PIN_RESET, DC_JUMPER);   // Make an object of the MicroOLED class called "oled"

//////////////////////////////
// Accelerometer Definition //
//////////////////////////////
MMA8452Q accel;                         // Make an object of the MMA8452Q class called "accel"
byte pl;                                // This means "portrait v landscape" but it can actuall hold 5 different values which are determined by the accelerometer

/////////////////////////
// Joystiic Definition //
/////////////////////////
byte JOY_ADDR = 0x20;                   // Not using a library for the Joystiic, so we need to know the I2C address
int reading = 0;                        // This variable will be used to hold the value that the Joystiic returns

/////////////////////////////
// Game Variables (Global) //
/////////////////////////////
unsigned int score = 0;

// User input from Joystiic
byte prevB = 1;   // Previous button value, used to determine "edges" of button presses
byte B = 1;       // Button value now
int H = 0;        // Horizontal ADC counts
int V = 0;        // Vertical ADC counts

// Display options
boolean invertOLED  = false;  
byte r = 4;                     // Ball radius
byte tR = 5;                    // Target radius

// Physics
int pX = 32;    // X position of the ball
int pY = 32;    // Y position of the ball
int vX = 0;     // X velocity of the ball
int vY = 0;     // Y velocity of the ball
int aX = 0;     // X acceleration of the ball
int aY = 0;     // Y acceleration of the ball

// 
int tX = 0;     // Holds the X location of the target
int tY = 0;     // Holds the Y location of the target


void setup() {
  delay(100);       // Delay allows OLED power supply to stabilize
  oled.begin();     // Initialize the OLED
  oled.clear(ALL);  // Clear the display's internal memory
  oled.display();   // Display what's in the buffer (splashscreen)
  delay(1000);      // Delay 1000 ms

  accel.init();     // Start up the accelerometer
  
  randomSeed(analogRead(A0) + analogRead(A1));    // Provide a seed for the random generator
  generateTarget();                               // Use randm functions to set the initial target location
}

void loop() {
  checkJoystiic();          // This function talks to the Joystiic to get the latest values
  checkAccelerometer();     // A similar process for the accelerometer
  updateBall();             // Now with the new controls known we can run the physics of the game to move the ball
  checkTarget();            // And with the ball in a new position we can check to see if it is on the target yet
  printScore();             // Good to know how well you're doing!
  
  oled.display();           // Some above functions modified the data in the OLED's memory, so now tell the display to show that new info    
                            // Don't use a delay in this loop because it already takes long enough for the OLED to refresh
}


void generateTarget( void )
{
  tX = random(2, 61);
  tY = random(2,45);
}

void checkTarget( void )
{
  if(((pX - tX) > -5) && ((pX - tX) < 5))     // Ball is within 5 pixels of target in X
  {
    if(((pY - tY) > -5) && ((pY - tY) < 5))   // Ball within 5 pixels of target in Y
    {
      generateTarget();                       // Create new target
      score++;                                // Good job!
    }
  }
  oled.circle(tX, tY, tR);                    // Draw circle at new target
}

void printScore( void )
{
  char c = '0';
  switch(score)
  {
    case 0 : c = '0'; break;
    case 1 : c = '1'; break;
    case 2 : c = '2'; break;
    case 3 : c = '3'; break;
    case 4 : c = '4'; break;
    case 5 : c = '5'; break;
    case 6 : c = '6'; break;
    case 7 : c = '7'; break;
    case 8 : c = '8'; break;
    case 9 : c = '9'; break;
    default : score = 0; break;
  }
  oled.drawChar(56,2,c);
}

void updateBall( void )
{
  oled.clear(PAGE);

    aX = accel.x/200;       // Use accelerometer readings to set the ball acceleration
    aY = -accel.y/200;

    vX = vX + aX  + ((V-512)/100);  // Add the acceleration (plus influence from Joystiic) to the velocity of the ball
    vY = vY + aY  - ((H-512)/100);

    if(vX > 10)                     // Limit the ball speed in all 4 directions
    {
      vX = 10;
    }
    if(vX < -10)
    {
      vX = -10;
    }
    if(vY > 10)
    {
      vY = 10;
    }
    if(vY < -10)
    {
      vY = -10;
    }

    pX = pX + vX;   // Add the velocity to the position to get a new postition
    pY = pY + vY;

    if(pX < r)        // Then check if the ball has hit any walls (plus a little margin for the size of the ball)
    {
      pX = r;
      vX = 0;
    }
    if(pX > (63-r))
    {
      pX = 63-r;
      vX = 0;
    }

    if(pY < r)
    {
      pY = r;
      vY = 0;
    }
    if(pY >(48-r))
    {
      pY = 48-r;
      vY = 0;
    }

    oled.circleFill(pX, pY, r); // Now draw a ball at the new location  
}


void checkJoystiic( void )
{
  prevB = B;
  
  H = getHorizontal();
  V = getVertical();
  B = getButton();

  if(prevB && (!B))
  {
    invertOLED = (!invertOLED);
    oled.invert(invertOLED);
  }
}

void checkAccelerometer( void )
{
  if (accel.available())
  {
    // First, use accel.read() to read the new variables:
    accel.read();
    pl = accel.readPL();

  }
}




// getHorizontal() returns an int between 0 and 1023 
// representing the Joystiic's horizontal position
// (axis indicated by silkscreen on the board)
// centered roughly on 512

int getHorizontal() {

  Wire.beginTransmission(JOY_ADDR);
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom(JOY_ADDR, 2);

  while (Wire.available()) {
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    reading = (uint16_t)msb << 8 | lsb;
  }

  Wire.endTransmission();
  return reading;

}

// getVertical() returns an int between 0 and 1023 
// representing the Joystiic's vertical position
// (axis indicated by silkscreen on the board)
// centered roughly on 512

int getVertical() {

  Wire.beginTransmission(JOY_ADDR);
  Wire.write(0x02);
  Wire.endTransmission(false);
  Wire.requestFrom(JOY_ADDR, 2);

  while (Wire.available()) {
    uint8_t msb = Wire.read();
    uint8_t lsb = Wire.read();
    reading = (uint16_t)msb << 8 | lsb;
  }

  Wire.endTransmission();
  return reading;

}

// getButton() returns a byte indicating the 
// position of the button where a 0b1 is not
// pressed and 0b0 is pressed

byte getButton() {

  byte btnbyte = 0b00000000;

  Wire.beginTransmission(JOY_ADDR);
  Wire.write(0x04);
  Wire.endTransmission(false);
  Wire.requestFrom(JOY_ADDR, 1);    // request 1 byte

  while (Wire.available()) {
    btnbyte = Wire.read();
  }

  Wire.endTransmission();
  return btnbyte;

}

