import time
import RPi.GPIO as GPIO
import threading
def wait_for_press(pin):
    exitt= 0
    while True:
        if exitt:
            break
        oldtime = 0
        while GPIO.input(pin):
            if not oldtime:
                oldtime = time.monotonic_ns()
            elif (time.monotonic_ns()-oldtime)>100000000:
                exitt = True
    while True:
        if exitt:
            break
        oldtime = 0
        while not GPIO.input(pin):
            if not oldtime:
                oldtime = time.monotonic_ns()
            elif (time.monotonic_ns()-oldtime)>100000000:
                exitt = True
                
class gpio:
    
    def __init__(self, lack):
        
        GPIO.setmode(GPIO.BCM)
        GPIO.setwarnings(False)
        GPIO.setup(22,GPIO.OUT)
        GPIO.setup(21,GPIO.OUT)
        GPIO.setup(10,GPIO.OUT)
        GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        GPIO.output(21,GPIO.LOW)
        GPIO.output(22,GPIO.HIGH)
        GPIO.output(10,GPIO.HIGH)
        self.lackThread=self.Lack(lack)
        
    def readyToStart(self):
        wait_for_press(23)
        
    def setLack(self):
       # GPIO.add_event_detect(23, GPIO.FALLING, callback=self.lack, bouncetime=200)
       self.lackThread.start()
        
        
    def lackMessages(self,restart):
        if restart:
            GPIO.output(10,GPIO.HIGH)
        else:
            GPIO.output(21,GPIO.LOW)
            GPIO.output(22,GPIO.LOW)
            GPIO.output(10,GPIO.LOW)
    class Lack(threading.Thread):
        def __init__(self,_lack):
            threading.Thread.__init__(self)
            self._lack = _lack
        def run(self):
            wait_for_press(23)
            self._lack()
            
if __name__ == '__main__':
    def lack():
        print('lack')
        time.sleep(1)
        wait_for_press(23)
        print('restart eseguito')
    gpio = gpio(lack)
    #gpio.readyToStart()
    #print('ciao')
    #gpio.setLack()
    #while True:
        
        #time.sleep(1e6)
        
        
    #print('high')
     #   GPIO.output(4,GPIO.HIGH)
      #  time.sleep(2)
       # print('low')
    GPIO.output(21,GPIO.LOW)
    time.sleep(.5)
    GPIO.output(21,GPIO.HIGH)
        
        










