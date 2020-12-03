from module import cropImage, squareImageDecode, captureGrayImage
import os 
import serial
import time
import cv2

fmArduinoSerial = serial.Serial('com12', 115200)
cameraArduinoSerial = serial.Serial('com7', 1000000,timeout=1)
servoArduinoSerial = serial.Serial('com13', 115200)

### Function ###
def processOneImage(direction):

    servoArduinoSerial.write(direction.encode('utf-8'))

    while True:

        res = servoArduinoSerial.read_until().decode('utf-8')
        
        if res == "turned\r\n":
            print(">>>> ... Processing ...\n")
            captureGrayImage(cameraArduinoSerial,'./captureImage/{}/image.bmp'.format(direction))
        
            image = cv2.imread('./captureImage/{}/image.bmp'.format(direction))
            image = cropImage(image, (0, 30), height=120, width=120)

            return squareImageDecode(image,80)

def porcessAllImage():
    
    binaryCodeList = []
    binaryCodeList.append(processOneImage("left")[0])
    binaryCodeList.append(processOneImage("center")[0])
    binaryCodeList.append(processOneImage("right")[0])

    return binaryCodeList
    
### Main ###
os.system("pip install opencv-python")
os.system("pip install pyserial")

while True:
    if fmArduinoSerial.readable():
        command = fmArduinoSerial.read_until().decode('utf-8')
        print("<< Recieve Command :",command,end='')

        time.sleep(1)

        # Control Camera to scan
        if command == "S\r\n":
            binaryCodeList = porcessAllImage()

            for i in range(len(binaryCodeList)):
                binaryCodeList[i] = [str(item) for item in binaryCodeList[i]]

            stringToSend = "S"
            stringToSend += "".join(binaryCodeList[0])
            stringToSend += "".join(binaryCodeList[1])
            stringToSend += "".join(binaryCodeList[2])

            print(">> Message to PC2 :",stringToSend);

            fmArduinoSerial.write(stringToSend.encode('utf-8'))

        # Control Camera to (-45 deg) for take a shot
        if command == "L\r\n":
            binaryCode,quadrantData, quadrantDataAVG = processOneImage("left")

            binaryCode = [str(item) for item in binaryCode]

            stringToSend = "L"
            for i in range(4):
                for j in range(4):
                    stringToSend += str(quadrantData[i][j][0]).zfill(3)
                stringToSend += str(quadrantDataAVG[i]).zfill(3)
            
            stringToSend += "".join(binaryCode)

            print(">> Message to PC2 :",stringToSend);

            fmArduinoSerial.write(stringToSend.encode('utf-8'))

        # Control Camera to (0 deg) for take a shot
        if command == "C\r\n":
            binaryCode,quadrantData, quadrantDataAVG = processOneImage("center")

            binaryCode = [str(item) for item in binaryCode]

            stringToSend = "C"
            for i in range(4):
                for j in range(4):
                    stringToSend += str(quadrantData[i][j][0]).zfill(3)
                stringToSend += str(quadrantDataAVG[i]).zfill(3)
            
            stringToSend += "".join(binaryCode)
            
            print(">> Message to PC2 :",stringToSend);

            fmArduinoSerial.write(stringToSend.encode('utf-8'))

        # Control Camera to (45 deg) for take a shot
        if command == "R\r\n":
            binaryCode,quadrantData, quadrantDataAVG = processOneImage("right")

            binaryCode = [str(item) for item in binaryCode]

            stringToSend = "R"
            for i in range(4):
                for j in range(4):
                    stringToSend += str(quadrantData[i][j][0]).zfill(3)
                stringToSend += str(quadrantDataAVG[i]).zfill(3)
            
            stringToSend += "".join(binaryCode)
            
            print(">> Message to PC2 :",stringToSend);

            fmArduinoSerial.write(stringToSend.encode('utf-8'))

### Main ### [For test via Terminal] 
# os.system("pip install opencv-python")
# os.system("pip install pyserial")
# while True:
#     command = input("Input Command :: ")
#     time.sleep(1)

#     # Control Camera to scan
#     if command == "scan":
#         binaryCodeList = porcessAllImage()

#         stringToSend = ""
#         stringToSend += "(-45 Deg) Binary Code is {}\n".format(binaryCodeList[0])
#         stringToSend += "(  0 Deg) Binary Code is {}\n".format(binaryCodeList[1])
#         stringToSend += "( 45 Deg) Binary Code is {}\n".format(binaryCodeList[2])

#         print(stringToSend)

#     # Control Camera to -45 deg for take a shot
#     if command == "left":
#         binaryCode,quadrantData, quadrantDataAVG = processOneImage("left")

#         stringToSend = ""
#         stringToSend += "(  0 Deg) Binary Code is {}\n".format(binaryCode)
#         for i in range(4):
#             for j in range(4):
#                 stringToSend += "Q[{}]-{} value is {} {}\n".format(i,j,quadrantData[i][j][0],quadrantData[i][j][1])
#             stringToSend += ">> AVG Q[{}] value is {}\n".format(i,quadrantDataAVG[i])

#         print(stringToSend)

#     # Control Camera to 0 deg for take a shot
#     if command == "center":
#         binaryCode,quadrantData, quadrantDataAVG = processOneImage("center")

#         stringToSend = ""
#         stringToSend += "(  0 Deg) Binary Code is {}\n".format(binaryCode)
#         for i in range(4):
#             for j in range(4):
#                 stringToSend += "Q[{}]-{} value is {} {}\n".format(i,j,quadrantData[i][j][0],quadrantData[i][j][1])
#             stringToSend += ">> AVG Q[{}] value is {}\n".format(i,quadrantDataAVG[i])

#         print(stringToSend)

#     # Control Camera to 45 deg for take a shot
#     if command == "right":
#         binaryCode,quadrantData, quadrantDataAVG = processOneImage("right")

#         stringToSend = ""
#         stringToSend += "(  0 Deg) Binary Code is {}\n".format(binaryCode)
#         for i in range(4):
#             for j in range(4):
#                 stringToSend += "Q[{}]-{} value is {} {}\n".format(i,j,quadrantData[i][j][0],quadrantData[i][j][1])
#             stringToSend += ">> AVG Q[{}] value is {}\n".format(i,quadrantDataAVG[i])

#         print(stringToSend)