/* ===============================================================================
 *  SumoIR Library
 *    Author: luisf18 (github)
 *      
 *  Gerenciamento dos modos:
 *   - IR.prepare() -> 1 somente quando muda de "STOP" para "PREPARE"
 *   - IR.start()   -> 1 somente quando muda de "PREPARE" para "START"
 *   - IR.stop()    -> 1 somente quando muda de "PREPARE" ou "START" para "STOP"
 *   - IR.on()      -> 1 depois de start até receber "STOP"
 *   - IR.off()     -> 1 sempre que esta em "STOP" (o inverso de IR.on())
 * ===============================================================================
 */

#include "SumoIR.h"

SumoIR IR;

void setup(){
  
  Serial.begin(115200);
  IR.begin(15); // sensor conectado no pino 15

  // comente se não houver LED
  IR.setLed(2,HIGH,180); // led no pino 2, acende quando esta HIGH e irá piscar com intervalo de 180ms

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
      
      Serial.print("-> sumo on durante ");
      Serial.print(IR.time_since_start());
      Serial.println(" ms");
      delay(600);
      
    }else if( IR.stop() ){
      
      /* chamado apenas uma vez quando se torna esse estado */
      /* codigo que desliga o robô */

      Serial.print("-> sumo stop, ultimo round: ");
      Serial.print(IR.last_round_duration());
      Serial.println(" ms");

    }else{

      /* codigo do robô desligado */

      Serial.println("-> sumo off");
      delay(600);

    }
}
