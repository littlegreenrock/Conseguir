
/*
 * HookEE.h
 *
 *  Created on: 24 May 2023
 *      Author: Jade
 */

#ifndef HOOKEE_H
#define HOOKEE_H
#pragma once

#include "Arduino.h"
#include "MenuData.h"

extern const uint8_t c_noKeyPerBank;
extern const uint8_t c_maxHWDepdntKeys;
extern const int c_mem_offset_keyblock;
const uint c_objSizeRec = 6;  // sizeof(hookRec) =  6B.



class noTryNoCatch  {
private:
  uint8_t getMxKs;
public:
  noTryNoCatch(){};
  ~noTryNoCatch(){};

  bool testKeyPos(uint8_t keyPos){
    return (keyPos>0 && keyPos<=c_maxHWDepdntKeys && keyPos <= getMxKs) ;
  }
  bool testAddr_Hx(){}
  bool testEeAddr(){}
  bool testDigits(){}     
  bool testAvail(){}

};  //  END class noTryNoCatch



class converter {
  // Just converts, does not check or validate.
    // A : Addr_Hx - solenoid location of hook.
    // K : keyPos - linear human reference to rl hook.
    // E : eeAddr - the EEPROM address for this key/hook record.
  public:
    uint AtoE(const uint8_t Addr_Hx) {
      return ((((Addr_Hx >> 4) - 1) * 8) + (Addr_Hx & 0x0f) * c_objSizeRec) + c_mem_offset_keyblock;
    }
    uint8_t EtoK(int eeAddr) {
      return ((eeAddr - c_mem_offset_keyblock) / c_objSizeRec) + 1;
    }
    uint8_t KtoA(uint8_t keyPos) {
      return ((((keyPos - 1) / c_noKeyPerBank) + 1) << 4) + (keyPos - 1) % c_noKeyPerBank;
    }
    uint KtoE(uint8_t keyPos) {
     return (keyPos - 1) * c_objSizeRec + c_mem_offset_keyblock;
    }
    uint8_t AtoK(uint8_t Addr_Hx) {
      return (Addr_Hx >> 4) * c_noKeyPerBank + (Addr_Hx&0x0f) +1;
    }
    uint32_t codeToUint32_t(const char* Code, const byte Len) {
      uint32_t someHxCode = (atol(Code) << 4) + (Len & 0x0f);
      return someHxCode;
    }
    uint8_t codeToBuffer(uint32_t someHxCode, char* destBuffer10) {
      // stores code as digits string in destBuffer10. returns str Length.
      char dumb[7];
      uint8_t Len = someHxCode&0x0f;
      sprintf(dumb, "%%0%dlu", Len);
      sprintf(destBuffer10, dumb, (someHxCode >> 4));
      return Len;
    } 
    
};  //  END class converter


class basicRomReadWrite {
public: 
  uint8_t readAddr(int E) 
  { // returns Addr_Hx value (A) at rom location (E)
    return EEPROM.read(E +0);
  }
  uint32_t readCode(uint E) 
  {
    uint32_t someHxCode;
    EEPROM.get(E + 1, someHxCode);
    return someHxCode;
  }
  uint8_t readAvail(uint E) {
    return EEPROM.read(E + 5);
  }
  void writeCode(uint E, uint32_t someHxCode) {
    EEPROM.put(E +1, someHxCode);
  }
  void writeAvail(uint E, uint8_t A) {
    EEPROM.update(E +5, A);
  }
};  //    END class basicRomReadWrite


class bitFlagInterface {
public:
  enum AC : uint8_t {
    ZERO = 0x00,	//	- 	//  initialisation state.
    CLRD = 0X01,	//	b0	//  hook cleared. Device positive no key.
    RDMD = 0x02,	//	b1	//  installed key, has been redeemed
    INST = 0x04,	//	b2	//  key was installed
    ORPH = 0x08,	//	b3	//  orphan record
    VIP  = 0x10,	//	b4	//  Key hook marked VIP
    RLK  = 0x20,	//	b5	//  key hook temp marked r/o, locked
    BLCK = 0x40,	//	b6	//  key hook out of service
    foo_spare = 0x80,	//	b7	//  {unused}
  };
  //INST -> RDMD -> CLRD; Tog:ORPH, VIP, FLK, BLCK, foo_spare
  // |ZERO won't break anything. |foo will go unnoticed. can still set an invalid state without care or validation.
public:
  uint8_t _addBit(uint8_t &Avail, AC name) {
    return (Avail|=name); 
  }
  uint8_t _clearBit(uint8_t &Avail, AC name) {
    return (Avail&=(~name));
  }
  uint8_t _flipBit(uint8_t &Avail, AC name) {
    return (Avail^=name);
  }
public:
  bool isBit(const uint8_t Avail, AC name) {
    return (Avail&name);
  }
  bool canDrop(const uint8_t Avail) {
    if ((Avail&0x6f)==INST) return true;
    else return false;
  }
  bool redeem(uint8_t &Avail) {
    if (canDrop(Avail)){
      _addBit(Avail, RDMD);
      _clearBit(Avail, INST);
      return true;
    }
  }
  
}; //  END class bitFlagInterface


class recordLock : public  basicRomReadWrite {
public: // 'structors
	recordLock(uint E) : _eeAddr{E}, _ownership{lockRecord(E)}	{};
	~recordLock(){
		closeRecord(); 
	};
private:
	const uint _eeAddr;
	const bool _ownership;  // == write permission

	bool lockRecord(uint E) 
  {
		uint8_t Avail = basicRomReadWrite::readAvail(E);
		if (bitRead(Avail, 5)==1) return false; // rec was already locked
		bitSet(Avail, 5);
		basicRomReadWrite::writeAvail(_eeAddr, Avail);
		return true;	// rec locked, I have control. 
	}
  void closeRecord() 
  {
		if (_ownership) {
      uint8_t A = basicRomReadWrite::readAvail(_eeAddr);
      bitClear(A, 5);
      basicRomReadWrite::writeAvail(_eeAddr, A);
    }
  }
    
public:
	void saveAndCloseRecord(uint8_t Avail, uint32_t someHxCode) 
  { 
		if (_ownership) {
      // uses update, so if the data is the same, no rom write occurs
      basicRomReadWrite::writeCode(_eeAddr, someHxCode);
      basicRomReadWrite::writeAvail(_eeAddr, Avail);
    }
	}
	void changeAvailStatus(uint8_t Avail) 
  { 
	  if (_ownership) basicRomReadWrite::writeAvail(_eeAddr, Avail);
	}
};  //  END class recordLock


class orphanRec : public basicRomReadWrite, public converter , public bitFlagInterface{
public: //  'structors
    // Orphaned records always begin with a code. 
  orphanRec(const char* digits, const byte Len) : Code{codeToUint32_t(digits, Len)} {};
  orphanRec(uint32_t someHxCode) : Code{someHxCode} {};
  ~orphanRec() {
    if (Code) dln("\tOrphan --> Soylent Green");
  }

private:
  void _commitRecord() {
    basicRomReadWrite::writeAvail(AtoE(Addr_Hx), this->Avail);
  }

public:
    uint8_t Addr_Hx {};
    const uint32_t Code;  
    uint8_t Avail = (uint8_t)AC::ORPH;

  uint8_t areYouMyMummy7(uint8_t keyPos) {
    uint32_t someHxCode = readCode(KtoE(keyPos)); 
    uint8_t A = readAvail(KtoE(keyPos));
    if (Code == someHxCode && canDrop(A)) {
      Addr_Hx = readAddr(KtoE(keyPos));
      Avail=A;

      return Addr_Hx;
    }else return 0;
  }
  bool dropKey(uint8_t solenoid) {
    if (solenoid == this->Addr_Hx) {
      bitSet(this->Avail, 1);
			bitClear(this->Avail, 2);
      _commitRecord();
    }
  }
  uint8_t getAvail() {
    return this->Avail;
  }
  uint8_t browseKeyListForMatch(uint32_t knownCode, uint8_t* listofKeyPos, uint8_t listLen)
  {
    if (knownCode<5 || listLen==0) return 0;
    uint8_t count=0;
    for (int i = 0; i<listLen; i++){
      uint32_t someHxCode = readCode(KtoE(listofKeyPos[i]));
      if (knownCode!=someHxCode) listofKeyPos[i]=0;
      else count++;
    }
    return count;
  }   
};  //  END class orphanRec



class hookRec : public recordLock, public bitFlagInterface, public converter {
  public: //  'structors
    //  hookRec always begins referenced to a rom record, and locked.
    hookRec(uint8_t keyPos) : recordLock{KtoE(keyPos)} {
      // acquire record from eeprom
      if (keyPos>0 && keyPos<=c_maxHWDepdntKeys) {
        Addr_Hx = KtoA(keyPos);
        Code    = readCode(AtoE(Addr_Hx));
        Avail   = readAvail(AtoE(Addr_Hx));
      }
      else delete this;   // TODO fail record assignment.
    }
    ~hookRec() {
      
    }
  protected:
    uint8_t Addr_Hx;
    uint32_t Code;
    uint8_t Avail;
  public:
    void setCode(const char* digits, const byte Len){
      setCode(codeToUint32_t(digits, Len));
    }
    void setCode(uint32_t someHxCode){
      Code = someHxCode;
    }
    bool operator==(const hookRec& compare) { 
      return Addr_Hx == compare.Addr_Hx && Code == compare.Code && Avail == compare.Avail;
    }
    
    bool compareCode(uint32_t someHxCode) {
      return (this->Code == someHxCode);
    }

    bool install() {
      _addBit(Avail, INST);
      _clearBit(Avail, RDMD);
      _clearBit(Avail, CLRD);
    }
    bool makeVIP(){
      _addBit(Avail, VIP);
    }
    bool flagAsOOS(){
      _addBit(Avail,BLCK);
    }

};  //  END class hookRec



class tally : private basicRomReadWrite, private converter {
  private:
    uint8_t Size{};
    uint8_t* theListOfKeys;
    AC Focus;
    AC subFocus;
    void _HeadCount() {
      uint8_t A, idx=0;
      for (int iKeyPos = 1; iKeyPos <= c_maxHWDepdntKeys; iKeyPos++) {
        A = readAvail(KtoE(iKeyPos));
        if (A&Focus) idx++;
        Size = idx;
      }
      _ListBuild(Size);
    }
    void _ListBuild(uint8_t size) {
      theListOfKeys = new uint8_t[size]; 
      uint8_t A, idx=0;
      for (int iKeyPos = 1; iKeyPos <= c_maxHWDepdntKeys; iKeyPos++) {
        A = readAvail(KtoE(iKeyPos));
        if (A&Focus) theListOfKeys[idx++]=iKeyPos;
      }
    }
  public:
    tally(AC discriminate = INST) : Focus {discriminate} { 
      subFocus=ZERO;
      _HeadCount();
    }
    tally(AC discriminate, AC additional = ZERO) : Focus {discriminate}, subFocus{additional} { 
      _HeadCount();
    }
    ~tally() { 
      delete[] theListOfKeys;
      theListOfKeys = nullptr;      
      //dln("\t--> destruct Tally");
    }
    uint8_t giveMe(char* buffer16, uint8_t item) {
      uint8_t keyPos{};
      if (item>0&&item<=Size){
        keyPos = theListOfKeys[item-1];
        hookRec temp(keyPos);
        char sub;
        //if (subFocus>ZERO && temp.is(subFocus)) sub='*';
        snprintf(buffer16, 17, "%2d: %-8s %-3c", keyPos, readCode(KtoE(keyPos)),sub);
      }
      return keyPos;
    }
    uint8_t giveMe(uint8_t entryNum) {
      if (entryNum>0 && entryNum<=Size) return theListOfKeys[entryNum-1];
      else return 0;
    }
    uint8_t Adoption(orphanRec* Lost) {
      uint8_t resultaddress;
      for (int i = 0; i<Size; i++) {
        resultaddress = Lost->areYouMyMummy7(theListOfKeys[i]);
      }
      return resultaddress;
    }
    uint8_t getListSize() {
      return Size;
    }
};



#endif // def HOOKEE_H_
