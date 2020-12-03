from matplotlib import pyplot as plt
import numpy as np
import cv2

def captureGrayImage(camera_serial, save_path, width=120, height=160,loop=20):
    if loop == 0:
        return 

    print(">>>> Looking for image ...")
    while not isImageStart(camera_serial):
        pass
    print(">> Found image !")
    
    image = np.zeros((height,width,1),dtype=np.uint8)
    for x in range(image.shape[1]):
        for y in range(image.shape[0]):
            temp = read1byte(camera_serial)
            image[y][image.shape[1]-x-1] = [temp]

    cv2.imwrite(save_path,image)

    print(">> Saved image ({})".format(loop-1))
    captureGrayImage(camera_serial, save_path,width,height,loop=loop-1)

def isImageStart(camera_serial,index=0):
    command = ['*','R','D','Y','*']
    if index < len(command):
        temp = read1byte(camera_serial)
        if ord(command[index]) == temp:
            return isImageStart(camera_serial,index+1)
        else:
            return False
    return True

def read1byte(camera_serial):
    temp = camera_serial.read(1)
    if (len(temp) != 1):
        return False
    return int.from_bytes(temp, "big") & 0xFF