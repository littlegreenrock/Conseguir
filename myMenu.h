
/*
 * naviFramework.h
 *
 *  Created on: 22 May 2023
 *      Author: Jade
 */

#ifndef naviFramework_H_
#define naviFramework_H_
#pragma once
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper /*dumb*/*>(pstr_pointer))
//#include "safeMan.h"
#include "HookEE.h"
#include "MenuData.h"
#include "Arduino.h"

//#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;
extern tally* Tally;
extern void singer(navTEXT);
extern const bool _CoolEffectsBra;
extern nav navD;
const char c_nt = '\0';

class lcdADF {
private:
	void _push(uint8_t j, uint8_t cmd) {
		while(j-- >0) lcd.command(cmd);
	}
	void _printPage(char* buffer16, uint8_t row, bool side=0){
		lcd.noDisplay();
		lcd.setCursor(side?15:0,min(1,row));
		CrsRgt(side?5:0);
		lcd.print(buffer16);
	}
public:
  void done(){
    lcd.home();
    lcd.display();
  }
  void home2(){
		lcd.setCursor(15,0);
		CrsRgt(5);
  }
	void scrollLeft(int8_t i=1) {	//lcd.scrollDisplayLeft
		_push(i,0x18);	//	PgRt
	}
	void scrollRight(int8_t i=1) {  //lcd.scrollDisplayRight
		_push(i,0x1c);	//	PgLt
	}
	void CrsLft(int8_t i=1) {
		_push(i,0x10);	//	[<-]
	}
	void CrsRgt(int8_t i=1){
		_push(i,0x14);	//	[->]
	}
	void printLHS(char* buffer16, uint8_t row) {
			_printPage(buffer16, row, false);			
	}
	void printRHS(char* buffer16, uint8_t row) {
			_printPage(buffer16, row, true);			
	}
	void fxDspl_CursPushLeft(){	_push(1,0x04);	}	// These  lcd.rightToLeft
	void fxDspl_CursPushRght(){	_push(1,0x06);	}	// are 4  lcd.leftToRight
	void fxCurs_DsplDragRght(){	_push(1,0x05);	}	// typing lcd. ~ ? ~
	void fxCurs_DsplDragLeft(){	_push(1,0x07);	} // modes. lcd.autoscroll
  // lcd.autoscroll ('rtjstfy') - except it's not, it only drags the display)
  // to 'print' right-jstfy, cursor start at 15, CursPushLeft, print characters
  // in reverse order. But this is just a R-L hack.  They all hacks. 
        //lcd.noAutoscroll might be one of these
};	//	END class lcdADF


class nibble {
public:
  uint8_t highNib(uint8_t val) {
    return val>>4;
  }
  uint8_t lowNib(uint8_t val) {
    return val&0x7;
  }
  uint8_t combineNib(uint8_t hiNib, uint8_t loNib) {
    return (hiNib<<4)+(loNib&0x7);
  } 
};  //  END class nibble


class thedynamicfeedbackthing : public nibble, public code_converter, public lcdADF { 
public: // 'structors 
  thedynamicfeedbackthing(uint8_t Size, uint8_t col, uint8_t row) 
    : _Length{(uint8_t)(Size + 1)}, _dynBuffer{new char[_Length]}, _CursorCoPos{combineNib(row,col)}
  { };
  thedynamicfeedbackthing(uint8_t Size, uint8_t col, uint8_t row, char asterix, bool b_pref) 
    : _Length{(uint8_t)(Size + 1)}, _dynBuffer{new char[_Length]}, _CursorCoPos{combineNib(row,col)}, Burst{b_pref}
  {
    if(asterix) _privacyChar = asterix;
  };
  ~thedynamicfeedbackthing() {
    delete _dynBuffer;
    _dynBuffer = nullptr;
  };

private: 
  const uint8_t _Length;
  char _privacyChar=c_nt; // char_ null-terminate
  bool Burst=true;  // when buffer is full: burst, or stop accepting more char.
  uint8_t _count=0;
  char* _dynBuffer;
  uint8_t _CursorCoPos; // row,col

  void _termiNull(uint8_t point){
    _dynBuffer[point] = c_nt;
  }

  void _customMemset(char Symbol, uint8_t Size, uint8_t _delay_ms=1) {
    memset(_dynBuffer, Symbol, Size);
    _termiNull(Size);
    if (isPrintable(Symbol) && Size>0) {
      _displayChar(0);
      delay(_delay_ms);
    }
  }

  void _readyCursor(uint8_t plusMore=0) {
    lcd.setCursor(highNib(_CursorCoPos), lowNib(_CursorCoPos)+plusMore);
    // hide or present input cursor for the human.
    (_count>0||plusMore>0||_privacyChar)?lcd.cursor():lcd.noCursor();
  }

  void _displayChar(char p) {
    _readyCursor();
    uint8_t i;
    for (i = 0; i < _count - 1; i++) lcd.print(p?_privacyChar:_dynBuffer[i]);
    lcd.print(_dynBuffer[i]);
  }

public:
  void add(const char Ch) {
    _dynBuffer[_count++] = Ch;
    _termiNull(_count);
    if (_count>=_Length) burst();
    else _displayChar(_privacyChar);
  }
      
  void burst(){ 
      singer(Cancelled);
      clrFeedback(_CoolEffectsBra);
      //lcdADF::CrsLft();

  }
  void clrFeedback(bool snazzy=0) {
    if (_CoolEffectsBra||snazzy) {
      _customMemset('-', _count, 150);
      _customMemset('.', _count, 150);
    }
    _customMemset(' ', _count);
    _customMemset(c_nt, _Length);
    _count = 0;
    _readyCursor();
  }
  bool getLen(uint8_t compareLength) { 
    return (_count>=compareLength);
  }
  uint8_t getLen() {
    return _count;
  }
  uint8_t Obtain(char* destBuffer){
    //  returns entered keys as string & length
    //  caller can use length to nt an abundant buffer
    strcpy(destBuffer, _dynBuffer);
    lcd.noCursor();
    return _count;
  }
  int Obtain() {  
    //  returns entered keys as an integer
    lcd.noCursor();
    return atoi(_dynBuffer);
  }
  uint32_t Obtain_as_someHxCode(){
    return (code_converter::codeToUint32_t(_dynBuffer, _count));
  }
};  //  END class the dynamicfeedbackthing


class ppp : public lcdADF {   //   working fine
private:
  void PPws(int8_t cnt, char WS=' ') {
    while (cnt-- > 0) (lcd.print(WS));
  }
public:
  void progProgPrint(const char* str, int8_t padders, justification J)
  {
    PPws(padders);
    char C;
    padders= 16 - max(0,padders); 
    while (C = pgm_read_byte(str++)) {
      padders-= lcd.print(C);
      if(padders<=0) break; //defence
    } // 'none' will not ws overwrite after label
    if (padders>0 && J!=none) PPws(padders);
  }
};  //  END class ppp


class labelStrFetcher : public nibble, public ppp { 
public:
  void Display(navTEXT requestLabel, bool RHS=false, justification J=none) {
    if (RHS) lcdADF::home2();
    else lcd.home();
    const char* str = _getStrPtr(requestLabel);
    int8_t padders = _getPrePadding(requestLabel, J);
    ppp::progProgPrint(str, padders, J);  // <-- actual lcd progressive PROGMEM print command
  }
  uint8_t getThatExtraValue(navTEXT requestLabel) {
    return (uint8_t)pgm_read_byte(&mLabelGroup[highNib((byte)requestLabel)][lowNib((byte)requestLabel)].value);
  }
private:
  const char* _getStrPtr(navTEXT requestLabel) {
    return &(mLabelGroup[highNib((byte)requestLabel)][lowNib((byte)requestLabel)].label[0]);
  }
  int8_t _getPrePadding(navTEXT requestLabel, justification &J) {
    if (J == none) J = _autoJ(requestLabel);
    int8_t whiteSpace = min(16, max(0, 16 - _getLennyP(requestLabel)));
    if (J == right) return whiteSpace;
    else if (J == center) return whiteSpace / 2;
    else return 0;
  }
  uint8_t _getLennyP(navTEXT requestLabel) {
    return (uint8_t)strlen_P(mLabelGroup[highNib((byte)requestLabel)] [lowNib((byte)requestLabel)].label);
  }
  justification _autoJ(navTEXT requestLabel) {
    justification J = (justification)((uint8_t)(getThatExtraValue(requestLabel))>>6);
    if (J == left || J == right || J == center) return J;
    return justification::none;
  }
};  //  END class labelStrFetcher


class menuElement : public labelStrFetcher {
public:  // 'structors
  menuElement(navTEXT myLabel, void (*action)())
    : myLabel{ myLabel }, action{ action } {
    Banner3();
  };
  menuElement(navTEXT myLabel)
    : myLabel{ myLabel } {
      _lookForMyActionPtr(myLabel);
      Banner3();
  };
  ~menuElement() {
    action = nullptr;
  };

private:
  navTEXT myLabel;
  void (*action)(); // function-ptr
  void _lookForMyActionPtr(navTEXT l) {
    action = doAdminActions[labelStrFetcher::getThatExtraValue(l)&0x3f];
  }

public:
    void Banner3() {
      labelStrFetcher::Display(myLabel, false);
    }
    void addAction(void (*action)()) {   //  TODO - what is my purpose?
    }
    bool executeAction() {
      if (action == nullptr) {
        Serial.print(myLabel,HEX);
        Serial.print(": has no action!");
        return false;
      }
      action();
      return true;
    }
};  //  END class menuElement


class naviFramework : public nibble {
public:  // 'structors
  naviFramework(navTEXT label) : myLabel{label} {
    _generateNewMenuItem(label); // <-- starting point
  };
  ~naviFramework() {
    delete Item;
    Item = nullptr;
  };
private:
  navTEXT myLabel;
  navTEXT _returnMenu;
  //int8_t idx;       // 0 idx
  //int8_t Tier;      // 1 indexed, Tier 0 is kiosk and misc labels
  bool unlimitedScroll = false;   //    TODO : get from settings
  menuElement* Item {nullptr};

  int8_t _idx()  { return lowNib(myLabel);  }
  int8_t _tier() { return highNib(myLabel); }
  
  void _generateNewMenuItem(navTEXT new_label) {
    myLabel = new_label;
    //idx = lowNib(myLabel);
    //Tier = highNib(myLabel);
    delete Item;
    Item = new menuElement(myLabel);
    //banner();
  }

public:
  int8_t switchTier(navTEXT label){
    _returnMenu = myLabel;
    _generateNewMenuItem(label);
  }

  int8_t returnTier(){
    if (highNib((uint8_t)(myLabel))==highNib((uint8_t)_returnMenu)) _returnMenu = L[highNib(myLabel)][0];
    else myLabel = _returnMenu;
    _generateNewMenuItem(_returnMenu);
  }

  bool browse(nav Dir) {
    int8_t idx = _idx();
    int8_t tier = _tier();
    dln(Dir, idx, tier);
    if (Dir==Prev||Dir==Next){
      idx += Dir;
      if (unlimitedScroll) {
        idx = (idx + menuMapSizes[tier]) % (menuMapSizes[tier]);
      }else {
        if (idx<0) idx = 0;
        if (idx>=menuMapSizes[tier]) idx = menuMapSizes[tier] -1;
      }
      _generateNewMenuItem((L[tier][idx]));
      return false;
    }
    else if (Dir==Selct) {
      trigger();
      return true;
    }
    else if (Dir==Esc) {
      if (_returnMenu!=myLabel) returnTier();
      else if (idx!=0) _generateNewMenuItem((L[tier][0]));
    }
      return false;
  }  
  void banner() { //  simply an abstracted request to print the current item label
    Item->Banner3();
  }
  void addItem(navTEXT label, void (*action)()) { //  TODO
  }  
  void addItemAct(void (*action)()) {
    Item->addAction(*action); // function-ptr
  } 
  void trigger(){
    Item->executeAction();
  }
  navTEXT gateLabel() {
    return myLabel;
  }
};     // END class naviFramework

class special {
public:
  special () {
    lcd.blink();
    delay(500);
    lcd.display();
  }
  bool main(uint32_t sinisterCode) {
    //return Settings.compareAdminCode(sinisterCode);
    return true;

  }
}; //  END class special


class kioskmenu : public menuElement, public thedynamicfeedbackthing {
public: // 'structors  
  kioskmenu(uint8_t minCodeLength, uint8_t inputBuffSz, uint8_t cursCol, uint8_t cursRow) :
    menuElement{EnterCode}, _minCodeLength{minCodeLength},
    thedynamicfeedbackthing{inputBuffSz, cursCol, cursRow}
    {
      _reset();
    };
  ~kioskmenu(){
    _disposal();
  };

private:
  uint8_t _minCodeLength;
  orphanRec* Orphan;
  tally* Tally;
  special* Feature{nullptr};

  void _disposal(bool Reset=false){
    lcd.noDisplay();
    delete Orphan;
    Orphan = nullptr;
    delete Tally;
    Tally = nullptr;
    delete Feature;
    Feature = nullptr;
    if(Reset) _reset();
  }

  void _reset() {
    menuElement::Banner3();
    thedynamicfeedbackthing::clrFeedback();
    Tally = new tally;
    delay(150);
    lcd.display();
    lcd.noBlink();
  }

  bool _probablyWrongAnyway(uint8_t Len) {
    uint8_t matchingAddress;
    if (Len >=_minCodeLength) {
      //if (Tally==nullptr) Tally = new tally;
      //char digits[Len+1];
      //Obtain(digits);
      //Orphan = new orphanRec(digits,Len);
      if (_specialK(nav::Selct, thedynamicfeedbackthing::Obtain_as_someHxCode())) {
        Orphan = new orphanRec(thedynamicfeedbackthing::Obtain_as_someHxCode());
        matchingAddress = Tally->Adoption(Orphan);
      }
      thedynamicfeedbackthing::clrFeedback();
      if (matchingAddress) _Fanfare(75);
      _disposal(true);
    }
  }

  void _Fanfare(int Dur) {
    menuElement Result(Success);
    singer(Success);
    Idle.setPause(2000);
  }

  bool _specialK(nav Dir, uint32_t sinister) {
    if (Dir==LhAS && !Feature && sinister==0) Feature = new special;
    if (Dir==Esc && sinister==0) _disposal(true);
    if (Dir==Selct) return (Feature->main(sinister));
  }
  
public:
  bool main(char kP) {
    if (kP=='!' || Tally==nullptr) _reset();
    if (kP=='x') _specialK(nav::LhAS, getLen());
    if (isDigit(kP)) add(kP);
    else if (kP=='*') thedynamicfeedbackthing::clrFeedback(); 
    else if (kP=='#') return _probablyWrongAnyway(getLen());
  }
};  //  END class Kioskmenu


class adminmenu : public naviFramework {
public: //  'structors  
  adminmenu() : naviFramework{SetCode} {
    lcd.display();
  };
  ~adminmenu() {
     _disposal();
  };
private:
  labelStrFetcher* additionalText = nullptr;
  hookRec* myRec = nullptr;
  bool jesusHasTheWheel = false;

  void _disposal(){
    lcd.noDisplay();
    delete additionalText;
    delete myRec;
    additionalText = nullptr;
    myRec = nullptr;
   }

public:
  void main(char kP, nav navD){
    if (jesusHasTheWheel==false) navigate(navD);
    else {  dln(":(");  
    }
  }

  void navigate(nav navD) {
    if (navD==Clr) navD=Esc;
    if (naviFramework::browse(navD)) 0;//jesusHasTheWheel=true;
    else if (navD==Esc) naviFramework::returnTier();
  }
  void drillDown(navTEXT nextLevel){
    jesusHasTheWheel=false;
    switchTier(nextLevel);
  }

};    //  END class adminmenu




class submenu : public menuElement {
public: // 'structors  
  submenu(navTEXT myLabel , uint8_t minCodeLength, uint8_t inputBuffSz) 
    : menuElement{myLabel}, _minCodeLength{minCodeLength}
    { 
      _reset();
    };
  ~submenu(){
    _disposal();
  };
/*
  "Which Hook?" : * cancel back to menu
  (user entry)  : * cls entry, back to menu
  [pull record, check status, display status summary]
  "Enter Code:" : * displays "Cancelled", paws, back to menu.
  (user entry)  : * cls entry, as prev.
  [set record, DB check, display warnings]
  -done-
 */
/*
  SetCode: hook? 2 code? done
  ShowCode: hook/All? 2 done
                "<->" nav Esc done
  InstallKey: hook? 2 done
  ClearCode: hook/All? 2 done
                "sure?" conf done
  DropKey: hook/all?  2 done
                "ready?" conf done

  SetMaxKeys: UI Selct/Esc done
  KP-BL: UI S/E done
  LCD-BL:
  Beep: UI S/E done
  SU: SUcode? setPriv subMenu done

 */
private: 
  uint8_t _minCodeLength;
  uint8_t settings_maxKs;
  hookRec* keyRecord;
  thedynamicfeedbackthing* fbKeypad;
  tally* Tally;
  labelStrFetcher* Label2ndy;

  void _disposal(){
    lcd.noDisplay();
    delete keyRecord;
    delete fbKeypad;
    delete Tally;
    delete Label2ndy;
    fbKeypad = nullptr;
    keyRecord = nullptr;
    Tally = nullptr;
    Label2ndy = nullptr;
  }

  void _reset() {
    menuElement::Banner3();
    fbKeypad = new thedynamicfeedbackthing(2, 12, 1);
    Tally = new tally;
    lcd.display();
  }

public: 
  void main(char kP) {
    if (kP=='!') _reset();
    if (isDigit(kP)) fbKeypad->add(kP);
    else if (kP=='*') fbKeypad->clrFeedback(); 
    else if (kP=='#') fbKeypad->Obtain();
  }

  void menuAction_enterHook(char kP) { 
    if (fbKeypad = nullptr) fbKeypad = new thedynamicfeedbackthing(2, 12, 1);
    uint8_t x = (uint8_t)fbKeypad->Obtain();
    uint8_t Len = fbKeypad->getLen();
    if (kP=='!') _reset();
    if (isDigit(kP)) fbKeypad->add(kP);
    else if (kP=='*') fbKeypad->clrFeedback(); 
    else if (kP=='#' && x > 0) {
      if (x<=settings_maxKs && x>0) {
        keyRecord = new hookRec(fbKeypad->Obtain());
        fbKeypad->clrFeedback();
      }
    }
  }

  void menuAction_enterCode(char kP) {
    if (fbKeypad = nullptr) fbKeypad = new thedynamicfeedbackthing(9,4,1);
    uint8_t Len = fbKeypad->getLen();
    if (kP=='!') _reset();
    if (isDigit(kP)) fbKeypad->add(kP);
    else if (kP=='*') fbKeypad->clrFeedback(); 
    else if (kP=='#' && Len > _minCodeLength) {
      char digits[Len+1];
      fbKeypad->Obtain(digits);
      keyRecord->setCode(digits, Len);
      fbKeypad->clrFeedback();
      }
    }
  
  void menuAction_installKey() {
    keyRecord->install();
    //adminRec->closeRecord();
  }

};  //  END class submenu 


#endif   // naviFramework_H_
