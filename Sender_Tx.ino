/**
 * By Ramiro Gutierrez Alaniz
 * Date October 31st, 2016
 * Router arduino code
 * Note : xbee code was obtained by the example from the xbee library documentation
 */

// Xbee library
#include <XBee.h>

// Xbee component from library
XBee xbee = XBee();

unsigned long start = millis();

// allocate two bytes for to hold a 10-bit analog reading
uint8_t payload[] = { 0, 0 };

// 16-bit addressing: Enter address of remote XBee, typically the coordinator
//Tx16Request tx = Tx16Request(0x1874, payload, sizeof(payload));

// 64-bit addressing: This is the SH + SL address of remote XBee
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x40d661c1);
// unless you have MY on the receiving radio set to FFFF, this will be received as a RX16 packet
Tx64Request tx = Tx64Request(addr64, payload, sizeof(payload));
// Transmitter response variable
TxStatusResponse txStatus = TxStatusResponse();

// Control variables
const int UP = 2;
const int DOWN = 3;
const int LEFT = 4;
const int RIGHT = 5;

const int TEST = 6;
const int STOP_BREAK = 7;

// state leds
const int SUCCESS_LED = 12;
const int ERROR_LED = 13;

// Signal variables
int up_signal = 0;
int down_signal = 0;
int left_signal = 0;
int right_signal = 0;
int test_signal = 0;
int stop_break_signal = 0;

// Directions 
enum { 
    FRONT = 5, 
    BACK = 10, 
    RIGHT_FRONT = 4, 
    LEFT_FRONT = 1, 
    LEFT_BACK = 2, 
    RIGHT_BACK = 8, 
    STOP = 0  
  } direction_type;

/*
 * setup function
 * init all the components
 */
void setup() {
  // set as outputs button pins
  pinMode( UP, OUTPUT );
  pinMode( DOWN, OUTPUT );
  pinMode( LEFT, OUTPUT );
  pinMode( RIGHT, OUTPUT );
  // set outputs test and break pins
  pinMode( TEST, OUTPUT );
  pinMode( STOP_BREAK, OUTPUT );
  // state led init
  pinMode( SUCCESS_LED, OUTPUT );
  pinMode( ERROR_LED, OUTPUT );
  // set state error
  set_state( 0 );
  // serial init for monitoring
  Serial.begin( 9600 );
  // Serial init on the xbee
  Serial1.begin( 9600 );
  // Set second serial to the xbee
  xbee.setSerial(Serial1);
}// End of setup function

/*
 * Loop function
 * Main action of the arduino
 */
void loop() {
    // Get signals fron the buttons
    up_signal = digitalRead( UP );
    down_signal = digitalRead( DOWN );
    left_signal = digitalRead( LEFT );
    right_signal = digitalRead( RIGHT );
    test_signal = digitalRead( TEST );
    stop_break_signal = digitalRead( STOP_BREAK );
    // start transmitting after a startup delay.  Note: this will rollover to 0 eventually so not best way to handle
    if (millis() - start > 15000) {
      payload[0] = get_instruction();
      xbee.send(tx);
    }
    // after sending a tx request, we expect a status response, wait max 5 seconds for it
    if (xbee.readPacket(5000)) {
      // got a response            	
    	if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
    	   xbee.getResponse().getTxStatusResponse(txStatus);
         // Verify if the response is actually a success
         if( txStatus.getStatus() == SUCCESS ) {
            set_state( 1 );
            Serial.println( "Message sended" );
         } else {
            set_state( 0 );
            Serial.println( "The message has't been sended." );
         }
      }      
    } else if( xbee.getResponse().isError() ) {
      set_state( 0 );
      Serial.println( "There was an error with the module." );
    }
    // a small delay for wating for response
    // small as it is at close range and is just a bit of data
    delay(100);
}// End of loop function

/*
 * set state
 * set the led state tu success if true else error
 */
void set_state( int state ) {
  if( state ) {
    digitalWrite( SUCCESS_LED, HIGH );
    digitalWrite( ERROR_LED, LOW );
  } else {
    digitalWrite( SUCCESS_LED, LOW );
    digitalWrite( ERROR_LED, HIGH );
  }
} // End of set state function

/*
 * get_instruction
 * function that returns the number of instruction that represents the comand on the receiber arduino
 */
int get_instruction() {
  
  if( stop_break_signal == LOW ) 
  {
    if( 
      up_signal == HIGH &&
      down_signal == LOW && 
      left_signal == LOW &&
      right_signal == LOW
    ) {
      Serial.println( "UP");
      return FRONT;
    } else if( 
      up_signal == LOW &&
      down_signal == HIGH && 
      left_signal == LOW &&
      right_signal == LOW
    ) {
      Serial.println( "DOWN" );
      return BACK;
    } else if( 
      up_signal == HIGH &&
      down_signal == LOW && 
      left_signal == HIGH &&
      right_signal == LOW
    ) {
      Serial.println( "LEFT FRONT" );
      return LEFT_FRONT;  
    } else if( 
      up_signal == HIGH &&
      down_signal == LOW && 
      left_signal == LOW &&
      right_signal == HIGH
    ) {
      Serial.println( "RIGHT FRONT" );
      return RIGHT_FRONT;  
    } else if( 
      up_signal == LOW &&
      down_signal == HIGH && 
      left_signal == HIGH &&
      right_signal == LOW
    ) {
      Serial.println( "LEFT DOWN" );
      return LEFT_BACK;
    } else if( 
      up_signal == LOW &&
      down_signal == HIGH && 
      left_signal == LOW &&
      right_signal == HIGH
    ) {
      Serial.println( "RIGHT DOWN" );
      return RIGHT_BACK;  
    }
  } else {
    Serial.println( "STOP" );
    return STOP;
  }

} // End of get_instruction function
