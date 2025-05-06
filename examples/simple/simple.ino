/* ===============================================================================
 *  SumoIR Library
 *    Author: luisf18 (github)
 *      
 *  Exemplo basico de uso
 * ===============================================================================
 */

#include "SumoIR.h"

SumoIR IR;

void setup(){
  
  Serial.begin(115200);
  IR.begin(15); // sensor conectado no pino 15

  // comente se não houver LED
  IR.setLed(2,HIGH,180); // led no pino 2, acende quando esta HIGH e irá piscar com intervalo de 150ms

  // IR.debug(false); // <- descomente se não quiser debug na serial

}

void loop() {

    IR.update();
    if ( IR.on() ) {
      
      /* codigo do robô ligado */

    }else{
      
      /* codigo do robô desligado */

    }
}
