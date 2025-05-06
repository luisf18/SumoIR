#ifndef IR_DECODER_H
#define IR_DECODER_H

#include <Arduino.h>

void IRAM_ATTR IR_decoder_interrupt();

class IR_decoder {
  // Variáveis
  volatile uint32_t IrCode = 0;
  volatile bool NewCode = false;
  volatile unsigned long lastTime = 0;
  volatile int bitCount = 0;
  volatile bool IsSamsung = false;

  uint8_t Pin = 255;

  // Último código recebido
  uint32_t BitSize = false;
  bool Recived = false;
  bool WasSamsung = false;
  uint32_t Code = 0;

  volatile bool Header = true;
  volatile bool Header_high_ok = false;

  int Data = 0;

public:
  void begin(int pin);
  bool update(bool Resume = true);
  void resume();
  bool recive();
  bool isSamsung();
  bool available();
  uint32_t read();
  uint32_t readRaw();
  uint32_t device();
  uint32_t command();
  uint32_t size();
  String string();
  
  // Macro do decodificador
  #define IR_END_READ_RETURN() \
    NewCode = true; \
    detachInterrupt(digitalPinToInterrupt(Pin)); \
    return

  #define IR_END_READ_ERRO_RETURN() Header = true; Header_high_ok = false; return

  // Novos bits
  #define IR_NEW_BIT_1() IrCode = (IrCode << 1) | 1;
  #define IR_NEW_BIT_0() IrCode = (IrCode << 1);
    
  // Verificação de duração
  #define IR_CHECK_DURATION_2(t, e) (duration > (t - e) && duration < (t + e))
  #define IR_CHECK_DURATION(t) IR_CHECK_DURATION_2(t, 300) // tolerância de + ou - 300ms

  // Decodificador
  void decoder();
};

extern IR_decoder IR_IN;

#endif // IR_DECODER_H
