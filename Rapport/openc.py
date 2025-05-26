import cv2
import numpy as np

# Assuming 'image' is already defined (BGR format)
image =cv2.imread("rainbow.jpg")
height, width = image.shape[:2]
image = cv2.flip(image, 0)
cv2.imwrite("inverted.jpg",image)
# Convert to HSV
imageHSV = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
cv2.imwrite("hsv.jpg",imageHSV  )
# Define your HSV bounds (example values, adjust as needed)
lowerbound_red = np.array([0, 100, 100])
higherbound_red = np.array([10, 255, 255])
lowerbound2_red = np.array([160, 100, 100])
higherbound2_red = np.array([179, 255, 255])
lowerbound_blue = np.array([100, 150, 0])
higherbound_blue = np.array([140, 255, 255])

# Create masks
mask_red = cv2.inRange(imageHSV, lowerbound_red, higherbound_red)
maskRed2 = cv2.inRange(imageHSV, lowerbound2_red, higherbound2_red)
mask_red = cv2.bitwise_or(mask_red, maskRed2)
mask_blue = cv2.inRange(imageHSV, lowerbound_blue, higherbound_blue)
maskApplied = cv2.bitwise_and(image, image, mask=cv2.bitwise_or(mask_red,mask_blue))
black  = np.zeros((height, width, 3), dtype=np.uint8)
momred = cv2.moments(mask_red)
momblue = cv2.moments(mask_blue)
baroRed =( int(momred["m10"] // momred["m00"]), int(momred["m01"]//momred["m00"])) 
baroBlue =( int(momblue["m10"] // momblue["m00"]), int( momblue["m01"]//momblue["m00"])) 

print(f"{width=}")
print(f"{baroBlue=}")
Pointed = cv2.circle(black,baroRed,15,(0,0,255),-1)
Pointed = cv2.circle(black,baroBlue,15,(255,0,0),-1)
cv2.imwrite("mask.jpg",maskApplied)
cv2.imwrite("bary.jpg",Pointed)