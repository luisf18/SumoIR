#include "IR_decoder.h"

#define IR IR_IN

// Led macros
#define LED_PIN  8
#define LED_INIT(){pinMode(LED_PIN,OUTPUT);  }
#define LED_ON(){  digitalWrite(LED_PIN,LOW);}
#define LED_OFF(){ digitalWrite(LED_PIN,HIGH);}
#define LED_TOGGLE(){ digitalWrite(LED_PIN,!digitalRead(LED_PIN)); }

// robot states
enum{
  ROBOT_OFF = 0,
  ROBOT_PREPARE,
  ROBOT_ON
};

// sony comands
enum{
  SONY_CMD_1 = 0,
  SONY_CMD_2,
  SONY_CMD_3,
};

// globals
uint8_t  robot_state = ROBOT_OFF;
uint32_t blink_timeout = 0;


// Main code
void setup() {
  
  // Begin Serial
  Serial.begin(115200);
  Serial.setTimeout(80);
  LED_INIT();
  LED_ON();
  delay(10);

  // sensor IR
  IR.begin(7);
  //IR.onRecive( IR_handler );

  Serial.println("INIT!");
  LED_OFF();
  
}


void loop() {
  
  // Check IR updates
  if( IR.update() ){
    
    Serial.println( IR.string() );

    // Logica de estados
    if( IR.isSony() ){
      switch( IR.command() ){
        case SONY_CMD_1: Serial.println("[PREPARE]"); robot_state = ROBOT_PREPARE; LED_OFF(); blink_timeout = 0; break;
        case SONY_CMD_2: Serial.println("[START]");   robot_state = ROBOT_ON;      LED_ON();  break;
        case SONY_CMD_3: Serial.println("[STOP]");    robot_state = ROBOT_OFF;     LED_OFF(); break;
      }
    }

  }

  // update led blink
  if( robot_state == ROBOT_PREPARE ){
    if( blink_timeout < millis() ){
        blink_timeout = millis() + 200;
        LED_TOGGLE();
    }
  }

}
