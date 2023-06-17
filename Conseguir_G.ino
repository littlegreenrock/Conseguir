#define mDEBUG
#define DEBUG 1
/*
 * 
 * Version: Conseguir_G 22 May 2023
 * 
 * 
 */

#include "Arduino.h"
//	PINS
//Green_Led == LED_BUILTIN = 13;
const uint8_t Buzzer_Pin = A1;
const uint8_t Led_Red = 10;
const uint8_t Door_Pin = 12;
const uint8_t wakePin = 2;

//using namespace std;
//#include "memdebug.h"
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <LiquidCrystal_I2C.h>
#include <Key.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <Wire.h>
//#define MAPSIZE 4
//#define LIST_MAX 4


#include "MenuData.h"
#include "myMenu.h"
#include "HookEE.h"
#include "safeMan.h"

#ifdef KEYPAD_H
	const byte ROWS = 4; //four rows
	const byte COLS = 3; //four columns
	char numericKeys[ROWS][COLS] = {
		{'1','2','3'},	{'4','5','6'},	{'7','8','9'},	{'*','0','#'} 	};
	byte rowPins[ROWS] = {6, 5, 4, 3 };	//connect to the row pinouts of the keypad
	byte colPins[COLS] = {9, 8, 7 }; 		//connect to the column pinouts of the keypad
	Keypad KP = Keypad( makeKeymap(numericKeys), rowPins, colPins, ROWS, COLS); 
#endif// def KEYPAD_H

// more dependancies
const uint8_t c_noKeyPerBank = 8;
const uint8_t c_maxHWDepdntKeys = 32;
const int c_mem_offset_keyblock = 0x20;  // EEPROM memory block start address_Hx
const bool _CoolEffectsBra = true;
nav navD {Stat};

// class pointers
kioskmenu* Kiosk {nullptr};
adminmenu* Admin {nullptr};

// static classes
LiquidCrystal_I2C lcd(0x27, 16, 2);
safeMan Idle;
//adminmenu AM(0);

ISR (WDT_vect) {  // watchdog interrupt
	 wdt_disable();  // disable watchdog
}  // end of WDT_vect
void sleep_watchdog() {
	ADCSRA = 0;  
	MCUSR = 0;     
	WDTCSR = bit (WDCE) | bit (WDE);
	//WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1);    // set WDIE, and 1 second delay
	WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
	wdt_reset();  // pat the dog
	set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
	noInterrupts ();           // timed sequence follows
	sleep_enable();
	// turn off brown-out enable in software
	MCUCR = bit (BODS) | bit (BODSE);
	MCUCR = bit (BODS); 
	interrupts ();             // guarantees next instruction executed
	sleep_cpu ();  
	// cancel sleep as a precaution
	sleep_disable();
}
void sleep_interrupt() {
	ADCSRA = 0;  
	set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
	sleep_enable();
	noInterrupts ();
	// will be called when pin D2 goes low  
	attachInterrupt (0, wake, FALLING);
	EIFR = bit (INTF0);  // clear flag for interrupt 0
	MCUCR = bit (BODS) | bit (BODSE);
	MCUCR = bit (BODS); 
	interrupts ();  // one cycle
	sleep_cpu ();   // one cycle
	sleep_disable();  // precautionary
}
void wake() {
	sleep_disable();
	detachInterrupt (0);   // precautionary
}
void dln() {
	Serial.println(); }
template<typename TT, typename... Ttypes>
void dln(TT first, Ttypes... Other) {
	if (DEBUG) Serial.print(first);
	else NULL;
	dln(Other...);
}
uint32_t generateRandomSeed() {
	int32_t seed;
	for (int i = 0; i < 6; i++) {
		seed += analogRead(i);
		seed *= analogRead(i);
	}
	//dln("\n----\trnd seed: ", abs(seed));
	return abs(seed);
}
/* BEGIN Functions  °º¤ø,¸¸,ø¤º°`°º¤ø,¸,ø¤°º¤ø,¸¸,ø¤º°`°º¤ø,¸  */

void safeMan::initStandbyMode() {  
		//  execute STANDBY conditions, once.
	Idle.act();
	lcd.noCursor();
	lcd.clear();
	//dln("init standby");
	setMode(STANDBY);
	//singer(Beep);
	delete Kiosk;
	delete Admin;
	Kiosk = nullptr;
	Admin = nullptr;
	delay(50);
};
void safeMan::initAdminMode() {  
		//  execute ADMIN conditions, once.
	//dln("admin mode");
	setMode(ADMIN);
	analogWrite(Led_Red, 0x10); // friendly glow
	//singer(Beep);
	Admin = new adminmenu(0);  // dummy arg 
	
};
void safeMan::initKioskMode() {  
		//  execute KIOSK conditions, once.
	//dln("Kiosk Mode");
	setMode(KIOSK);
	digitalWrite(Led_Red, LOW); // extinguish
	//singer(Beep);
	Kiosk = new kioskmenu(5,9,4,1); // placeholder args for (mincodelength, maximum user entered digits, user cursor col, user cursor row(0 indexed))
};

char keyPress()	{
	char key = KP.getKey();
	if (isDigit(key))  {
		singer(NavUD);
    if (key == '4' || key == '1') navD = Prev;
    else if (key == '6' || key == '9') navD = Next;
		}
  else if (key == '#') navD = Selct;
  else if (key == '*') navD = Esc;
	else if (Idle.checkForPausedRanout()) key='.';
	else key = 0;
	return key;
		/*  ie: return KP.getKeys();
			char getKey = give me the first keypress (like you only can handle one at a time)
			bool getKeys = are there any keys in the buffer waiting?
			bool isPress(char) = is this char in the buffer? (is this button pressed?)
			bool keyStateChanged() = has keystate changed since last time?
			bool findInList(char\\int) = is this key active?
		*/
};

char DeyBored() {
  char key{};
  nav myNav;
  if (KP.getKeys()) {
    for (int i = 0; i < 4; i++)  // Scan first 4 from key list.
    {
      if (KP.key[i].stateChanged) {
        switch (KP.key[i].kstate) {
          case PRESSED:
            key = KP.key[i].kchar;
            if (isDigit(key)) {
              singer(NavUD);
              if (key == '4' || key == '1') myNav = Prev;
              else if (key == '6' || key == '9') myNav = Next;
            } else {
              if (key == '#') myNav = Selct;
              else if (key == '*') myNav = Clr;
            }
            Serial.print(key);
            break;
          case HOLD:
            if (KP.key[i].kchar == '0') myNav = All;
            break;
          case RELEASED:
            Idle.act();
            if (KP.key[i].kchar == '*') myNav = Esc;
            break;
        }  //  END switch
      } else {
        if ((KP.key[KP.findInList('#')].kstate == HOLD)
            && (KP.key[KP.findInList('*')].kstate == HOLD)
            && Idle.getMode(KIOSK)) {
          dln("That there is a spare armadillo");
          myNav = LhAS;
        }
      }
    }
  } else if (Idle.checkForPausedRanout()) {
    key = '.';
    myNav = Paws;
  } else {
    key = 0;
    myNav = Stat;
  }
  if (navD!=myNav) {
    navD=myNav;
  }
  return key;
};


/*  °º¤ø,¸¸,ø¤º°`°º¤ø,¸,ø¤°º¤ø,¸¸,ø¤º°`°º¤ø,¸ END Functions */

void setup() { /*   (ノಠ益ಠ)ノ彡   ::   BEGIN SETUP   ::   ┌∩┐(⋟﹏⋞)┌∩┐   */ 
	lcd.begin();
	if (DEBUG) {
		Serial.begin(115200);
		while (!Serial) { Serial.println(); };
	}
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(Led_Red, OUTPUT);
	pinMode(Door_Pin, INPUT_PULLUP);
	pinMode(Buzzer_Pin, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
	digitalWrite(Led_Red, LOW);
	digitalWrite(Buzzer_Pin, 0);
	digitalWrite(wakePin, HIGH);  // enable PULLUP for INPUT pin
	Idle.blinkieSetup(1000, 5);
	KP.addEventListener(kpEvent);		//	TODO: wtf does this do?
	KP.setHoldTime(350);
  randomSeed(generateRandomSeed());
	singer(navTEXT::Beep);
  
};    // END setup()

void loop() { /*   ┌∩┐(◣_◢)┌∩┐   ::   BEGIN LOOP   ::   (╯°□°）╯︵ ┻━┻   */
	circumState Md = Idle.checkDoor();	// replaces blinky and checkSecurity
  char kP{};
	//kP = keyPress();		// Only returns one keypress at a time
  kP=DeyBored();
	
	if (kP && Md==KIOSK) Kiosk->main(kP);
	if (kP && Md==ADMIN) Admin->main(kP);	
	
};    // END loop()



void kpEvent(KeypadEvent key) {
		// enum nav : int8_t { 
		// 	Prev  = -1,		Selct	= 0x20,		uUp   = 0x30,
		// 	Next  = +1,		Esc		= 0x21,		uDn		= 0x31,
		// 	Up		= -10,  All		= 0x22,		
		// 	Dn		= +10,	LSta	= 0x24, 
		// 	Stat  =  0,		LHas	= 0x28,
		// };
	switch (KP.getState()) {
		case PRESSED:
			break;
		case RELEASED:
      Idle.act();
			if (key == '*') {}
			break;
		case HOLD:
      if (key == '0') dln("double handling -> navD = All");
      if (key == '*') dln("cancel-Esc");
			break;
	}
};



  
    
    
    


