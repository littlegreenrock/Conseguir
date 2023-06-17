
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
		ArrowRight(side?5:0);
		lcd.print(buffer16);
	}
public:
  void done(){
    lcd.home();
    lcd.display();
  }
  void home2(){
		lcd.setCursor(15,0);
		ArrowRight(5);
  }
	void scrollLeft(int8_t i=1) {	
		_push(i,0x18);	//	PgRt
	}
	void scrollRight(int8_t i=1) {
		_push(i,0x1c);	//	PgLt
	}
	void ArrowLeft(int8_t i=1) {
		_push(i,0x10);	//	[<-]
	}
	void ArrowRight(int8_t i=1){
		_push(i,0x14);	//	[->]
	}
	void printLHS(char* buffer16, uint8_t row) {
			_printPage(buffer16, row, false);			
	}
	void printRHS(char* buffer16, uint8_t row) {
			_printPage(buffer16, row, true);			
	}
	void fxDspl_CursPushLeft(){	_push(1,0x04);	}	//	These
	void fxDspl_CursPushRght(){	_push(1,0x06);	}	//	are 4
	void fxCurs_DsplDragLeft(){	_push(1,0x07);	}	//	typing
	void fxCurs_DsplDragRght(){	_push(1,0x05);	}	//	modes.
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


class thedynamicfeedbackthing : public nibble, public converter, public lcdADF { 
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
      //lcdADF::ArrowLeft();

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
    return (converter::codeToUint32_t(_dynBuffer, _count));
  }
};  //  END class the dynamicfeedbackthing


class ppp : public lcdADF {
private:
  void PPws(int8_t cnt, char WS=' ') {
    while (cnt-- > 0) (lcd.print(WS));
  }
public:
  void progProgPrint(const char* str, int8_t padCntr)
  {
    PPws(padCntr);
    char C;
    padCntr= 16 - padCntr; 
    while (C = pgm_read_byte(str++)) {
      padCntr-= lcd.print(C);
      if(padCntr<=0) break;
    }
    if (padCntr>0) PPws(padCntr);
  }
};  //  END class ppp


class labelStrFetcher : public nibble, public ppp {
public:
  void Display(navTEXT requestLabel, bool RHS) {
      if (RHS) lcdADF::home2();
      else lcd.home();
      Display(requestLabel, none);
    }
  void Display(navTEXT requestLabel) 
  {
    Display(requestLabel, none);  
  }
  void Display(navTEXT requestLabel, justification J)
  {
    const char* str = getStrPtr(requestLabel);
    int8_t padCntr = getPrePadding(requestLabel,J);
    ppp::progProgPrint(str, padCntr); // <-- actual lcd progressive PROGMEM print command
  }
private:
  const char* getStrPtr(navTEXT requestLabel) {
    return &(mLabelGroup[highNib((byte)requestLabel)][lowNib((byte)requestLabel)].label[0]);
  }
  int8_t getPrePadding (navTEXT requestLabel, justification override = none) {
    justification J = autoJ(requestLabel);
    if (override!=none) J = override;
    int8_t whiteSpace = min(16,max(0,16-getLennyP(requestLabel)));
    if (J==right) return whiteSpace;
    else if (J==center) return whiteSpace/2;
    else return 0;
  }
  uint8_t getLennyP(navTEXT requestLabel) 
  {  
	  return (uint8_t)strlen_P(mLabelGroup[highNib((byte)requestLabel)] [lowNib((byte)requestLabel)].label);
  }
  justification autoJ(navTEXT requestLabel) 
  {
    justification J = (justification)highNib(mLabelGroup[highNib((byte)requestLabel)] [lowNib((byte)requestLabel)].value);
    if (J==left||J==right||J==center) return J;
    return justification::none;
  }
};  //  END class labelStrFetcher


class menuElement : public labelStrFetcher {
public:  // 'structors
  menuElement(navTEXT Label, void (*action)())
    : Label{ Label }, action{ action } {
    Banner3();
  };
  menuElement(navTEXT Label)
    : Label{ Label } {
    Banner3();
  };
  ~menuElement() {
    action = nullptr;
  };

private:
  navTEXT Label;
  void (*action)(); // function-ptr

public:
    void Banner3() {
      // set cursor NOW before printing the menu label.
      lcd.home();
      Display(Label);
    }
    void Banner4() { Display(Label, true); }

    void addAction(void (*action)()) {   //  TODO - what is my purpose?
    }
    bool executeAction() {
      if (action == nullptr) {
        dln(Label, ": has no action");
        return false;
      }
      dln("action from ", Label);
      action();
      return true;
    }
};  //  END class menuElement


class naviFramework : public nibble {
public:  // 'structors
  naviFramework(navTEXT label) {
    generateNewMenuItem(label); // <-- starting point
  };
  ~naviFramework() {
    delete Item;
    Item = nullptr;
  };
private:
  navTEXT Label;
  navTEXT _prevLabel;
  int8_t idx;       // 0 idx
  int8_t Tier;      // 1 indexed, Tier 0 is kiosk and misc labels
  bool unlimitedScroll = false;   //    TODO : get from settings
  menuElement* Item {nullptr};

  void generateNewMenuItem(navTEXT new_label) {
    Label = new_label;
    idx = lowNib(Label);
    Tier = highNib(Label);
    delete Item;
    Item = new menuElement(Label);
    banner();
  }

public:
  int8_t switchTier(navTEXT label){
    _prevLabel = Label;
    generateNewMenuItem(label);
  }

  int8_t returnTier(){
    int8_t parent = T[Tier][0];
    if (highNib(_prevLabel)==parent) switchTier(_prevLabel);
    else switchTier((navTEXT)(T[parent][1]));
  }

  void browse(nav Dir) {
    idx += Dir;
    if (unlimitedScroll) {
      idx = (idx + menuMapSizes[Tier]) % (menuMapSizes[Tier]);
      }else {
        if (idx<0) idx = 0;
        if (idx>=menuMapSizes[Tier]) idx = menuMapSizes[Tier] -1;
      }
    generateNewMenuItem((navTEXT)(T[Tier][idx+1]));
  }  
  void banner() { //  simply an abstracted request to print the current item label
    Item->Banner3();
  }
  void addItem(navTEXT label, void (*action)()) { //  TODO
  }  
  void addItemAct(void (*action)()) {
    Item->addAction(*action); // function-ptr
   } 
  navTEXT gateLabel() {
    return Label;
  }
};     // END class naviFramework






class kioskmenu : public menuElement, public thedynamicfeedbackthing {
public: // 'structors  
  kioskmenu(uint8_t minCodeLength, uint8_t inputBuffSz, uint8_t cursCol, uint8_t cursRow) :
    menuElement{EnterCode}, _minCodeLength{minCodeLength},
    thedynamicfeedbackthing{inputBuffSz, cursCol, cursRow}
    {
      reset();
    };
  ~kioskmenu(){
    disposal();
  };

private:
  uint8_t _minCodeLength;
  orphanRec* Orphan;
  tally* Tally;

  void disposal(){
    lcd.noDisplay();
    delete Orphan;
    Orphan = nullptr;
    delete Tally;
    Tally = nullptr;
  }

  void reset() {
    menuElement::Banner3();
    thedynamicfeedbackthing::clrFeedback();
    Tally = new tally;
    lcd.display();
  }

  void probablyWrongAnyway(uint8_t Len) {
    if (Len >=_minCodeLength) {
      //if (Tally==nullptr) Tally = new tally;
      char digits[Len+1];
      Obtain(digits);
      Orphan = new orphanRec(digits,Len);
      clrFeedback();
      uint8_t matchingAddress = Tally->Adoption(Orphan);
      if (matchingAddress) Fanfare(75);
      disposal();
    }
  }

  void Fanfare(int Dur) {
    menuElement Result(Success);
    singer(Success);
    delay(1500);
  }

public:
  void main(char kP) {
    if (kP=='!' || Tally==nullptr) reset();
    if (isDigit(kP)) add(kP);
    else if (kP=='*') clrFeedback(); 
    else if (kP=='#') probablyWrongAnyway(getLen());
  }
};  //  END class Kioskmenu



class adminmenu : public naviFramework {
public: //  'structors 
  adminmenu(int foo) : naviFramework{SetCode} {
    naviFramework::banner(); 
    lcd.display();
  };
  ~adminmenu() { _disposal(); };
private:
  labelStrFetcher* additionalText = nullptr;
  hookRec* myRec = nullptr;
  thedynamicfeedbackthing* userEntry;
  bool jesusHasTheWheel = false;

  void _disposal(){
    lcd.noDisplay();
    delete additionalText;
    delete myRec;
    delete userEntry;
    additionalText = nullptr;
    myRec = nullptr;
    userEntry = nullptr;
  }

public:
  void main(char kP){
    if (!jesusHasTheWheel) {
      if (navD==Prev||navD==Next) naviFramework::browse(navD);
      if (navD==Selct)  jesusHasTheWheel=true;
    }
    //if (navD==Esc) jesusHasTheWheel=false;
    if (jesusHasTheWheel) {
      if (navD==Esc) backupOneLevel();
      else if (navD==Selct) {
        if (naviFramework::gateLabel()==DevSetting) drillDown(SetMaxKeys);
        else if (naviFramework::gateLabel()==SuSetting) drillDown(mCodeLen);
        else jesusHasTheWheel=false;
      }
    }
  }

  void drillDown(navTEXT l) {
    jesusHasTheWheel=false;
    naviFramework::switchTier(l);
  }
  void backupOneLevel() {
    jesusHasTheWheel=false;
    naviFramework::returnTier();
  }

};    //  END class adminmenu






class submenu : public menuElement {
public: // 'structors  
  submenu(navTEXT Label , uint8_t minCodeLength, uint8_t inputBuffSz) 
    : menuElement{Label}, _minCodeLength{minCodeLength}
    { 
      reset();
    };
  ~submenu(){
    disposal();
  };

private: 
  uint8_t _minCodeLength;
  uint8_t settings_maxKs;
  hookRec* keyRecord;
  thedynamicfeedbackthing* fbKeypad;
  tally* Tally;
  labelStrFetcher* Label2ndy;

  void disposal(){
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

  void reset() {
    menuElement::Banner3();
    fbKeypad = new thedynamicfeedbackthing(2, 12, 1);
    Tally = new tally;
    lcd.display();
  }

public: 
  void main(char kP) {
    if (kP=='!') reset();
    if (isDigit(kP)) fbKeypad->add(kP);
    else if (kP=='*') fbKeypad->clrFeedback(); 
    else if (kP=='#') fbKeypad->Obtain();
  }

  void menuAction_enterHook(char kP) { 
    if (fbKeypad = nullptr) fbKeypad = new thedynamicfeedbackthing(2, 12, 1);
    uint8_t x = (uint8_t)fbKeypad->Obtain();
    uint8_t Len = fbKeypad->getLen();
    if (kP=='!') reset();
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
    if (kP=='!') reset();
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
