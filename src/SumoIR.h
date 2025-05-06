/* ====================================================
 *  SumoIR Library
 *    Author: luisf18 (github)
 *    Ver.: 0.1.0
 *      
 *      Processa os sinais de IR para sumôs.
 *      
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
    unsigned long LED_timeout;
    bool LED_state_on;
    bool LED_state;
    uint16_t LED_dt;

    // other protocols
    bool SAMSUNG_en = false;

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

    // timing
    uint32_t time_ms_at_start = 0;
    uint32_t round_duration_ms = 0;

    IRrecv IR_IN = IRrecv(Pin); // Receptor

    // ---------------------------------------------------------------------
    // Basic functions
    // Begin, available and read
    // ---------------------------------------------------------------------
    void begin(){ begin(Pin); }
    void begin(uint8_t pin){
      round_duration_ms = 0;
      if(Pin!=pin){
        Pin = pin;
        IR_IN.setReceivePin(Pin);
      }
      pinMode(Pin,INPUT_PULLUP);
      IR_IN.enableIRIn();
    }

    void setMode( int mode ){ if( mode >= SUMO_STOP && mode <= SUMO_START )  Mode = mode; }

    // ---------------------------------------------------------------------
    // Readings
    // ---------------------------------------------------------------------
    int  mode(){ return Mode; } // operation mode ( SUMO_STOP = 0, SUMO_PREPARE, SUMO_START )
    bool available(){ return Available; }
    int  read(){ return command; }
    bool change(){ return Change; }
    bool start(){ return (Mode == SUMO_START) && Change; }
    bool stop(){ return (Mode == SUMO_STOP) && Change; }
    bool on(){ return Mode == SUMO_START; }
    bool off(){ return Mode == SUMO_STOP; }
    bool prepare(){ return Mode == SUMO_PREPARE; }

    // ---------------------------------------------------------------------
    // Timing
    // time_since_start and last_round_duration
    // ---------------------------------------------------------------------
    uint32_t time_since_start(){ return (millis() - time_ms_at_start); }
    uint32_t last_round_duration(){ return round_duration_ms; }

    // ---------------------------------------------------------------------
    // Samsung protocol
    // ---------------------------------------------------------------------
    inline void enable_samsung(){ SAMSUNG_en = true; }
    inline void disaable_samsung(){ SAMSUNG_en = false; }
    
    // ---------------------------------------------------------------------
    // Led
    // functions: setLed and updateLed
    // ---------------------------------------------------------------------
    void setLed( int pin, bool state_on, uint16_t dt ){
      LED = pin;
      LED_state_on = state_on;
      LED_dt = dt;
      if(LED >= 0){
        pinMode( LED, OUTPUT );
        digitalWrite( LED, !LED_state_on );
      }
    }

    // ---------------------------------------------------------------------
    // IR Protocol Informations
    // protocol and protocol_str
    // ---------------------------------------------------------------------
    decode_type_t protocol( ){ return IN_protocol; }
    const char * protocol_str( ){ return protocol_str( IN_protocol ); }
    const char * protocol_str( decode_type_t p ){
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

    // ---------------------------------------------------------------------
    // Debug Informations
    // Enable or disable debug logging
    // ---------------------------------------------------------------------
    void debug(bool debug_on){ DEBUG = debug_on; };
    bool debug(){ return DEBUG; };
    void logif(){
        if( DEBUG ) log();
    }
    void log(){
        char buf[100];
        get_log( buf, 100 );
        Serial.println( buf );
    }

    bool get_log( char *buf, uint16_t buf_len ){
      if( !buf ) return 0;
      if( buf_len < 50 ) return 0;
      sprintf(buf, ">> [%s] IR CMD: %d [ 0x%.8lX ] [ %s ]\n", mode_str(), command, IN_data, protocol_str());
      return 1;
    }

    // Sumo modes    
    const char * mode_str(){
      switch ( Mode ){
        case SUMO_START:   return "START";   break;
        case SUMO_STOP:    return "STOP";    break;
        case SUMO_PREPARE: return "PREPARE"; break;
      }
      return "UNKNOWN";
    }

    // ---------------------------------------------------------------------
    // loop
    // update
    // ---------------------------------------------------------------------
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
            default: command = IN_cmd+1; break;
          }
        }else if( IN_protocol == SAMSUNG ){
          if( SAMSUNG_en ){
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
        }

        Change = (Mode_before != Mode);
        
        if( Change ){
          switch(Mode){
            case SUMO_PREPARE: // PREPARE
              LED_state = LED_state_on;
              LED_timeout = millis() + LED_dt;
              break;

            case SUMO_START: // START
              LED_state = LED_state_on;
              time_ms_at_start = millis();
              break;
            
            case SUMO_STOP: // STOP
              LED_state = !LED_state_on;
              if( Mode_before==SUMO_START ){
                round_duration_ms = (millis()-time_ms_at_start);
              }
              break;
          }
          if( LED >= 0 ){
            digitalWrite(LED,LED_state);
          }
        }
        logif();
      }
      
      if( LED >= 0 ){ // LED update
        if( Mode == SUMO_PREPARE ){
          if( millis() >= LED_timeout ){
              LED_timeout = millis() + LED_dt;
              LED_state = !LED_state;
              digitalWrite(LED,LED_state);
          }
        }
      }
      
      return command;
    }

};

#endif