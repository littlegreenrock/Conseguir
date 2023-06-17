

/*
 * safeMan.h
 *
 *  Created on: 23 May 2023
 *      Author: Jade
 */

#ifndef SAFEMAN_H_
#define SAFEMAN_H_
#pragma once

#include "Arduino.h"
extern const uint8_t Door_Pin;
extern Keypad KP;

void dln() ;
template<typename TT, typename... Ttypes>
void dln(TT first, Ttypes... Other) ;


enum circumState { 
  INIT = 0x0,     // initialization. should never return here.
  KIOSK = 0x1,    // guest function
  STANDBY = 0x2,  // security functions and internal delays
  ADMIN = 0x3,    // Admin mode, menu
  INVALID = 0x4,  // the last option, should never reach here.
};

class switchbouncer {
public: 
	enum swState : bool { 
		Closed = 0, Open = 1, /* */   };

  switchbouncer () { }
  switchbouncer (uint8_t pin_number) : _pin{pin_number} {
		pinMode(_pin, INPUT_PULLUP);
	 }
  ~switchbouncer () { }
  swState getState() {
    _edgeDetect();
    return theDoor;  
  }

private:  
  volatile uint8_t _edgeHistory;
  const uint8_t _pin{Door_Pin};
  volatile swState theDoor;

  uint8_t _readSw() { 
    _edgeHistory <<= 1;   // it's a bitshift assignment: << =
    bitWrite(_edgeHistory, 0, digitalRead(_pin));
    return _edgeHistory;
  }
  void _edgeDetect() { 
    switch (_readSw()) {
      case 0x7f:  //  rising edge
        theDoor = Open;
        break;
      case 0x80:  //  falling edge
      case 0x00:  //  open circuit, no signal, this is the default
        theDoor = Closed;
      break;
    }   //  END switch
  }
};  //    END class switchbouncer


class safeMan : private switchbouncer {
public:	// 'structors
	safeMan() {
		setMode(circumState::INIT);
	}
	~safeMan() {}

private:
	uint32_t _millicent;  // <-- needed by _blinkie() to compare with millis()
	//uint _Idle_ms;
	uint _pawscent{};
	circumState _nowMode;
	uint8_t SUPERUSER;
	int _BlinkPeriod;
	int8_t _BlinkDuty;

	circumState _checkDoorstateTogglingConditions(swState S) {
		if (_nowMode == INIT) setMode(STANDBY);
		else if (S == Closed) {
			if (_nowMode == ADMIN) initStandbyMode();
			else if (_nowMode == STANDBY) initKioskMode();
		} else if (S == Open) {
			if (_nowMode == KIOSK) initStandbyMode();
			else if (_nowMode == STANDBY) initAdminMode();  // intentionally last
		}
		return _nowMode;
	}

	uint _blinkie() {
		//	so you know it hasn't crashed
		//	returns the eff. idle time in ms since last act();
		if ((millis() % _BlinkPeriod) < _BlinkDuty) {
			digitalWrite(LED_BUILTIN, 1);
		} else digitalWrite(LED_BUILTIN, 0);
		return max(0, (uint)(millis() - _millicent));
	}

	void initStandbyMode();  // See main()
	void initAdminMode();    // See main()
	void initKioskMode();    // See main()

	void idleCheckList(uint idletimefromblinkie) {
		 // RESPONSE TO IDLE TIMEOUTS
			switch (idletimefromblinkie) { 
			case (1 - 49):       //	~keypress debounce time.
			  //  check display repetition faults
				break;
			case (50 - 299):  //
			  //
				break;
			case (300 - 499):  //	~longpress key
			                   //
				break;
			case (500 - 999):  //	~0.5s
			  //  end melody
				break;
			case (1000 - 2999):  //	~1s
			  //
			case (3000 - 11999):  //	~3s
			  // end short paws pause
				break;
			case (12000 - 23999):  //	~12s
			  //	12s, KIOSK: clear any entered user input (as if '*')
			  //	ADMIN: clear any entered input (as if '*'), with brief timeout warning.
				break;
			case (24000 - 29999):  //	~24s
			                       //	KIOSK: reset. dim BL's
			                       //	ADMIN: cancel and reset menu. less brief timeout warning?
				break;
			case (30000 - 44999):  // ~30s
			                       //	ADMIN: cancel all, display countdown, obnoxious timeout warning
				break;
			case (45000 - 54999):
				//	ADMIN: contact Superior, device may be unsecure and unattended.  Display warn.
				break;
			case (55000 - 59999):
				//	ADMIN: assume the worst. contact Superior. Display fault.
			default:  // beyond 60s
			  // engage low power mode.
				break;
		};  //  END switch(idletime)
	}

public:
	circumState checkDoor() {
		_checkDoorstateTogglingConditions(getState()); // it's correct, leave it
		idleCheckList(_blinkie());	// this too
		return _nowMode;
	}

	bool getMode(circumState guess) { return (guess == _nowMode); }
	
	circumState getMode() {	return checkDoor(); }
	
	void setMode(circumState mode) { 
		if (_nowMode != mode) {
			_nowMode = mode;
			act();	}
	}
	
	void act() { 
		_millicent = millis();
		setPause();	// cancel pause
	}
	void setPause() {	_pawscent = 0;	}

	void setPause(int ms) { _pawscent = ms + millis(); }
	
	bool checkForPausedRanout() {
		if (_pawscent>0 && _pawscent<millis()) {
			setPause();
			return true;	// you only get this once.
		}
		return false;
	}
	
	bool isSU() { return (SUPERUSER > 1); }
	
	bool idleMoreThan(unsigned int ms) {return(millis()-_millicent>ms);}

	uint blinkieSetup(uint setPeriod, uint8_t percent) {
		if (setPeriod > 0 && percent > 0 && percent < 100) {
			_BlinkPeriod = setPeriod;
			_BlinkDuty = (percent * setPeriod) / 100;
		}
		return (uint)_BlinkDuty;  // expressed as (ms/setPeriod)
	}
};  //    END class safeMan


#endif // def SAFEMAN_H_
