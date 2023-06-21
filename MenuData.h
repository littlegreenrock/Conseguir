
/*
 * MenuData.h
 *
 *  Created on: 1 Apr 2023
 *      Author: Jade
 */

#ifndef MENUDATA_H_
#define MENUDATA_H_

#pragma once
#define PGM_P       const char *
#define PGM_VOID_P  const void *
#define PSTR(s) (__extension__({static const char __c[] PROGMEM = (s); &__c[0];}))
#define FPSTR(pstr_pointer) (reinterpret_cast<const __FlashStringHelper /*dumb*/*>(pstr_pointer))   /* https://arduino-esp8266.readthedocs.io/en/latest/PROGMEM.html */
#include "Arduino.h"
#include <stdint.h>
#include "safeMan.h"

extern safeMan Idle;
void dln() ;
template<typename TT, typename... Ttypes>
void dln(TT first, Ttypes... Other) ;
using int_array = int const[];

enum justification : uint8_t { 
	none,left,center,right,
}; /*	0, 0x01, 0x02, 0x03  		*/

enum nav : int8_t { 
	Prev  = -1,		Selct	= 0x20,		uUp   = 0x30,
	Next  = +1,		Esc		= 0x21,		uDn		= 0x31,
	Up		= -10,  All		= 0x22,		Paws  = 0x32,
	Dn		= +10,	Clr 	= 0x24, 
	Stat  =  0,		LhAS	= 0x28,
};

enum navTEXT : uint8_t {
	/*0*/	 Blank				= 0x00, EnterCode	= 0x01, Success			= 0x02,
	/* */	 TakeKey			= 0x03, NavLR			= 0x04, NavUD				= 0x05, 
	/* */	 NavHELP			= 0x06, WhichHook	= 0x07, OrAll				= 0x08, 
	/* */	 AreYouSure		= 0x09, Cancelled	= 0x0a, MaxHooks		= 0x0b,
	/* */	 TheOtherOne	= 0x0c, 
	/*1*/	 SetCode			= 0x10, ShowCode	= 0x11, Install			= 0x12,
	/* */	 Clear				= 0x13, Drop			= 0x14, DevSetting	= 0x15,
	/*2*/	 SetMaxKeys		= 0x20, KP_BL			= 0x21, LCD_BL			= 0x22,
	/* */	 Beep					= 0x23, SuSetting	= 0x24, 
	/*3*/	 mCodeLen			= 0x30, setDate		= 0x31, setTime			= 0x32,
	/* */	 setTZ				= 0x33, T1CtRTA		= 0x34, 
}; 

uint8_t T[][7] {		//[8]
		// parent-Tier,
		//            menuItem uniqueID (ff for none)
		{0,	0x00, 0x01, 0x07, 0x08, 0x09, 0x0a}, //0xff		//	Tier 0, Kiosk
		{1, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15}, //0xff		//	Tier 1
		{1, 0x20, 0x21, 0x22, 0x23, 0x24, 0xff}, //0xff		//	Tier 2
		{2, 0x30, 0x31, 0x32, 0x33, 0x34, 0xff}, //0xff		//	Tier 3
		//{0, 0x02, 0x03, 0x04, 0x05, 0x06, 0x0b, 0x0c},		//	Tier 0b, additional misc
};

navTEXT L[][6] {
	{Blank, EnterCode, WhichHook, OrAll, AreYouSure, Cancelled},
	{SetCode, ShowCode, Install, Clear, Drop, DevSetting},
	{SetMaxKeys, KP_BL, LCD_BL, Beep, SuSetting},
	{mCodeLen, setDate, setTime, setTZ, T1CtRTA},
	//{Success, TakeKey, NavLR, NavUD, NavHELP, MaxHooks, TheOtherOne},
}; 

extern void nTest(nav);
extern void nText(navTEXT);

// array of function pointers
void (*doAdminActions []) () =
{
	[] {  //  SetCode ----
		;		},
	[] {  //  ShowCode 
		;		},
	[] {  //  Install 
		;		},
	[] {  //  Clear 
		;		},
	[] {  //  Drop 
		;		},
	[] {  //  DevSetting 
		  nText(SetMaxKeys);		},
			
	[] {  //  SetMaxKeys ----
		;		},
	[] {  //  KP_BL 
		;		},
	[] {  //  LCD_BL 
		;		},
	[] {  //  Beep 
		;		},
	[] {  //  SuSetting 
		nText(mCodeLen);		},
			
	[] {  //  mCodeLen ----
		;		},
	[] {  //  setDate 
		;		},
	[] {  //  setTime 
		;		},
	[] {  //  setTZ 
		;		},
	[] {  //  T1CtRTA 
		;		},
};


typedef struct {
	 char label [17];
	 uint8_t value;  // doAdminAction[idx]
} mLabel;

const mLabel kioskMap [13] PROGMEM = { 	// Tier 0. Parent 0
	{  ""									,0x4f	},
	{  "Enter code"				,0x4f	},	
	{	 "Success!"					,0x8f	},	
	{	 "Take your key."		,0x8f	},	
	{	 "*Esc \x02\x38  \x32\x01 Ent#"	,0xcf	},	
	{	 "\x02[8]  Cancel [*]"	,0x8f	},	
	{	 "For help: hold *"	,0x4f	},	
	{	 "which hook?"			,0x4f	},	
	{	 "ALL? (hold 0)"		,0x4f	},	
	{	 "are you sure?"		,0x8f	},	
	{	 "Cancelled"				,0x8f	},	
	{	 "  8  16  24  32"	,0x4f	},	
	{	 "Rule Them All"		,0xcf	},	
};
const mLabel adminMap [6] PROGMEM = {  // Tier 1. Parent 1
	{	"Set Code"					,0x40	},	
	{	"Show Code(s)"			,0x41	},	
	{	"Install Key"				,0x42	},	
	{	"Clear Code(s)"			,0x43	},	
	{	"Drop Key(s)"				,0x44	},	
	{	"Device settings"		,0x45	},	
};
const mLabel settingMap [5] PROGMEM = {  // Tier 2. Parent 1
 	{	 "Set Max Keys"			,0x46	},	
	{	 "Keypad backlight"	,0x47	},	
	{	 "LCD backlight"		,0x48	},	
	{	 "Beep volume"			,0x49	},	
	{	 "Admin. mode"			,0x4a	},	
};
const mLabel suPrefsMap [5] PROGMEM = {	// Tier 3. Parent 2. 
	{	 "_min code length"	,0xcb	},	
	{	 "_set date"				,0xcc	},	
	{	 "_set time"				,0xcd	},	
	{	 "_set time zone"		,0xce	},	
	{	 "The One Code to"	,0xcf	},	
};

typedef navTEXT (*DrillDownFun) (nav);


const uint8_t menuMapSizes[] = {13,6,5,5};    // 1-idx

const mLabel* mLabelGroup[] = {
	kioskMap, adminMap, settingMap, suPrefsMap 
};

// // Print a string from Program Memory directly to save RAM 
// void progressivePrint (const char * str) {
// 	char c;
// 	if (!str) return;
// 	while ((c = pgm_read_byte(str++)))
// 		Serial.print (c);
// 		//lcd.print(c);
// 			// USE: progressivePrint ((const char *) &(mLabelGroup[tier][item][0]);
// }
// const char* charPROGMEM(navTEXT request) {
// 	return &(mLabelGroup[highNibb((byte)request)][lowNibb((byte)request)].label[0]);
// }
// uint8_t lennyP(navTEXT request) {  
// 	return (uint8_t)strlen_P(mLabelGroup[highNibb((byte)request)][lowNibb((byte)request)].label);
// }
// justification getJ(navTEXT request) {
// 	int8_t j = pgm_read_byte(mLabelGroup[highNibb((byte)request)][lowNibb((byte)request)].value);
// 	return (justification)highNibb(j);
// }
// void fetch_mLabel(mLabel* dest_mLabel, navTEXT request) {
// 	const byte tier = highNibb(request);
// 	const byte item  = lowNibb(request);
// 	memcpy_P (dest_mLabel, &(mLabelGroup[highNibb((byte)request)][lowNibb((byte)request)]), sizeof(mLabel));
// 	char*s;
// 	s = strchr(dest_mLabel->label, '\0');
// }

// uint8_t fetchNavLabel(char* destBuffer16, navTEXT request, justification J = left) {  
// 		const byte tier = highNibb((byte)request);
// 		const byte item  = lowNibb((byte)request);
// 		char dumb[9] {"%0s%-16s"}; // J==left
// 		if (J==right) dumb[4]='+';
// 		mLabel tempFetch;
// 		memcpy_P (&tempFetch, &(mLabelGroup[highNibb((byte)request)][lowNibb((byte)request)]), sizeof tempFetch);
// 		if (J==center) dumb[1] = (char)0x30 + (byte)(16-sizeof tempFetch.label)/2;  // strlen(fetch.label) ?????
// 		snprintf(destBuffer16, 17, dumb, "", tempFetch.label);
// 		return tempFetch.uniqID;
// }

enum AC : uint8_t {
	ZERO = 0x00,	//	- 	//  initialisation state.
	CLRD = 0X01,	//	b0	//  hook has cleared. Device positive no key.
	RDMD = 0x02,	//	b1	//  key was installed, has been redeemed
	INST = 0x04,	//	b2	//  key was installed
	ORPH = 0x08,	//	b3	//  orphan record
	VIP  = 0x10,	//	b4	//  Key hook marked VIP
	RLK  = 0x20,	//	b5	//  key hook temp marked r/o, locked
	BLCK = 0x40,	//	b6	//  key hook out of service
	foo_spare = 0x80,	//	b7	//  {unused}
};

//   ~   -   -   ~   -   -   ~   -   -   ~   -   -   ~   -   -   ~   -   -   ~

/* ~ CUSTOM ~ LCD ~ CHARACTERS ~ */
	const uint8_t Lady_c[8] PROGMEM = { 0x03, 0x05, 0x09, 0x01, 0x02, 0x12, 0x0C, 0x00 };
	const uint8_t keyA_c[8] PROGMEM = { 0x00, 0x00, 0x0e, 0x1f, 0x17, 0x1e, 0x0e, 0x00 };
	const uint8_t keyB_c[8] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0x1f, 0x0a, 0x02, 0x00 };
	const uint8_t Tick_c[8] PROGMEM = { 0x00, 0x01, 0x03, 0x16, 0x1c, 0x08, 0x00, 0x00 };
	const uint8_t Back_c[8] PROGMEM = { 0x02, 0x06, 0x0E, 0x1E, 0x0E, 0x06, 0x02, 0x00 };
	const uint8_t locked[8] PROGMEM = { 0x00, 0x0e, 0x11, 0x11, 0x1f, 0x1b, 0x1b, 0x1f };
	const uint8_t unlock[8] PROGMEM = { 0x0e, 0x11, 0x10, 0x10, 0x1f, 0x1b, 0x1b, 0x1f };
	const uint8_t arrUp_c[8] PROGMEM = { 0x04, 0x0e, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00 };
	const uint8_t arrDn_c[8] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0x1f, 0x0e, 0x04, 0x00 };
	const uint8_t aUpDn_c[8] PROGMEM = { 0x04, 0x0e, 0x1f, 0x00, 0x00, 0x1f, 0x0e, 0x04 };
	const uint8_t sungQ_c[8] PROGMEM = { 0x00, 0x00, 0x00, 0x03, 0x03, 0x0f, 0x0f, 0x00 };
	const uint8_t sungW_c[8] PROGMEM = { 0x00, 0x00, 0x00, 0x0c, 0x0c, 0x0f, 0x0f, 0x00 };
	const uint8_t sungA_c[8] PROGMEM = { 0x0f, 0x0f, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00 };
	const uint8_t sungS_c[8] PROGMEM = { 0x0f, 0x0f, 0x0c, 0x0c, 0x00, 0x00, 0x00, 0x00 };
/*  */
const uint8_t* const CustomGlyphs[] PROGMEM = {
	Lady_c, keyA_c, keyB_c, Tick_c, Back_c,    /* leave */
	locked, unlock, arrUp_c, arrDn_c, aUpDn_c, /* me    */
	sungQ_c, sungW_c, sungA_c, sungS_c,        /* alone */
};
enum Glyph : uint8_t {
	/* */ Lady = 0,  KeyA,  KeyB,  Tick,  Back,
	/* */ padLocked,  padUnlocked,  arrowUp,  arrowDn,  arrUpandDn,
	/* */ custLogoQ,  custLogoW,  custLogoA,  custLogoS,
};

const int _Note_Hz440[61] PROGMEM = { 
			65,   69,   73,   78,   82,   87,   93,   98,  104,  110,
		 117,  123,  131,  139,  147,  156,  165,  175,  185,  196,
		 208,  220,  233,  247,  262,  277,  294,  311,  330,  349,
		 370,  392,  415,  440,  466,  494,  523,  554,  587,  622,
		 659,  698,  740,  784,  831,  880,  932,  988, 1047, 1109,
		1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,  
		2093, 
		};

// typedef struct {
// 	 uint8_t N;
// 	 uint8_t D;
// 	 uint8_t V;
// 	 uint8_t T;
// } ndvt;


// const ndvt Requests [8] PROGMEM = {
// 	{  0,  0,  0,  0 },		//	Blank
// 	{ 30, 25, 30,  7 },		//	Success
// 	{	28, 25, 30,  7 },		//	Beep
// 	{ 22, 25, 30,  5 },		//	Cancelled
// 	{ 23, 15, 20,  5 },		//	NavLR
// 	{ 22, 15, 20,  5 },		//	NavUD
// 	{ 18,  5, 20,  5 },		//	AreYouSure
// 	{ 21, 15, 20,  5 },		//	Clear
// };

// ndvt requestNote (uint8_t n) {
// 	return (Requests[n]);
// }


#endif /* MenuData.h */
