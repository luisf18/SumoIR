#include "SumoIR.h"

SumoIR IR;

int strategy = 0; // estrategia

void setup(){
  
  Serial.begin(115200);
  IR.begin(15); // sensor conectado no pino 15

  // comente se não houver LED
  IR.setLed(2,HIGH,180); // led no pino 2, acende quando esta HIGH e irá piscar com intervalo de 150ms

  // IR.debug(false); // <- descomente se não quiser debug na serial

}

void loop() {

    IR.update();

    if ( IR.prepare() ) {

      /* robô em preparação */

      Serial.println("-> sumo prepare");
      delay(400);

    }else if ( IR.start() ) {

      /* chamado apenas uma vez quando se torna esse estado */
      /* movimento inicial */
      
      Serial.println("-> sumo start");
    
    } else if ( IR.on() ){

      /* codigo do robô ligado */
      
      Serial.println("-> sumo on");
      delay(400);
      
    }else if( IR.stop() ){
      
      /* chamado apenas uma vez quando se torna esse estado */
      /* codigo que desliga o robô */

      Serial.println("-> sumo stop");

    }else{

      /* codigo do robô desligado */

      Serial.println("-> sumo off");
      delay(400);

    }
}
