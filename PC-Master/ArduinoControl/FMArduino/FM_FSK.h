/*
  4-FSK with FM 
*/
#ifndef FM_FSK_H
#define FM_FSK_H

#include <Arduino.h>
#include <Adafruit_MCP4725.h>
#include <TEA5767.h>
#include <Wire.h>

/* cbi this for increase analogRead Speed */
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define defaultFreq 1700                  // DAC speed [Can generate 1700 waveCycle/sec (Hz)] 
#define zetaLength 4                      // Number of zeta in 1 cycle

class FM_FSK {
  private:
    Adafruit_MCP4725 DAC;
    TEA5767 radio = TEA5767();

    /*Both Side*/
    int frameSize = 16;
    int baudRate = 1;
   
    /*Sender Side*/
    long freq0, freq1, freq2, freq3;
    long delay0, delay1, delay2, delay3;

    int zeta[zetaLength];
    float sine[zetaLength];
    uint16_t sine_dac[zetaLength];

    /*Reciever Side*/
    long numCycle0min,numCycle1min,numCycle2min,numCycle3min;
    long numCycle0max,numCycle1max,numCycle2max,numCycle3max;

    int checked = false;
    int rSlope=10;
    int maxAnalog = 0;
    int prevAnalog = 0;
    
    long baudTimer = 0;

    int countBaud = 0;
    uint16_t recieveFrame = 0x0000;
    uint16_t recieveByte = 0x0000;

    bool startedCount = false;
    int count = 0;
    
    void initialDAC(uint8_t dacAddr);
    void initialTEA(float fmFreq);
    void initialThreshold();

    void generateWaveInBaud(int freq, int delayPerZeta);

  public:
    FM_FSK();
    ~FM_FSK();
    
    void begin(uint8_t dacAddress,float fmFreq,int freq0, int freq1, int freq2, int freq3);
    void setFrameSize(int frameSize);
    void setBaudRate(int baudRate);
    void setRSlope(int rSlope);
    
    void sendSignalFrame(uint16_t frame);
    bool readSignalFrame();
    uint16_t getRecieveFrame();
};

#endif
