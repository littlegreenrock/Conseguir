//#define mDEBUG
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

#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <LiquidCrystal_I2C.h>
#include <Key.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <Wire.h>

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
//naviFramework Navigator(SetCode);


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
};
void safeMan::initAdminMode() {  
		//  execute ADMIN conditions, once.
	//dln("admin mode");
	setMode(ADMIN);
	analogWrite(Led_Red, 0x10); // friendly glow
	//singer(Beep);
  //settings::getMiCoLe();
	Admin = new adminmenu();
	
};
void safeMan::initKioskMode() {  
		//  execute KIOSK conditions, once.
	//dln("Kiosk Mode");
	setMode(KIOSK);
	digitalWrite(Led_Red, LOW); // extinguish
	//singer(Beep);
  //settings::getMiCoLe()
	Kiosk = new kioskmenu(5,9,4,1); // placeholder args for [mincodelength, maximum user entered digits, user cursor col, & user cursor row(both 0 indexed)]
};

char singleKeyPress()	{
	char key = KP.getKey();
	if (isDigit(key))  {
		singer(NavUD);
		if (key == '4' || key == '1') navD = Prev;
		else if (key == '6' || key == '9') navD = Next;
		}
	else if (key == '#') navD = Selct;
	else if (key == '*') navD = Esc;
	else if (Idle.checkIfStillPaused()==-1) key='.';
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
						}else {
							if (key == '#') myNav = Selct;
							else if (key == '*') myNav = Clr;
						}
						 //Serial.print(key);	// faster, handle 4 easily
						break;
					case HOLD:
						if (KP.key[i].kchar == '0') myNav = All;
						if (KP.key[i].kchar == '*') myNav = Esc;
						break;
					case RELEASED:
						//Idle.act();
						break;
				}  //  END switch
			} else {
				if ((KP.key[KP.findInList('#')].kstate == HOLD)
						&& (KP.key[KP.findInList('*')].kstate == HOLD)
						&& Idle.getMode(KIOSK)) {
					myNav = LhAS;
					key = 'x';
				}
			}
		}
	// } else if (Idle.checkIfStillPaused()==-1) {
	// 	key = '.';
	// 	myNav = Paws;
	} else {
		key = 0;
		myNav = Stat;
	}
	if (navD!=myNav) {
		navD=myNav;
	}
	return key;
};

void nTest(nav D){

}
void nText(navTEXT N) {
  if (N == SetMaxKeys) Admin->drillDown(N);
  else if (N == mCodeLen) Admin->drillDown(N);
}

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
	KP.setHoldTime(500);
	randomSeed(generateRandomSeed());
	singer(navTEXT::Beep);
  
	
};    // END setup()

void loop() { /*   ┌∩┐(◣_◢)┌∩┐   ::   BEGIN LOOP   ::   (╯°□°）╯︵ ┻━┻   */
	// Heartbeat
	char kP{};		//kP = singleKeyPress();
	kP=DeyBored();
	int8_t paw = Idle.checkDoor((kP));
	circumState Md = Idle.getMode();
	if (paw==-1) {
		kP=='.';  navD=Paws;  	}
	
	

	//if (isPrintable(kP)) Serial.print(kP);	// bad for simultaneous presses. Fine for multiple keys, two finger typing.
	
	if (kP && Md==KIOSK) Kiosk->main(kP);
	if (kP && Md==ADMIN) Admin->main(kP, navD);	
	
};    // END loop()



void kpEvent(KeypadEvent key) {
	switch (KP.getState()) {
		case PRESSED:
			Idle.act();
			break;
		case RELEASED:
			break;
		case HOLD:
			if (key == '0' && KP.keyStateChanged()) navD=All;
			break;
	}
};



	
		
		
		


