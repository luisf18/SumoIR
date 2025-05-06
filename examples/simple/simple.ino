// ==========================================
// IR
// ==========================================
#include "IR.h"
#define IR_PIN 27
bool IRmode = false;
enum{
  PREPARE = 0,
  START,
  STOP
}

// ==========================================
// SENSOR
// ==========================================

#define S_ESQ 21
#define S_DIR 25

void initSensor(){
  pinMode( S_ESQ, INPUT );
  pinMode( S_DIR, INPUT );
}

bool opDirRead(){
  return digitalRead( S_DIR );
}

bool opEsqRead(){
  return digitalRead( S_ESQ );
}

// ==========================================
// Main
// ==========================================

void setup(){
  Serial.begin(115200);
  IR_IN.begin(IR_PIN);
  initSensor();
}

void loop() {
  
  IR_IN.update();
  if(IR_IN.recive()){
    
    Serial.println(IR_IN.string());

    if( IR_IN.read() == 1 ){ // prepare
      if( IRmode == STOP || IRmode == PREPARE ){
        Serial.println("[PREPARE]");
        IRmode = PREPARE;
      }
    }else if( IR_IN.read() == 2 ){ // start
      Serial.println("[START]");
      if( IRmode == PREPARE ){
        IRmode = START;
      }
    }else if( IR_IN.read() == 3 ){ // stop
      Serial.println("[STOP]");
      IRmode = STOP;
    }
  }

  if( IRmode == START ){
    bool dir = opDirRead();
    bool esq = opEsqRead();
    if( dir && esq ){
      motor.move( 1000, 1000 );
      Serial.println("[ATAQUE]");
    }else if( dir ){
      motor.move( 1000, 750 );
      Serial.println("[DIR]");
    }else{
      motor.move( 750, 1000 );
      Serial.println("[ESQ]");
    }
    Serial.printf("[Esq: %d][Dir: %d]\n",esq,dir);
  }

}
