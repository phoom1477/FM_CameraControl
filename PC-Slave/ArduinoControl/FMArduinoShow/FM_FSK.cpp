#include "FM_FSK.h"

/*----------------------------- Private ------------------------------*/
// Initialization
void FM_FSK::initialDAC(uint8_t dacAddress) {
  
  this->DAC.begin(dacAddress);

  /*Get (deg) zeta*/
  for (int i = 0; i < zetaLength; i++) {
    this->zeta[i] = 360 / zetaLength * i;
  }

  /*Get deg of zeta*/
  for (int i = 0; i < zetaLength; i++)
  {
    float radianI = this->zeta[i] * PI / 180;
    this->sine[i] = sin(radianI);

    /*Map (radiant) zeta with DAC value*/
    this->sine_dac[i] = (uint16_t)map(this->sine[i] * 1000, -1000, 1000, 0, 4095);
  }
}
void FM_FSK::initialTEA(float fmFreq){
  Wire.begin();
  Wire.setClock(400000UL);

  radio.set_frequency(fmFreq);

  sbi(ADCSRA, ADPS2) ;                     
  cbi(ADCSRA, ADPS1) ;
  cbi(ADCSRA, ADPS0) ;
}
void FM_FSK::initialThreshold(){
  int numCycle0 = this->freq0/this->baudRate;
  int numCycle1 = this->freq1/this->baudRate;
  int numCycle2 = this->freq2/this->baudRate;
  int numCycle3 = this->freq3/this->baudRate;
  
  this->numCycle0min = numCycle0 - (((numCycle0+numCycle1)/2) - numCycle0);
  this->numCycle1min = (numCycle0+numCycle1)/2;
  this->numCycle2min = (numCycle1+numCycle2)/2;
  this->numCycle3min = (numCycle2+numCycle3)/2;
  
  this->numCycle0max = (numCycle0+numCycle1)/2;
  this->numCycle1max = (numCycle1+numCycle2)/2;
  this->numCycle2max = (numCycle2+numCycle3)/2;
  this->numCycle3max = numCycle3 + (numCycle3-((numCycle2+numCycle3)/2));
}

// Method
void FM_FSK::generateWaveInBaud(int freq, int delayPerZeta) {
  Serial.print(freq/this->baudRate);
  Serial.print("] ");
  
  for (int cycle = 0; cycle < freq/this->baudRate; cycle++ ) {
    for (int zeta = 0; zeta < zetaLength; zeta++ ) {
      DAC.setVoltage(this->sine_dac[zeta], false);
      delayMicroseconds(delayPerZeta);
    }
  }
}

/*----------------------------- Public -------------------------------*/
// Constructor 
FM_FSK::FM_FSK() {
}
// Destructor
FM_FSK::~FM_FSK() {
}

// Method
void FM_FSK::begin(uint8_t dacAddress,float fmFreq,int freq0, int freq1, int freq2, int freq3) {
  this->initialDAC(dacAddress);
  this->initialTEA(fmFreq);

  /*Send*/
  this->freq0 = freq0;
  this->freq1 = freq1;
  this->freq2 = freq2;
  this->freq3 = freq3;

  this->delay0 = (1000000 / freq0 - 1000000 / defaultFreq) / zetaLength;
  this->delay1 = (1000000 / freq1 - 1000000 / defaultFreq) / zetaLength;
  this->delay2 = (1000000 / freq2 - 1000000 / defaultFreq) / zetaLength;
  this->delay3 = (1000000 / freq3 - 1000000 / defaultFreq) / zetaLength;

  this->initialThreshold();
}

void FM_FSK::setFrameSize(int frameSize){
  this->frameSize = frameSize;
}
void FM_FSK::setBaudRate(int baudRate){
  this->baudRate = baudRate;
  this->initialThreshold();
}
void FM_FSK::setRSlope(int rSlope){
  this->rSlope = rSlope;
}
void FM_FSK::sendSignalFrame(uint16_t frame) {
  Serial.print("Send Frame [LSB->MSB] : ");
  for (int i = 0; i < this->frameSize; i += 2) {
    uint16_t temp = frame & 0x0003;

    Serial.print(temp);
    Serial.print("->[");

    if (temp == 0x0000) {
      this->generateWaveInBaud(this->freq0, this->delay0);
    }
    else if (temp == 0x0001) {
      this->generateWaveInBaud(this->freq1, this->delay1);
    }
    else if (temp == 0x0002) {
      this->generateWaveInBaud(this->freq2, this->delay2);
    }
    else if (temp == 0x0003) {
      this->generateWaveInBaud(this->freq3, this->delay3);
    }
    frame >>= 2;
  }
  delayMicroseconds(20000);
  Serial.println();
}

bool FM_FSK::readSignalFrame(){
  int tmpAnalog = analogRead(A0);
  
  if ( tmpAnalog - this->prevAnalog > this->rSlope && this->checked == false) {
    this->maxAnalog = 0;
    this->checked = true;                    
  }

  if (tmpAnalog > this->maxAnalog) {
    this->maxAnalog = tmpAnalog;        
  }

  if (this->maxAnalog - tmpAnalog > this->rSlope) {   
    if (this->checked == true) {
      if(!this->startedCount){
        this->startedCount = true;
        this->baudTimer =  micros();
      }
      this->count += 1;
    }
    this->checked = false;                          
  }

  if(this->startedCount && (micros() - this->baudTimer >= 1000000/this->baudRate)){
    if(this->numCycle0min <= this->count && this->count <= this->numCycle0max){
      this->recieveByte = 0x0000;
    }
    else if(this->numCycle1min <= this->count && this->count <= this->numCycle1max){
      this->recieveByte = 0x0001;
    }
    else if(this->numCycle2min <= this->count && this->count<= this->numCycle2max){
      this->recieveByte = 0x0002;
    }
    else if(this->numCycle3min <= this->count && this->count <= this->numCycle3max){
      this->recieveByte = 0x0003;
    }
    else{
      this->recieveByte = 0xFFFF;
    }
    
    if(this->recieveByte == 0xFFFF && this->countBaud == 0){
      Serial.print(this->countBaud);
      Serial.print("[");
      Serial.print(this->count);
      Serial.print("]->");
      Serial.print(this->recieveByte);
      Serial.print("-----------------------> Drop");
      Serial.println();
      this->startedCount = false;
      this->maxAnalog = 0;
    }
    else{
      Serial.print(this->countBaud);
      Serial.print("[");
      Serial.print(this->count);
      Serial.print("]->");
      Serial.print(this->recieveByte);
      Serial.println();
      
      this->recieveByte <<= 2*this->countBaud;
      this->recieveFrame =  this->recieveFrame | this->recieveByte;
      this->countBaud++;  
    }
    
    this->baudTimer = micros();
    this->count = 0;
    
    if(this->countBaud == (this->frameSize/2)){
      Serial.println("--------------");
      this->startedCount = false;
      this->countBaud = 0;
      this->maxAnalog = 0;
      return true;
    }   
  }
  this->prevAnalog = tmpAnalog;
  return false;
}
uint16_t FM_FSK::getRecieveFrame(){
  uint16_t tmp = this->recieveFrame;
  this->recieveFrame = 0x0000;

  return tmp;
}
