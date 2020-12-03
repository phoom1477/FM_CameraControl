from matplotlib import pyplot as plt
import numpy as np
import cv2

def cropImage(image, offset=(0, 0), width=120, height=120):
    showImage = image.copy()
    
    # Rectangle crop
    cv2.line(showImage, offset, (offset[0]+width, offset[1]), (0, 255, 0), 1)
    cv2.line(showImage, offset, (offset[0], offset[1]+height), (0, 255, 0), 1)
    cv2.line(showImage, (offset[0]+width, offset[1]),
             (offset[0]+width, offset[1]+height), (0, 255, 0), 1)
    cv2.line(showImage, (offset[0], offset[1]+height),
             (offset[0]+width, offset[1]+height), (0, 255, 0), 1)

    plt.imshow(showImage)
    plt.show(block=False)
    plt.pause(1)
    plt.close()

    # Image numpy store data in [y,x] coordinate >> shape[0] is y, shape[1] is x
    return image[offset[1]:offset[1]+height, offset[0]:offset[0]+width]

def squareImageDecode(square_image, threshold):
    grayImage = cv2.cvtColor(square_image, cv2.COLOR_BGR2GRAY)

    # Image numpy store data in [y,x] coordinate >> shape[0] is y, shape[1] is x
    centerPoint_Q0 = [((grayImage.shape[0]//2)-(grayImage.shape[0]//4),
                       (grayImage.shape[1]//2)+(grayImage.shape[1]//4))]
    centerPoint_Q1 = [((grayImage.shape[0]//2)-(grayImage.shape[0]//4),
                       (grayImage.shape[1]//2)-(grayImage.shape[1]//4))]
    centerPoint_Q2 = [((grayImage.shape[0]//2)+(grayImage.shape[0]//4),
                       (grayImage.shape[1]//2)-(grayImage.shape[1]//4))]
    centerPoint_Q3 = [((grayImage.shape[0]//2)+(grayImage.shape[0]//4),
                       (grayImage.shape[1]//2)+(grayImage.shape[1]//4))]
    pointPosition = [centerPoint_Q0, centerPoint_Q1,
                     centerPoint_Q2, centerPoint_Q3]

    binaryCode = []
    quadrantData = [[], [], [], []]
    quadrantDataAVG = []
    for quadrant in range(4):
        average = 0

        for y in [-grayImage.shape[0]*5//100, grayImage.shape[0]*5//100]:
            for x in [-grayImage.shape[1]*5//100, grayImage.shape[1]*5//100]:

                print("Q[{}] = {} {}".format(quadrant, grayImage[pointPosition[quadrant][0][0]+y,
                                                                 pointPosition[quadrant][0][1]+x], (pointPosition[quadrant][0][0]+y, pointPosition[quadrant][0][1]+x)))
                quadrantData[quadrant].append(
                    [grayImage[pointPosition[quadrant][0][0]+y, pointPosition[quadrant][0][1]+x],(pointPosition[quadrant][0][0]+y, pointPosition[quadrant][0][1]+x)])

                average += grayImage[pointPosition[quadrant]
                                     [0][0]+y, pointPosition[quadrant][0][1]+x]

                # Marked point
                # [!!! Beware!!!  must mark after get the color value]
                cv2.drawMarker(
                    grayImage, (pointPosition[quadrant][0][1]+x, pointPosition[quadrant][0][0]+y), 150)

        average //= 4
        quadrantDataAVG.append(average)

        if average >= threshold:
            binaryCode.append(1)
        else:
            binaryCode.append(0)

        print("AVG Q[{}] = {}\n".format(quadrant, average))

    print("BinaryCode = {}\n".format(binaryCode))

    plt.imshow(grayImage)
    plt.show(block=False)
    plt.pause(1)
    plt.close()

    return binaryCode, quadrantData, quadrantDataAVG
