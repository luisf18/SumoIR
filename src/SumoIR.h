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

enum{
  SUMO_STOP = 0,
  SUMO_PREPARE,
  SUMO_START
}

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
    uint16_t LED_dt;

    bool Change = false;

  public:

    IRrecv IR_IN = IRrecv(Pin); // Receptor
    boolean availlable = false;
    int command = -1;

    // Begin
    void begin(){ IR_IN.enableIRIn(); }
    void begin(uint8_t pin){ if(Pin!=pin){ Pin = pin; IR_IN.setReceivePin(Pin); } IR_IN.enableIRIn(); }

    void setLed( int pin, bool state_on, uint16_t dt ){
      LED = pin;
      LED_state_on = state_on;
      LED_dt = dt;
      if(LED >= 0){
        pinMode( Pin, OUTPUT );
        digitalWrite( Pin, !state_on );
      }
    }

    void updateLed(){
      if( LED < 0 ) return;
      if( Mode == SUMO_STOP ){
        digitalWrite(Pin,!state_on);
      }else if( Mode == SUMO_START ){
        digitalWrite(LED,LED_state_on);
      }else if( Mode == SUMO_PREPARE ){
        if(Change){ 
          digitalWrite(LED,LED_state_on);
        }else if( millis() >= LED_timeout ){
          LED_timeout = millis() + dt;
          digitalWrite(LED,!digitalRead(LED));
        }
      }
    }

    ////////////////////////////// IR INPUT /////////////////////////////////////
    int update() {
      
      availlable = false;
      command = -1;
      Change = false;

      if( IR_IN.decode() ){
        IR_IN.resume();
        
        IN_cmd      = IR_IN.decodedIRData.command;
        IN_protocol = IR_IN.decodedIRData.protocol;
        IN_data     = IR_IN.decodedIRData.decodedRawData;
        
        availlable = true;
        // Print command
        // CMD.log(">> IR CMD: %d [ 0x%.8X ] [ ",cmd,data); IR_print_protocol( protocol ); CMD.log(" ]\n");
        Mode_before = Mode;
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
          //if( sound ) cb_click();
        }
      }
      Change = (Mode_before != Mode);
      updateLed();
      return command;
    }

    // return protocol
    decode_type_t protocol( decode_type_t p ){ return IN_protocol; }
    void protocol_str( char *str ){ protocol_str( str, IN_protocol ); }
    void protocol_str( char *str, decode_type_t p ){
      switch ( p ){
        case NEC:       strcpy( str, "NEC"        ); break;
        case SONY:      strcpy( str, "SONY"       ); break;
        case RC5:       strcpy( str, "RC5"        ); break;
        case RC6:       strcpy( str, "RC6"        ); break;
        case SHARP:     strcpy( str, "SHARP"      ); break;
        case JVC:       strcpy( str, "JVC"        ); break;
        case SAMSUNG:   strcpy( str, "SAMSUNG"    ); break;
        case LG:        strcpy( str, "LG"         ); break;
        case WHYNTER:   strcpy( str, "WHYNTER"    ); break;
        case PANASONIC: strcpy( str, "PANASONIC"  ); break;
        case DENON:     strcpy( str, "DENON"      ); break;
        //case DISH: Serial.println("DISH"); break ;
        //case SANYO: Serial.println("SANYO"); break ;
        //case MITSUBISHI: Serial.println("MITSUBISHI"); break ;
        //case AIWA_RC_T501: Serial.println("AIWA_RC_T501"); break ;
        default:        strcpy( str, "UNKNOWN"    ); break;
      }
    }

    // Informations
    void log(){
      char p_buf[15];
      protocol_str(p_buf);
      char buf[50];
      sprintf( buf, ">> IR CMD: %d [ 0x%.8X ] [ %s ]", command, IN_data, p_buf );
      Serial.println(buf);
    }
    
    int mode(){
      return Mode;
    }

    bool change(){ return Mode == SUMO_START; }
    bool start(){ return (Mode == SUMO_START) && Change; }
    bool stop(){ return (Mode == SUMO_STOP) && Change; }
    bool on(){ return Mode == SUMO_START; }
    bool off(){ return Mode == SUMO_STOP; }
    bool prepare(){ return Mode == SUMO_PREPARE; }
    
    // void str(){
    //   char buf[100];
    //   //snprintf( buf, 100, ">> IR CMD: %d [ 0x%.8X ] [ %s ]",command,IN_data, protocol_str().c_str() );
    //   //String txt(buf);
    //   //Serial.println(txt);
    //   //return txt;
    // }

    //void cb_click();

};

#endif