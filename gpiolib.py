import time
import RPi.GPIO as GPIO
class gpio:
    
    def __init__(self, lack):
        self.lack=lack
        GPIO.setmode(GPIO.BCM)
        GPIO.setwarnings(False)
        GPIO.setup(22,GPIO.OUT)
        GPIO.setup(21,GPIO.OUT)
        GPIO.setup(10,GPIO.OUT)
        GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_UP)
        GPIO.output(21,GPIO.LOW)
        GPIO.output(22,GPIO.HIGH)
        GPIO.output(10,GPIO.HIGH)
    def readyToStart(self):
        GPIO.wait_for_edge(23, GPIO.FALLING)
        GPIO.remove_event_detect(23)
        time.sleep(1)
        
    def setLack(self):
        GPIO.add_event_detect(23, GPIO.FALLING, callback=self.lack, bouncetime=100)
        
        
    def lackMessages(self,restart):
        if restart:
            GPIO.output(10,GPIO.HIGH)
        else:
            GPIO.output(21,GPIO.LOW)
            GPIO.output(22,GPIO.LOW)
            GPIO.output(10,GPIO.LOW)
            
if __name__ == '__main__':
    def lack(port):
        GPIO.remove_event_detect(23)
        print('lack')
        time.sleep(1)
        GPIO.wait_for_edge(23, GPIO.FALLING)
        GPIO.remove_event_detect(23)
        print('restart eseguito')
        GPIO.add_event_detect(23, GPIO.FALLING, 
                callback=lack, bouncetime=100)
        time.sleep(1)
    gpio = gpio(lack)
    '''while True:
        
        #time.sleep(10)
        
        
        print('high')
        GPIO.output(4,GPIO.HIGH)
        time.sleep(2)
        print('low')'''
    GPIO.output(21,GPIO.LOW)
    time.sleep(1)
    GPIO.output(21,GPIO.HIGH)
        
        
