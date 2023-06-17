
class ezcursor {
  private:
    bool _NoEnd;  //  = to loop at ends, or stop at ends
    const int _Lowest;    //  = lowestlowable Lowest Number;  (Const)
    const int _TotalNo;   //  = number Of Entries;        (Const)
    const int _Highest;   //  = lowestlowable Highest number  (Const) (may differ from _TotalNo);
  public:
    ezcursor(const int lowest, const int NofEntries, const int highest, bool scrollPref = true)
      : _Lowest{ lowest }, _TotalNo{ NofEntries }, _Highest{ highest }, _NoEnd{ scrollPref } {};
    ~ezcursor(){};

    int eZCursor(int cursPos, int dir_w_Mag) {
        /*   
          cursPos = position/index/marker
          dM  = direction and magnitude (ie: -1, +1, up, down)
        */
      int newCursPos;
      if (_NoEnd) newCursPos = ((cursPos + dir_w_Mag + _TotalNo - _Lowest) % _TotalNo) + _Lowest;
      else {
        if (cursPos > _Highest) newCursPos = _Highest;
        else if (cursPos < _Lowest) newCursPos = _Lowest;
      }
      return newCursPos;
    }
    void setScrollPref(bool scrollPref) {
      _NoEnd = scrollPref;
    }
};  //    END class ezCursor

int combineBytes(uint8_t highB, uint8_t lowB) {
    return (highB<<8)+lowB;
}

void LoadCustomGlyph(int charMemSlot, Glyph glyphName) {
  uint8_t pixelArray[8];
  memcpy_P(pixelArray, &(CustomGlyphs[glyphName]), 8);
  Serial.write(pixelArray, 8);
  lcd.createChar(charMemSlot, pixelArray);
}

void singer(int note, int duration, int vibCent, int tremHz) {
  /*  
      max 1 semitone.   freq 5-6.5Hz
      6.5 : 1/(6.5*2) * 1000 = 76.9ms
      singer(330, 500, 50, 6.5); // 330hz, 500ms, 50%, 6.5Hz
  */
  int noteHz = pgm_read_word(_Note_Hz440 + note);
  float vibrato10 = (((float)vibCent * 0.03) * noteHz) / 100;
  //float vibrato071 = (((float)vibCent*0.03)*noteHz)/141;
  float tremolo = 83 / (float)tremHz;
  if (tremolo <= 2) tremolo = duration;
  bool flip = 1;
  while (duration > 0) {
    tone(Buzzer_Pin, noteHz + (flip ? vibrato10 : -vibrato10), tremolo);
    duration -= tremolo;
    flip = !flip;
    delay(tremolo);  // using linear, lock-device delay
  }
  noTone(Buzzer_Pin);
  digitalWrite(Buzzer_Pin,LOW);
}
void singer(navTEXT anyRequests7) { 
  int N, D, V, T;
  int Melody[4]{};
  char buffer[5];
  switch (anyRequests7) {
    case navTEXT::Success:
      break;
    case navTEXT::Beep:
      N = 28;
      D = 25;
      V = 30;
      T = 7;
      break;
    case navTEXT::Cancelled:
      N = 22;
      D = 25;
      V = 30;
      T = 5;
      break;
    case navTEXT::NavLR:
      N = 23;
      D = 15;
      V = 20;
      T = 5;
      break;
    case navTEXT::NavUD:
      N = 22;
      D = 15;
      V = 20;
      T = 5;
      break;
    case navTEXT::AreYouSure:
      break;
    case navTEXT::Blank:
      N = 18;
      D = 5;
      V = 20;
      T = 5;
      break;
    case navTEXT::Clear:
      N = 21;
      D = 15;
      V = 20;
      T = 5;
      break;
    default:
      N=0;
      break;
  }  //  ENDS switch
  if (N > 0) singer(N, D, V, T);
}


class bootStrap {
public:
  uint8_t memCell[6];
  
  bool operator==(const bootStrap& compare) { 
    uint8_t i =0;
    int sum =0;
    while (i <6) sum += (memCell[i] - compare.memCell[i++]);
    return (sum==0);
  }
  void initializeEEPROM(uint settingAddr, uint eeAddress=0) {
    if (settingAddr>eeAddress+6 && settingAddr+8 < c_mem_offset_keyblock) {
      memset(memCell, 0xf0, 6);
      memCell[3]=settingAddr^0xff;
      memCell[4]=settingAddr;
      EEPROM.put(eeAddress,memCell);
    }
  }
  uint locateSettings(uint eeAddress) { 
    uint Luke=0xffff;
    int8_t i=0;
    while (eeAddress < c_mem_offset_keyblock) {
      byte MR = EEPROM.read(eeAddress);
      if (MR==EEPROM.read(eeAddress+1)) {
        EEPROM.get(eeAddress,memCell);
        if ((memCell[5]==memCell[2]) && (memCell[4]^0xff == memCell[3])) {
          Luke = memCell[4];
          break;
        }
      }
    }
    return Luke;
  }
};  //  END class bootStrap


class settings {
public: 
  settings(uint Luke) : addrOffset_Settings{EEPROM.read(Luke)} {
    loadSettings();
  };
  ~settings(){
    0;
  };
private:  
  const uint addrOffset_Settings;
  struct par {
    uint8_t cs;             // 0
    uint8_t maxHooks;       // 1
    uint8_t minCodeLength;  // 2
    uint8_t backlight_KP;   // 3
    uint8_t backlight_LCD;  // 4
    uint8_t beepVolume;     // 5
    uint checkSum() {
    return (0xff^(maxHooks +minCodeLength +backlight_KP +backlight_LCD +beepVolume));}
  };
  static par Param;
  uint32_t SU_admin_code;

  // static char 2fa_label[] = "SafeKeyPing"
  //char SU_2fa_key[17];  
  //char user_name[13];

public: 
  bool saveSettings() {
    Param.cs = Param.checkSum();
    EEPROM.put(addrOffset_Settings, Param);
    return (Param.cs == EEPROM.read(addrOffset_Settings));
  }
  bool loadSettings() {
    EEPROM.get(addrOffset_Settings, Param);
    return (Param.cs == Param.checkSum());
  }
  uint8_t getBeep() { return Param.beepVolume; }
  uint8_t getBlKp() { return Param.backlight_KP; }
  uint8_t getBlLcd() { return Param.backlight_LCD; }
  uint8_t getMCL() { return Param.minCodeLength; }
  uint8_t getMxKs() {return Param.maxHooks; }
  bool compareAdminCode(uint32_t attempt) { return (attempt==SU_admin_code); }
  uint8_t setBeep(uint8_t value) {Param.beepVolume= value;}
  uint8_t setBlKp(uint8_t value) {Param.backlight_KP= value;}
  uint8_t setBlLcd(uint8_t value) {Param.backlight_LCD= value;}
  uint8_t setMCL(uint8_t value) {Param.minCodeLength= value;}
  uint8_t setMxKs(uint8_t value) {Param.maxHooks= value;}
  uint32_t setAdminCode(uint32_t newcode) {SU_admin_code= newcode;}
protected:
  /*   
      -  16 bytes for the secret key ie: JBSWY3DPEHPK3PXP
      -  11 B label ie: SafeKeyPing
      -  5B user ie: Fleet.  Let's say 16 + 12 = 28 B (with no nullterm)
      -  a QR code would read as : otpauth://totp/SafeKeyPing:Fleet?secret=JBSWY3DPEHPK3PXP&issuer=SafeKeyPing
      -  more info https://dan.hersam.com/tools/gen-qr-code.php
  */
};  //  END class settings



// debug stuff //
/*
uint8_t _codeToBuffer(uint32_t someHxCode, char* destBuffer10) {
  char dumb[7];
  sprintf(dumb, "%%0%dlu", (someHxCode & 0x0f));
  sprintf(destBuffer10, dumb, (someHxCode >> 4));
  return ((uint8_t)(someHxCode & 0x0f));  // Len
}
void _reportKeyStr(char* buffer20, uint8_t Avail, uint8_t Addr_Hx, uint32_t Code) {
  char digits[11]{};
  _codeToBuffer(Code, digits);
  sprintf(buffer20, "x%2x %8s %1c%1c%1c%1c", Addr_Hx, digits, (Avail % 0x04 ? 'I' : Avail % 0x02 ? 'R'
                                                                                                 : 0)),
    (Avail % 0x01 ? '.' : Avail = 0x0 ? 'i' : 0), (Avail % 0x10 ? 'V' : Avail % 0x40 ? 'B'
                                                                      : Avail % 0x08 ? 'O'
                                                                                     : 0),
    (Avail % 0x20 ? 'L' : 0);
}
void _keyMemDump(int banks = 4) {
  banks = constrain(banks, 1, 4);
  uint8_t Avail, Addr_Hx;
  uint32_t Code;
  char header[24]{};
  sprintf(header, "Key (adr)%8s%4s", "code", "*");
  for (int col = 0; col < banks; col++) {
    Serial.print(header);
    Serial.print(col == (banks - 1) ? "\n" : "\t");
  }
  for (int i = 1; i <= 8; i++) {
    for (int col = 0; col < banks; col++) {
      char printStr[28]{};
      char digits[5]{};
      //int eeAddress = Conv.KtoE((col*8)+i);
      int eeAddress = ((col * 8) + i - 1) * c_objSizeRec + c_mem_offset_keyblock;
      Addr_Hx = EEPROM.read(eeAddress);
      EEPROM.get(eeAddress + 1, Code);
      Avail = EEPROM.read(eeAddress + 5);
      _reportKeyStr(printStr, Avail, Addr_Hx, Code);
      sprintf(digits, " %2d  ", (col * 8 + i));
      Serial.print(digits);
      Serial.print(printStr);
      Serial.print(col == (banks - 1) ? "\n" : "\t");
    }
  }
} */


