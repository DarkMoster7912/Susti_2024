from gpiolib import gpio,wait_for_press
from comunication import ser
from multiprocessing import *
import RPi.GPIO as GPIO
import time
import os
import signal


procList = [None]
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.cleanup()
GPIO.setup(22,GPIO.OUT)
GPIO.setup(21,GPIO.OUT)
GPIO.setup(10,GPIO.OUT)
GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_UP)
def lack():
        print('lack')
        gpio(lack).lackMessages(False)
        os.kill(os.getpid(),signal.SIGTERM)
            
def ComProFunc(Lack,first):
    Gpio = gpio(Lack)
    Ser = ser()
    #Ser.waitFullLenghtMessage()
    if first:
        Gpio.readyToStart()
    Gpio.setLack()
    Ser.start()
    Ser.write('g1')
    print('done')
    while True:
        if Ser.msgs[1]:
            print('cam sx: '+Ser.msgs[1])
        if Ser.msgs[2]:
            print('cam dx: '+Ser.msgs[2])
        print(Ser.msgs[0])    
        if(int(Ser.msgs[0][3])):
            match Ser.msgs[2]:
                case 'r':
                    Ser.victim(False,2)
                    while(int(Ser.msgs[0][3])):
                        print(int(Ser.msgs[0]))
                    Ser.msgs[2]=Ser.msgs[1]=None
                case 'y':
                    Ser.victim(False,1)

                    while(int(Ser.msgs[0][3])):
                        print(int(Ser.msgs[0]))

                    Ser.msgs[2]=Ser.msgs[1]=None
                case 'g':
                    Ser.victim(False,0)
                    while(int(Ser.msgs[0][3])):
                        print(int(Ser.msgs[0]))
                    Ser.msgs[2]=Ser.msgs[1]=None
            match Ser.msgs[1]:
                case 'r':
                    Ser.victim(True,2)
                    while(int(Ser.msgs[0][3])):
                        print(int(Ser.msgs[0]))
                    Ser.msgs[2]=Ser.msgs[1]=None
                case 'y':
                    Ser.victim(True,1)
                    
                    while(int(Ser.msgs[0][3])):
                        print(int(Ser.msgs[0]))
                    Ser.msgs[2]=Ser.msgs[1]=None
                case 'g':
                    Ser.victim(True,0)
                    while(int(Ser.msgs[0][3])):
                        print(int(Ser.msgs[0]))
                    Ser.msgs[2]=Ser.msgs[1]=None
        time.sleep(.1)
procList[0] = Process(target=ComProFunc, args=(lack,True))
procList[0].start()
procList[0].join()
wait_for_press(23)
print('restart eseguito')
while True:           
    procList[0] = Process(target=ComProFunc, args=(lack,False))
    procList[0].start()
    procList[0].join()
    wait_for_press(23)
    






