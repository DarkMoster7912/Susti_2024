from gpiolib import gpio
from comunication import ser
from multiprocessing import *
import RPi.GPIO as GPIO
import time
import os
import signal

procList = [None]
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(22,GPIO.OUT)
GPIO.setup(21,GPIO.OUT)
GPIO.setup(10,GPIO.OUT)
GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_UP)
def lack(port):
        GPIO.remove_event_detect(23)
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
        try:
            match Ser.msgs[1]:
                case 'r':
                    Ser.victim(False,2)
                case 'y':
                    Ser.victim(False,1)
                case 'g':
                    Ser.victim(False,0)
                case _:
                    print('mess cam0:' + str(Ser.msgs[1]))
            match Ser.msgs[2]:
                case 'r':
                    Ser.victim(True,2)
                case 'y':
                    Ser.victim(True,1)
                case 'g':
                    Ser.victim(True,0)
                case _:
                    print('mess cam1:' + str(Ser.msgs[2]))
            Ser.msgs[1]=Ser.msgs[2]=None
        except Exception as e:
            print(e)
        time.sleep(.1)
procList[0] = Process(target=ComProFunc, args=(lack,True))
procList[0].start()
procList[0].join()
GPIO.wait_for_edge(23, GPIO.RISING)
GPIO.remove_event_detect(23)
GPIO.wait_for_edge(23, GPIO.FALLING)
GPIO.remove_event_detect(23)
print('restart eseguito')
while True:           
    procList[0] = Process(target=ComProFunc, args=(lack,False))
    procList[0].start()
    procList[0].join()
    GPIO.wait_for_edge(23, GPIO.RISING)
    GPIO.remove_event_detect(23)
    GPIO.wait_for_edge(23, GPIO.FALLING)
    GPIO.remove_event_detect(23)
    GPIO.wait_for_edge(23, GPIO.FALLING)
    GPIO.remove_event_detect(23)
    
    


