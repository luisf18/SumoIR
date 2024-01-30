/* ====================================================
 *  SumoIR Library
 *    Author: luisf18 (github)
 *    Ver.: 1.0.0
 *      
 *      Processa os sinais de IR para sumôs.
 *      
 *    last_update: 11/08/2023
 *    created:     11/08/2023
 * ====================================================
 */

#ifndef SUMO_IR_H
#define SUMO_IR_H

#include "Arduino.h"
#include <IRremote.hpp>

class SumoIR{

  private:

    int Pin = 15;

    // Decode info and data:
    uint16_t      IN_cmd;
    decode_type_t IN_protocol;
    uint32_t      IN_data;

    // SUMO mode
    uint8_t  Mode = SUMO_STOP;

    int LED;
    int LED_timeout;
    bool LED_state_on;
    uint16_t LED_dt;

    bool DEBUG = true;

    bool Change = false;
    bool Available = false;
    int command = -1;

  public:

    enum{
        SUMO_STOP = 0,
        SUMO_PREPARE,
        SUMO_START
    };

    IRrecv IR_IN = IRrecv(Pin); // Receptor

    // Begin
    void begin(){ begin(Pin); }
    void begin(uint8_t pin){ if(Pin!=pin){ Pin = pin; IR_IN.setReceivePin(Pin); } pinMode(Pin,INPUT_PULLUP); IR_IN.enableIRIn(); }

    bool available(){ return Available; }
    int read(){ return command; };

    // Led
    void setLed( int pin, bool state_on, uint16_t dt ){
      LED = pin;
      LED_state_on = state_on;
      LED_dt = dt;
      if(LED >= 0){
        pinMode( LED, OUTPUT );
        digitalWrite( LED, !LED_state_on );
      }
    }

    void updateLed(){
      if( LED < 0 ) return;
      if( Mode == SUMO_STOP ){
        digitalWrite(LED,!LED_state_on);
      }else if( Mode == SUMO_START ){
        digitalWrite(LED,LED_state_on);
      }else if( Mode == SUMO_PREPARE ){
        if(command == 1){ 
          digitalWrite(LED,LED_state_on);
          LED_timeout = millis() + LED_dt;
        }else if( millis() >= LED_timeout ){
          LED_timeout = millis() + LED_dt;
          digitalWrite(LED,!digitalRead(LED));
        }
      }
    }

    ////////////////////////////// IR INPUT /////////////////////////////////////
    int update() {
      
      Available = false;
      command = -1;
      Change = false;

      if( IR_IN.decode() ){
        IR_IN.resume();

        uint8_t Mode_before = Mode;
        
        IN_cmd      = IR_IN.decodedIRData.command;
        IN_protocol = IR_IN.decodedIRData.protocol;
        IN_data     = IR_IN.decodedIRData.decodedRawData;
        
        Available = true;

        if( IN_protocol == SONY ){
          switch(IN_cmd){
            case 0: command = 1; if( Mode == SUMO_STOP    ) Mode = SUMO_PREPARE; break; // PREPARE
            case 1: command = 2; if( Mode == SUMO_PREPARE ) Mode = SUMO_START; break; // START
            case 2: command = 3; Mode = SUMO_STOP; break; // STOP
          }
        }else if( IN_protocol == SAMSUNG ){
          switch(IN_cmd){
            case 4:   command = 1; if( Mode == SUMO_STOP    ) Mode = SUMO_PREPARE; break; // PREPARE
            case 5:   command = 2; if( Mode == SUMO_PREPARE ) Mode = SUMO_START; break; // START
            case 6:   command = 3; Mode = SUMO_STOP; break; // STOP
            case 8:   command = 4; break;
            case 9:   command = 5; break;
            case 10:  command = 6; break;
            case 12:  command = 7; break;
            case 13:  command = 8; break;
            case 14:  command = 9; break;
            case 17:  command = 0; break;
            case 96:  command = 10; break; // Up
            case 97:  command = 11; break; // Down
            case 98:  command = 12; break; // Rigth
            case 101: command = 13; break; // Left
            case 104: command = 14; break; // OK
            case 108: command = 15; break; // A
            case 20:  command = 16; break; // B
            case 21:  command = 17; break; // C
            case 22:  command = 18; break; // D
            case 224: command = 19; break; // STB
            case 2  : command = 20; break; // ON
          }
        }
        Change = (Mode_before != Mode);
        logif();
      }
      updateLed();
      return command;
    }

    // return protocol
    decode_type_t protocol( ){ return IN_protocol; }
    String protocol_str( ){ return protocol_str( IN_protocol ); }
    String protocol_str( decode_type_t p ){
      switch ( p ){
        case NEC:       return "NEC"      ; break;
        case SONY:      return "SONY"     ; break;
        case RC5:       return "RC5"      ; break;
        case RC6:       return "RC6"      ; break;
        case SHARP:     return "SHARP"    ; break;
        case JVC:       return "JVC"      ; break;
        case SAMSUNG:   return "SAMSUNG"  ; break;
        case LG:        return "LG"       ; break;
        case WHYNTER:   return "WHYNTER"  ; break;
        case PANASONIC: return "PANASONIC"; break;
        case DENON:     return "DENON"    ; break;
        //case DISH:         Serial.println("DISH");         break;
        //case SANYO:        Serial.println("SANYO");        break;
        //case MITSUBISHI:   Serial.println("MITSUBISHI");   break;
        //case AIWA_RC_T501: Serial.println("AIWA_RC_T501"); break;
      }
      return  "UNKNOWN";
    }

    // Informations
    // Enable or disable debug logging
    void debug(bool debug_on){ DEBUG = debug_on; };
    bool debug(){ return DEBUG; };
    void logif(){
        if( DEBUG ) log();
    }
    void log(){
        Serial.println( str() );
    }

    String str(){
      char buf[100];
      sprintf( buf, ">> [%s] IR CMD: %d [ 0x%.8X ] [ %s ]\n", mode_str(), command, IN_data, protocol_str().c_str() );
      return buf;
    }

    // Sumo modes    
    String mode_str(){
      switch ( Mode ){
        case SUMO_START:   return "START";   break;
        case SUMO_STOP:    return "STOP";    break;
        case SUMO_PREPARE: return "PREPARE"; break;
      }
      return "UNKNOWN";
    }

    int mode(){ return Mode; }
    void setMode( int mode ){ if( mode >= SUMO_STOP && mode <= SUMO_START )  Mode = mode; }

    bool change(){ return Change; }
    bool start(){ return (Mode == SUMO_START) && Change; }
    bool stop(){ return (Mode == SUMO_STOP) && Change; }
    bool on(){ return Mode == SUMO_START; }
    bool off(){ return Mode == SUMO_STOP; }
    bool prepare(){ return Mode == SUMO_PREPARE; }

};

#endif