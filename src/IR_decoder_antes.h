



class IR_decoder
{
private:
    const int default_pin = 15;
    int Pin;
public:
    IR_decoder( int pin = default_pin ){
        attachInterrupt(digitalPinToInterrupt(IR_PIN), handleIR, CHANGE);
    }
    //~IR_decoder();
};



#define IR_PIN 19

volatile uint32_t irCode = 0;
volatile bool newIrCode = false;
volatile unsigned long lastTime = 0;
volatile int  bitIndex = 0;
volatile bool isSamsung = false;

// END
#define IR_END_READ_RETURN() \
  newIrCode = true; \
  detachInterrupt(digitalPinToInterrupt(IR_PIN));\
  return

#define IR_END_READ_ERRO_RETURN() \
  bitIndex = 0; \
  return

// New Bits
#define IR_NEW_BIT_1() irCode = (irCode << 1) | 1;
#define IR_NEW_BIT_0() irCode = (irCode << 1);
  
// duration checking
#define IR_CHECK_DURATION_2( t, e ) ( duration > (t-e) && duration < (t+e) )
#define IR_CHECK_DURATION( t ) IR_CHECK_DURATION_2( t, 300 )



void IRAM_ATTR handleIR() {

  uint32_t currentTime = micros();
  uint32_t duration = currentTime - lastTime;
  lastTime = currentTime;

  /*/

    ===============================================================================================
    SONY
    ===============================================================================================

    HIGH
    ---------              -------     -------          -------     ------      --------
            | 2400us       | 600 | 600 | 600 |   1200   | 600 | 600 | 600  ....
    LOW     ----------------     -------     ------------     -------
    data     start                0           1                0

    12 bits

    ===============================================================================================
    SAMSUNG...
    ===============================================================================================

    HIGH
    ---------              -------     -------          -------     ------      --------
            | 2400us       | 600 | 600 | 600 |   1200   | 600 | 600 | 600  ....
    LOW     ----------------     -------     ------------     -------
    data     start                0           1                0

    12 bits
    

  /*/

  // RISE ------------------------------------------------------------------
  if( digitalRead(IR_PIN) == HIGH ) {

    // identifica o header
    if( bitIndex == 0 ){
      
      // Samsung header (4.5ms)
      if( IR_CHECK_DURATION(4500) ){
        irCode = 0;
        bitIndex = 1;
        isSamsung = true;
      }


      // Sony header (2.4ms)
      else if ( IR_CHECK_DURATION(2400) ) {
        irCode = 0;
        bitIndex = 1;
        isSamsung = false;
      }

    }else{

      bitIndex++;
      
      // Process Samsung bits -----------------------------------------------------------
      if (isSamsung) {
        // (560µs)
        if( !IR_CHECK_DURATION( 560 ) ){
          IR_END_READ_ERRO_RETURN();
        }
      }
      
      // Process Sony bits --------------------------------------------------------------
      else{
        // Sony '1' (1200µs) --------------------------
        if ( IR_CHECK_DURATION( 1200 ) ){ IR_NEW_BIT_1(); }
        // Sony '0' (600µs) ------------------------------
        else if ( IR_CHECK_DURATION( 600 ) ){ IR_NEW_BIT_0(); }
        // erro ------------------------------------------
        else{
          IR_END_READ_ERRO_RETURN();
        }

        // Check if all bits are received
        if( bitIndex >= 13 ){
          IR_END_READ_RETURN();
        }
      }

    }
  
  // FALL ------------------------------------------------------------------
  }else{
    
    if( bitIndex == 0 ) return;
    
    // HEADER -----------------------------------------------------------------------

    // SAMSUNG --------------------------------------------------------------
    if (isSamsung) {

      if( bitIndex == 1 ){
        // check low Samsung header (4.5ms)
        if( !IR_CHECK_DURATION_2( 4500, 500 ) ){
          digitalWrite(2,HIGH);
          IR_END_READ_ERRO_RETURN();
        }
      }else{

        // Samsung '1' (1690µs) --------------------------
        if( IR_CHECK_DURATION( 1690 ) ){ IR_NEW_BIT_1(); }
        // Samsung '0' (560µs) ---------------------------
        else if ( IR_CHECK_DURATION( 560 ) ){ IR_NEW_BIT_0(); }
        // erro ------------------------------------------
        else{ IR_END_READ_ERRO_RETURN(); }

        // Check if all bits are received
        if( bitIndex >= 33 ){
          IR_END_READ_RETURN();
        }
      }
    }
    
    // SONY ----------------------------------------------------------------
    else{
      if( !IR_CHECK_DURATION( 600 ) ) {
        IR_END_READ_ERRO_RETURN();
      }
    }

  }
}

void setup() {
  pinMode(2,OUTPUT);
  Serial.begin(115200);
  pinMode(IR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IR_PIN), handleIR, CHANGE);
  digitalWrite(2,LOW);
}

int state = false;

void loop() {
  if (newIrCode) {
    if (isSamsung) {
      Serial.print("Samsung IR Code: 0x");
      Serial.println(irCode, HEX);
    } else {
      Serial.printf("Sony IR Code: 0x%X8\tBIN: ", irCode);
      Serial.println(irCode, BIN);
    }
    newIrCode = false;
    bitIndex = 0;
    digitalWrite(2,LOW);
    attachInterrupt(digitalPinToInterrupt(IR_PIN), handleIR, CHANGE);
  }
}