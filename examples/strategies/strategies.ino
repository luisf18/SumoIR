/* ====================================================
 *  SumoIR Library
 *    Author: luisf18 (github)
 *      
 *  Este exemplo é um template para selecionar
 *  estratégias diferente no controle.
 *      
 * ====================================================
 */

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

    if( IR.available() ){
      int cmd = IR.read();
      if( cmd >= 4 && cmd <= 9 ){ // faixa de valores validos ( lembrando que 1, 2 e 3 são eservados pra start, stop e prepare)
        strategy = cmd;
      }
    }

    if ( IR.on() ) {

      Serial.println( "running strategy " + String(strategy) );
      
      /* codigo do robô ligado */
      switch( strategy ){
        case 4:
          /* estrategy 4 */
        break;

        case 5:
          /* estrategy 5 */
        break;

        case 6:
          /* estrategy 6 */
        break;

        // ... outras estratégias
      }
      
    }else{
      /* codigo do robô desligado */
    }
}
