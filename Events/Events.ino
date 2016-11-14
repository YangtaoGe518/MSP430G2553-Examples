/* Events
The code illustrates how to deal with synchronous and asynchronous events. In particular it illustrates how to use
interrupts to capture a user pressing a switch and how tho use the millisecond timer to trigger regular events.

An edge interrupt is used to capture the falling and rising edge signal when a switch is pressed.
The code counts the number of edges (Bounces) that are observed when the switch is pressed and sends this data back
over the serial port. If switch bounce is not properly dealt with this will result in unexpected operation. e.g. if
the switch is used to increment a counter then multiple increments may occur with what the user thought was a single
switch press.

The code uses the millisecond timer to determine when the switch input has stopped bouncing by waiting for a time, set
by the DEBOUNCE_TIME parameter, after the last edge detect interrupt occurs before setting the pressed flag.
The DEBOUNCE_TIME should be adjusted to give the best trade off between rejecting the switch bounce and reponsivity
from a user perspective.

The red LED is used to indicate when the switch is pressed (LED on) and released (LED off).
A simple state machine is implemented to show the effect of pressing the switch. On each switch press the state machine should
advance in a circular manner through the four states indicated by the Green LED Green: OFF, ON, SLOW FLASH, FAST FLASH

The flash operation also shows how you can efficiently carry out actions at regular intervals.

Benn Thomsen, November 2016
*/


#define DEBOUNCE_TIME 50   // Time to wait after last detected edge. (This such be adjusted so that it is lon
#define S2 5               // Switch 2 (S2) is connected to pin 5 on the MSP430G2553
#define OFF 0              // Green LED OFF state
#define ON 1               // Green LED ON  state
#define SLOW_FLASH 2       // Slow Flash  state
#define FAST_FLASH 3       // Fast Flash state

#define SLOW 1000          // Green LED ON  state
#define FAST 250           // Green LED ON  state

// These global variables are defined as volatile variables because they are changed inside interrupts
volatile unsigned long last_time = 0;   // time for first falling edge 
volatile int fall_count = 0;            // variable to hold the number of falling edges detected on a single switch press
volatile int rise_count = 0;            // variable to hold the number of rising edges detected on a single switch release

// Global variables
boolean pressed = false;                // flag to indicate that the switch is pressed
boolean led = false;                    // flag to indicate current state of green led
boolean flash = false;                  // flag to indicate flash operation enabled
unsigned long last_flash = 0;           // last flash time (ms)
int flash_time = 1000;                  // flash interval
int state = OFF;                        // State variable


void setup()
{ 
  Serial.begin(9600);                     // Initialise Serial port for user output

  pinMode(S2, INPUT_PULLUP);              // Set pin mode for S2 to input and enable pullup resistor
  attachInterrupt(S2, falling, FALLING);  // Attach Interrupt to detect falling edge on S2
  
  pinMode(RED_LED, OUTPUT);              // Set pin 2 (red LED) mode to output
  digitalWrite(RED_LED,LOW);             // Turn red LED off
  
  pinMode(GREEN_LED, OUTPUT);            // Set pin 14 (green LED) mode to output
  digitalWrite(GREEN_LED,LOW);           // Turn green LED off
  
  last_time = millis();                  //Start millis timer
}

void loop()
{
  // If falling edage has been detected and the delay from the detection of the last falling edge is > DEBOUNCE_TIME
  if (fall_count && ((millis() - last_time) > DEBOUNCE_TIME)) {
    Serial.print("Switch pressed. Count: ");
    Serial.println(fall_count);             // send falling edge count to serial port
    fall_count = 0;                         // Reset falling edge count
    pressed = true;                         // set flag to indicate that switch is pressed
    if (!digitalRead(S2)) {
      attachInterrupt(S2, rising, RISING);    // Attach rising edge Interrupt to detect release of switch
      digitalWrite(RED_LED,HIGH);             // turn RED LED on to indicate switch is pressed
    }
  }
  
  if (rise_count && ((millis() - last_time) > DEBOUNCE_TIME)) {
    Serial.print("Switch released. Count: ");
    Serial.println(rise_count);             // send rising edge count to serial port
    rise_count = 0;                         // Reset rising edge count
    if (digitalRead(S2)) {
      attachInterrupt(S2, falling, FALLING);  // Attach falling edge Interrupt to detect next press of switch
      digitalWrite(RED_LED,LOW);              // turn RED LED off to indicate switch release
    }
  }
  
  // Service Switch pressed action
  if (pressed) {
    pressed = false;
    // State machine
    switch (state) {
      case OFF:
        digitalWrite(GREEN_LED,HIGH);
        state = ON;
        Serial.println("State: ON");
        break;
      case ON:
        flash = true;
        flash_time = SLOW;
        state = SLOW_FLASH;
        Serial.println("State: SLOW FLASH");
        break;
      case SLOW_FLASH:
        flash = true;
        flash_time = FAST;
        state = FAST_FLASH;
        Serial.println("State: FAST FLASH");
        break;
      case FAST_FLASH:
        flash = false;
        digitalWrite(GREEN_LED,LOW);
        state = OFF;
        Serial.println("State: OFF");
        break;
    }
  } 
  
  // Service led flash if enabled
  if (flash && (millis()-last_flash > flash_time)) {
    led = !led;
    digitalWrite(GREEN_LED,led);
    last_flash = millis();
  }
  
}

// Interupt Service Routines (Note: these should be short)

/* Code to execute on falling edge interrupt*/
void falling()
{
  fall_count++;
  last_time = millis();
}

/* Code to execute on rising edge interrupt */
void rising()
{
  rise_count++;
  last_time = millis();
}

