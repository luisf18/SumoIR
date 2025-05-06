#include "IR_decoder.h"

// Instância global do decodificador
IR_decoder IR_IN;

void IRAM_ATTR IR_decoder_interrupt() {
  IR_IN.decoder();
}

void IR_decoder::begin(int pin) {
  Pin = pin;
  pinMode(pin, INPUT_PULLUP);
  resume();
}

bool IR_decoder::update(bool Resume) {
  Recived = false;
  if (NewCode) {
    BitSize = bitCount;
    WasSamsung = IsSamsung;
    // Inverte a ordem
    Code = 0;
    for (int i = 0; i < bitCount; i++) {
      Code = (Code << 1) | (IrCode & 1);
      IrCode >>= 1;
    }

    // Data - para a função Read
    Data = -1;
    if (IsSamsung) {
      switch (command()) {
        case 4: Data = 1; break; // PREPARE
        case 5: Data = 2; break; // START
        case 6: Data = 3; break; // STOP
        case 8: Data = 4; break;
        case 9:   Data = 5; break;
        case 10:  Data = 6; break;
        case 12:  Data = 7; break;
        case 13:  Data = 8; break;
        case 14:  Data = 9; break;
        case 17:  Data = 0; break;
        case 96:  Data = 10; break; // Up
        case 97:  Data = 11; break; // Down
        case 98:  Data = 12; break; // Right
        case 101: Data = 13; break; // Left
        case 104: Data = 14; break; // OK
        case 108: Data = 15; break; // A
        case 20:  Data = 16; break; // B
        case 21:  Data = 17; break; // C
        case 22:  Data = 18; break; // D
        case 224: Data = 19; break; // STB
        case 2:   Data = 20; break; // ON
      }
    } else {
      switch (command()) {
        case 0: Data = 1; break; // PREPARE
        case 1: Data = 2; break; // START
        case 2: Data = 3; break; // STOP
        case 128: Data = 1; break; // PREPARE
        case 129: Data = 2; break; // START
        case 130: Data = 3; break; // STOP
      }
    }
    Recived = true;
    if (Resume) resume();
    return true;
  }
  return false;
}

void IR_decoder::resume() {
  NewCode = false;
  Header = true;
  IrCode = 0;
  bitCount = 0;
  Header_high_ok = false;
  attachInterrupt(digitalPinToInterrupt(Pin), IR_decoder_interrupt, CHANGE);
}

bool IR_decoder::recive() {
  return Recived;
}

bool IR_decoder::available() {
  return Recived;
}

bool IR_decoder::isSamsung() {
  return WasSamsung;
}

uint32_t IR_decoder::read() {
  return Data;
}

uint32_t IR_decoder::readRaw() {
  return Code;
}

uint32_t IR_decoder::device() {
  return (WasSamsung ? 0 : Code >> 7);
}

uint32_t IR_decoder::command() {
  return (WasSamsung ? (Code >> 16) & 0xFF : Code & 0xEF );
}

uint32_t IR_decoder::size() {
  return BitSize;
}

String IR_decoder::string() {
  char buf[100];
  sprintf(buf, ">> IR CMD[%d]: %d [ 0x%.8X ] [ command: %d ] [ %s ]\n", BitSize, Data, Code, command(), WasSamsung ? "SAMSUNG" : "SONY");
  return buf;
}

void IR_decoder::decoder() {
  uint32_t currentTime = micros();
  uint32_t duration = currentTime - lastTime;
  lastTime = currentTime;

  bool IR_HIGH = digitalRead(Pin);

  // HEADER -----------------------------------------------------------------------
  if (Header) {
    if (IR_HIGH) {
      // Samsung header (4.5ms)
      if (IR_CHECK_DURATION(4500)) {
        Header_high_ok = true;
        IrCode = 0;
        bitCount = 0;
        IsSamsung = true;
      }
      // Sony header (2.4ms)
      else if (IR_CHECK_DURATION(2400)) {
        Header_high_ok = true;
        IrCode = 0;
        bitCount = 0;
        IsSamsung = false;
        Header = false;
      }
    } else if (Header_high_ok) {
      if (IsSamsung) {
        // Check low Samsung header (4.5ms)
        if (IR_CHECK_DURATION_2(4500, 500)) {
          Header = false;
        } else {
          IR_END_READ_ERRO_RETURN();
        }
      }
    }
  } else {
    // CODE -----------------------------------------------------------------------
    if (IsSamsung) {
      // [ SAMSUNG HIGH ] --------------------------------------------------------------
      if (IR_HIGH) {
        // (560µs)
        if (!IR_CHECK_DURATION(560)) {
          IR_END_READ_ERRO_RETURN();
        }
      }
      // [ SAMSUNG LOW ] --------------------------------------------------------------
      else {
        bitCount++;
        // Samsung '1' (1690µs) --------------------------
        if (IR_CHECK_DURATION(1690)) { IR_NEW_BIT_1(); }
        // Samsung '0' (560µs) ---------------------------
        else if (IR_CHECK_DURATION(560)) { IR_NEW_BIT_0(); }
        // erro ------------------------------------------
        else { IR_END_READ_ERRO_RETURN(); }
        // Check if all bits are received
        if (bitCount >= 32) {
          IR_END_READ_RETURN();
        }
      }
    } 
    // [ SONY bits ] --------------------------------------------------------------
    else {
      // [ SONY HIGH ] --------------------------------------------------------------
      if (IR_HIGH) {
        bitCount++;
        // Sony '1' (1200µs) --------------------------
        if (IR_CHECK_DURATION(1200)) { IR_NEW_BIT_1(); }
        // Sony '0' (600µs) ------------------------------
        else if (IR_CHECK_DURATION(600)) { IR_NEW_BIT_0(); }
        // erro ------------------------------------------
        else { IR_END_READ_ERRO_RETURN(); }
        // Check if all bits are received
        if (bitCount >= 12) {
          IR_END_READ_RETURN();
        }
      }
      // [ SONY LOW ] --------------------------------------------------------------
      else {
        if (!IR_CHECK_DURATION(600)) {
          IR_END_READ_ERRO_RETURN();
        }
      }
    }
  }
}
