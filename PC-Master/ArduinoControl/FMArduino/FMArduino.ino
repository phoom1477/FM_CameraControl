#include<SoftwareSerial.h>
#include<String.h>
#include"FM_FSK.h"
/*
Frame Designed:
  Frame Size = 16 bits
  >> I-Frame:
  *-----2-----------2-------------------------------------8------------------------------------------4--------*
  |           |           |                                                                 |                 |
  |   0   0   |  No   No  | Payload Payload Payload Payload Payload Payload Payload Payload | Sum Sum Sum Sum |
  |           |           |                                                                 |                 |
  *-----------------------------------------------------------------------------------------------------------*
  >> S-Frame:
  *-----2-----------2-------------------------------------8------------------------------------------4--------*
  |           |           |                                                                 |                 |
  |   0   1   |  No   No  |    0       1       1       0       1       0       1       0    | Sum Sum Sum Sum |
  |           |           |                                                                 |                 |
  *-----------------------------------------------------------------------------------------------------------*
  >> U-Frame:
  *-----2-----------2-------------------------------------8------------------------------------------4--------*
  |           |           |                                                                 |                 |
  |   1   0   | Ctrl Ctrl |    1       0       0       1       1       0       1       1    | Sum Sum Sum Sum |
  |           |           |                                                                 |                 |
  *-----------------------------------------------------------------------------------------------------------*
*/
#define frameSize 16

/*I-FRAME*/
#define iFrame 0x0000             // iFrame(Type) 00 [0000 0000 0000 0000]
uint16_t iFrameNo = 0x0000;
uint16_t iPayload = 0x0000;

/*S-FRAME*/
#define sFrame 0x4000             // sFrame(Type) 01 [0100 0000 0000 0000]
uint16_t sFrameNo = 0x0000;
#define sPayload 0x06A0

/*U-FRAME*/
#define uFrame 0x8000             // uFrame(Type) 10 [1000 0000 0000 0000]
uint16_t uControl;
#define uPayload 0x09B0

/*Variable Sender Side*/
String inData = "";
uint16_t prevSendFrame = 0x0000;
uint16_t buffSendFrame = 0x0000;
bool canSend = true;
bool inCommunication = false;
bool recieverDeliveredData = false;
#define timeOutLimit 2000         // timeOutLimit should > [(frameSize/baudRate)*1000] 
long timer = 0;

/*Variable Reciever Side*/
String outData = "";
uint16_t recieveFrame = 0x0000;
bool haveFrameToDecrypt = false;
bool haveDataToDeliver = false;

/*Variable FM_FSK_Communication*/
FM_FSK fskFm;

/* Binary Memory */
String binaryLeft = "";
String binaryCenter = "";
String binaryRight = "";

void setup() {
  Serial.begin(115200);
  fskFm.begin(0x64,94.5,300,700,1100,1500);
  fskFm.setFrameSize(16);
  fskFm.setBaudRate(12);
  fskFm.setRSlope(25);
  
  Serial.flush();
  delay(1000);
  Serial.flush();
  Serial.println("===================================================");
  Serial.println("Command List : Select from below");
  Serial.println("Binary (\"xxxx\")   OR   Scan (\"S\")");
  Serial.println("===================================================");
}
void loop() {
  //===================== Sending Timer ===============//
  if (!canSend && (millis() - timer >= timeOutLimit)) {
    Serial.println(">> Sending Status : TimeOut ## Send again! ##");

    /*Send old frame*/
    fskFm.sendSignalFrame(prevSendFrame);
    timer = millis();
    canSend = false;
    inCommunication = true;
  }

  //===================== Send ========================//
  if (Serial.available() && inData.length() == 0) {
    /*Get serial input into inData*/
    inData = Serial.readStringUntil('\n');
    if(inData == binaryLeft){
      inData = "L";
    }
    else if(inData == binaryCenter){
      inData = "C";
    }
    else if(inData == binaryRight){
      inData = "R";
    }
    else if(inData == "S"){
      inData = "S";
    }
    else{
      Serial.println("===================================================");
      Serial.println("Input Wrong !!!");
      Serial.println("Please input again : ");
      Serial.println("Binary (\"xxxx\")   OR   Scan (\"S\")");
      Serial.println("===================================================");
      inData = "";
    }
  }
  
  if (canSend && inData.length() != 0) {
    /*Get first char from inData to send*/
    iPayload = (uint16_t)inData[0] << 4;                            // iFrame(1char payload) [0000 ???? ???? 0000]
    buffSendFrame = iFrame | iFrameNo | iPayload;                   // iFrame [00nn ???? ???? 0000]
    buffSendFrame = buffSendFrame | checkSum(buffSendFrame);        // iFrame [00nn ???? ???? ssss]
      
    inData.remove(0,1);
      
    /*Send new iFrame*/
    fskFm.sendSignalFrame(buffSendFrame);
    prevSendFrame = buffSendFrame;
    timer = millis();
    canSend = false;
    inCommunication = true;
    recieverDeliveredData = false;  
  }
  else if(canSend && inData.length() == 0 && inCommunication && !recieverDeliveredData){
    uControl = 0x0000;                                              // uFrame (control) [0000 0000 0000 0000]
    buffSendFrame = uFrame | uControl | uPayload;                   // uFrame [0000 1001 1011 0000]
    buffSendFrame = buffSendFrame | checkSum(buffSendFrame);        // uFrame [0000 1001 1011 ssss]

    /*Send new uFrame*/
    fskFm.sendSignalFrame(buffSendFrame);
    prevSendFrame = buffSendFrame;
    timer = millis();
    canSend = false;
    inCommunication = false;
    recieverDeliveredData = false;
  }

  //===================== Recieve ==================//
  haveFrameToDecrypt = fskFm.readSignalFrame();
  if(haveFrameToDecrypt){
    recieveFrame = fskFm.getRecieveFrame();
    
    uint16_t recieveFrameNo = (recieveFrame & 0x3000);
    uint16_t recieveFramePayload = (recieveFrame & 0x0FF0);

    /*Error Checking with checksum*/
    if(checkSum(recieveFrame) == 0x0000){
      /*recieveFrame is I_FRAME*/
      if((recieveFrame & 0xC000) == iFrame){
        if(recieveFrameNo == sFrameNo){
          Serial.println("Receive I_FRAME");
          outData += char(recieveFramePayload >> 4);
          haveDataToDeliver = true;
            
          sFrameNo = recieveFrameNo ^ 0x1000;                         // sFrame (frameNo) [00nn 0000 0000 0000]
          buffSendFrame = sFrame | sFrameNo | sPayload;               // sFrame [00nn 0110 1010 0000]
          buffSendFrame = buffSendFrame | checkSum(buffSendFrame);    // sFrame [00nn 0110 1010 ssss]
  
          /*Send sFrame with ((recieveFrameNo+1)%2)*/
          fskFm.sendSignalFrame(buffSendFrame);
        }else{
          sFrameNo = sFrameNo;     
          buffSendFrame = sFrame | sFrameNo | sPayload;               // sFrame [00nn 0110 1010 0000]
          buffSendFrame = buffSendFrame | checkSum(buffSendFrame);    // sFrame [00nn 0110 1010 ssss]
  
          /*Send sFrame with (recieveFrameNo)*/
          fskFm.sendSignalFrame(buffSendFrame);
        }
      }
      /*recieveFrame is S_FRAME*/
      else if((recieveFrame & 0xC000) == sFrame){
        Serial.println("Receive S_FRAME ACK");
        if(!canSend){
          if(recieveFrameNo == (iFrameNo & 0x3000) ^ 0x1000){
            iFrameNo = (iFrameNo & 0x3000) ^ 0x1000;
            canSend = true;
          }
        }
      }
      /*recieveFrame is U_FRAME*/
      else if((recieveFrame & 0xC000) == uFrame){
        uint16_t recieveFrameControl = recieveFrameNo;
        
        /*Recieve [00] Command for [Command Rx to Disconnect]*/
        if(recieveFrameControl == 0x0000){ 
          Serial.println("Receive U_FRAME [Command Rx to Disconnect]");
          if(haveDataToDeliver){
            deliverDataToUser();
            haveDataToDeliver = false;
          }
         
          uControl = 0x1000;                                        // uFrame (control) [0001 0000 0000 0000]
          buffSendFrame = uFrame | uControl | uPayload;             // uFrame [0001 1001 1011 0000]
          buffSendFrame = buffSendFrame | checkSum(buffSendFrame);  // uFrame [0001 1001 1011 ssss]
          
          /*Send new uFrame*/
          fskFm.sendSignalFrame(buffSendFrame);
          prevSendFrame = buffSendFrame;
          timer = millis();
          canSend = false;
        }
        /*Recieve [01] Command for [Rx Disconnected]*/
        else if(recieveFrameControl == 0x1000){
          Serial.println("Receive U_FRAME [Rx Disconnected]");
          recieverDeliveredData = true;

          uControl = 0x2000;                                        // uFrame (control) [0002 0000 0000 0000]
          buffSendFrame = uFrame | uControl | uPayload;             // uFrame [0002 1001 1011 0000]
          buffSendFrame = buffSendFrame | checkSum(buffSendFrame);  // uFrame [0002 1001 1011 ssss]
          
          /*Send new uFrame*/
          fskFm.sendSignalFrame(buffSendFrame);
          canSend = true;
        }
        /*Recieve [02] Command for [Disconnect Completed]*/
        else if(recieveFrameControl == 0x2000){
          Serial.println("Receive U_FRAME [Disconnect Completed]");
          
          canSend = true;
        }
      }
    }
    haveFrameToDecrypt = false;
    recieveFrame = 0x0000;
  }  
}

/*###################### Function #######################*/
uint16_t checkSum(uint16_t frame){
  uint8_t sum = 0x00;
  uint8_t carry = 0x00;
  
  /*Sum data with N:4*/
  for(int i=0;i<frameSize;i+=4){
    sum = sum + (uint8_t)((frame & 0x000F));
    frame >>= 4;
  }
  
  carry = sum & 0xF0;
  carry >>= 4;
  sum = sum & 0x0F;
  
  /*Add sum with carry*/
  sum = sum + carry;

  /*Take 1's complement*/
  sum = ~sum;

  /*return last 4 bits of 1's complement(sum)*/
  return sum & 0x000F;
}

void deliverDataToUser(){
  String coordinate[4][4] = {
    {"(24,84)","(24,96)","(36,84)","(36,96)"},
    {"(24,24)","(24,36)","(36,24)","(36,36)"},
    {"(84,24)","(84,36)","(96,24)","(96,36)"},
    {"(84,84)","(84,96)","(96,84)","(96,96)"}
  };
  
  if(outData[0] == 'S'){
    binaryLeft = outData.substring(1,5);
    binaryCenter = outData.substring(5,9);
    binaryRight = outData.substring(9,13); 
  }

  if(outData[0] == 'L'){
    String checkCode = outData.substring(61,65);
    Serial.println("===================================================");
    if(checkCode != binaryLeft){
      Serial.print("********* Picture Changed : ");
      Serial.print(binaryLeft + " to ");
      Serial.println(checkCode + " *********");
      if(binaryCenter == checkCode){
        binaryCenter = "";
      }
      if(binaryRight == checkCode){
        binaryRight = "";
      }
      binaryLeft = checkCode;
    }
    Serial.print("            Left Picture -> ");
    Serial.println(binaryLeft);
    
    int n = 0;
    for(int i=1;i<=4;i++){
      for(int j=1;j<=4;j++){
        Serial.print("Quadrant[" + String(i) + "] : ");
        Serial.print(outData.substring(((n*3)+1),((n*3)+4)));
        Serial.println("  " + coordinate[i-1][j-1]);
        n+=1;
      }
      Serial.print("Average of Quadrant[");
      Serial.print(String(i) + "] : ");
      Serial.println(outData.substring(((n*3)+1),((n*3)+4)));
      n+=1;
    }
    Serial.println("===================================================");
  }

  if(outData[0] == 'C'){
    String checkCode = outData.substring(61,65);
    Serial.println("===================================================");
    if(checkCode != binaryCenter){
      Serial.print("********* Picture Changed : ");
      Serial.print(binaryCenter + " to ");
      Serial.println(checkCode + " *********");
      if(binaryLeft == checkCode){
        binaryLeft = "";
      }
      if(binaryRight == checkCode){
        binaryRight = "";
      }
      binaryCenter = checkCode;
    }
    Serial.print("            Center Picture -> ");
    Serial.println(binaryCenter);
    
    int n = 0;
    for(int i=1;i<=4;i++){
      for(int j=1;j<=4;j++){
        Serial.print("Quadrant[" + String(i) + "] : ");
        Serial.print(outData.substring(((n*3)+1),((n*3)+4)));
        Serial.println("  " + coordinate[i-1][j-1]);
        n+=1;
      }
      Serial.print("Average of Quadrant[");
      Serial.print(String(i) + "] : ");
      Serial.println(outData.substring(((n*3)+1),((n*3)+4)));
      n+=1;
    }
    Serial.println("===================================================");
  }

  if(outData[0] == 'R'){
    String checkCode = outData.substring(61,65);
    Serial.println("===================================================");
    if(checkCode != binaryRight){
      Serial.print("********* Picture Changed : ");
      Serial.print(binaryRight + " to ");
      Serial.println(checkCode + " *********");
      if(binaryCenter == checkCode){
        binaryCenter = "";
      }
      if(binaryLeft == checkCode){
        binaryLeft = "";
      }
      binaryRight = checkCode;
    }
    Serial.print("            Right Picture -> ");
    Serial.println(binaryRight);
    
    int n = 0;
    for(int i=1;i<=4;i++){
      for(int j=1;j<=4;j++){
        Serial.print("Quadrant[" + String(i) + "] : ");
        Serial.print(outData.substring(((n*3)+1),((n*3)+4)));
        Serial.println("  " + coordinate[i-1][j-1]);
        n+=1;
      }
      Serial.print("Average of Quadrant[");
      Serial.print(String(i) + "] : ");
      Serial.println(outData.substring(((n*3)+1),((n*3)+4)));
      n+=1;
    }
    Serial.println("===================================================");
  }

  Serial.println("===================================================");
  Serial.println("                Binary Code");
  Serial.println("            Choose your choice");
  Serial.println(" Left   Picture : \"" + binaryLeft + "\"");
  Serial.println(" Center Picture : \"" + binaryCenter + "\"");
  Serial.println(" Right  Picture : \"" + binaryRight + "\"");
  Serial.println("===================================================");
  
  outData = "";
}
