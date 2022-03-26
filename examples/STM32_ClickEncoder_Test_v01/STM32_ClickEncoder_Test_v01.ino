/*
Updated ClickEncoder library for STM32
Original library is from https://github.com/0xPIT/encoder
Updated by IF 
2022-03-26
- tested with Maple Mini board / Arduino STM32 (https://github.com/rogerclarkmelbourne/Arduino_STM32/)
- tested with SSD1306 128_32 I2C OLED Display /  SSD1306_i2c library for STM32 (https://github.com/i998/SSD1306_i2c_STM32)

Hardware Configuration:

Maple Mini Pin 12 <--> Encoder CLK 
Maple Mini Pin 13 <--> Encoder DT   
Maple Mini Pin 14 <--> Encoder Switch  

By default, active signals from Encoder and its button are LOW (i.e Encoder pins are pulled up to VCC) 

If a LCD display is used:
Maple Mini Pin 0 <--> I2C Display SDA  
Maple Mini Pin 1 <--> I2C Display SCL 

=================================================================*/

// Example Settings 
//Uncomment to use LCD screen
//#define WITH_LCD 1

//Pins for ClickEncoder
#define CE1 12     //CLK
#define CE2 13    // DT
#define CE3 14    // SW


//If a local copy of the library is used for this example 
//#include "src/ClickEncoder.h"

//Usual path
#include <ClickEncoder.h>


//Create STM32 Timer 
	#define TIMER_RATE 1000    // in microseconds; should give 1 millisecond  toggles

	// We'll use timer 2
	HardwareTimer timer(2);



#ifdef WITH_LCD

#include <SSD1306_i2c.h>
#include <Adafruit_GFX.h>

//Setup I2C interface
// hardware interface:
	//TwoWire I2C_FAST=TwoWire(1,I2C_FAST_MODE); //I2C #1  
	TwoWire I2C_FAST=TwoWire(2,I2C_FAST_MODE); //I2C #2


//Set Up Display(s) 
#define LCD_RESET 4 //Pin number that can be used for  SSD1306 reset (not used in this test)
//Setup the SSD1306_i2c with a pointer to the I2C interface 
	SSD1306_i2c lcd = SSD1306_i2c(&I2C_FAST,LCD_RESET);

#endif


//Setup ClickEncoder 
ClickEncoder encoder1 = ClickEncoder(CE1, CE2, CE3);


//Set STM32 Timer interrupt service routine function
void TimerISR(void) {
	encoder1.service();
}

//Variables to store encoder values 
int16_t last, value;


#ifdef WITH_LCD

//Displays Encoder Acceleration Status on LCD display
void displayAccelerationStatus() {
  lcd.clearDisplay();
  lcd.setTextSize(1);
  lcd.setTextColor(WHITE);
  lcd.setCursor(0, 16);  
  lcd.print("Acceleration ");
  lcd.print(encoder1.getAccelerationEnabled() ? "on " : "off");
  lcd.display();
}
#endif

void setup() {

//==============================
   Serial.begin(9600);
   //Serial.begin(115200);
   //The program will wait for serial to be ready up to 10 sec then it will continue anyway  
     for (int i=1; i<=10; i++){
          delay(1000);
     if (Serial){
         break;
       }
     }
    Serial.println("Setup() started ");
  //===============================

 
//Setup STM32 Timer   
    // Pause the timer while we're configuring it
    timer.pause();

    // Set up period
    timer.setPeriod(TIMER_RATE); // in microseconds

    // Set up an interrupt on channel 1
    timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
    timer.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
	// Attach the timer interrupt handler 
    timer.attachCompare1Interrupt(TimerISR);

    // Refresh the timer's count, prescale, and overflow
    timer.refresh();

    // Start the timer counting
    timer.resume();


#ifdef WITH_LCD

//Initialize the SSD1306 library
	//SSD1306_SWITCHCAPVCC: by default, we'll generate the high voltage from the 3.3v line internally
	//0x3C: initialize with the I2C addr 0x3C for the 128x32  (0x3D for the 128x64)
	//false: we do not use hardware reset
	lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);  
  
	
	//Uncomment to just check LCD works - shall show Adafruit logo on start	
	//lcd.display();
    //delay(1000);
	
	
  
	// Clear the buffer and transfer it to the display
	lcd.clearDisplay();
    lcd.display();
	
	#endif

 //No readings from Encoder yet
  last = -1;
}

void loop() {  
  value += encoder1.getValue();
  
  if (value != last) {
    last = value;
    Serial.print("Encoder Value: ");
    Serial.println(value);
	
#ifdef WITH_LCD
//Compile the screen
  lcd.clearDisplay();
  lcd.setTextSize(2);
  lcd.setTextColor(WHITE);
  lcd.setCursor(0,0);
  lcd.print(value);
     //display the compiled picture		 
  lcd.display();
#endif

  }
  
  ClickEncoder::Button b = encoder1.getButton();
  if (b != ClickEncoder::Open) {
    Serial.print("Button: ");
    #define VERBOSECASE(label) case label: Serial.println(#label); break;
    switch (b) {
      VERBOSECASE(ClickEncoder::Pressed);
      VERBOSECASE(ClickEncoder::Held)
      VERBOSECASE(ClickEncoder::Released)
      VERBOSECASE(ClickEncoder::Clicked)
      case ClickEncoder::DoubleClicked:  //toggle acceleration on button double click 
          Serial.println("ClickEncoder::DoubleClicked");
          encoder1.setAccelerationEnabled(!encoder1.getAccelerationEnabled());
          Serial.print("  Acceleration is ");
          Serial.println((encoder1.getAccelerationEnabled()) ? "enabled" : "disabled");
		  
#ifdef WITH_LCD
          //Display AccelerationStatus
          displayAccelerationStatus();
#endif
        break;
    }
  }    
}
