import serial
import time
import threading
from multiprocessing import Queue
from random import randint
import RPi.GPIO as GPIO

class ser:
    def __init__(self):
        #GPIO.output(21,GPIO.HIGH)
        self.msgs=[None,None,None]
        self.addresses = ['/dev/ttyAMA0','/dev/ttyACM0','/dev/ttyACM1']
        self.CamCons= []
        self.initCAM(0)
        self.initCAM(1)
        self.initPICO()
        self.GetMsgs(self,self.PicoCon).start()
        self.GetMsgs(self,self.CamCons[0]).start()
        self.GetMsgs(self,self.CamCons[1]).start()
    def initPICO(self):
        GPIO.setup(21,GPIO.OUT)
        self.PicoCon=serial.Serial(self.addresses[0], 1000000, timeout = 0)
        print('working')        
        self.PicoCon.reset_input_buffer()
        #self.write('r')
        GPIO.output(21,GPIO.LOW)
        time.sleep(0.5)
        GPIO.output(21,GPIO.HIGH)
        oldTime = time.monotonic()
        print('inizio')
        i=6
        while self.msgs[0]!= 'xd':
            if((i+0.05)>(time.monotonic()-oldTime)>(i-0.05)):
                print('reset')
                GPIO.output(21,GPIO.LOW)
                time.sleep(0.5)
                GPIO.output(21,GPIO.HIGH)
                i+=6
            try:
                self.msgs[0]=self.PicoCon.readline().decode('utf-8').rstrip()
            except UnicodeDecodeError:
                pass#self.msgs[0]='xd'   ''' 
        self.msgs[0]=self.PicoCon.readline().decode('utf-8').rstrip()
        
        self.PicoCon.reset_input_buffer()
        print('pico ok')
    def initCAM(self,n):
        self.CamCons.insert(n,serial.Serial(self.addresses[n+1],1000000, timeout = 0))
        print('working')        
        self.CamCons[n].reset_input_buffer()
        #self.write('r')
        oldTime = time.monotonic()
        
        
    def restartDevices(self):
        for cam in self.CamCons:
            cam.reset_input_buffer()
        self.initPICO()
    def write(self, comType):
        self.PicoCon.write(bytes(str(comType),'utf-8'))
    def read(self, device):
        
        if device == self.PicoCon:
            
            msg='g'
            
            while msg[-1]!='e':
                try:
                    msg=msg + device.readline().decode('utf-8').rstrip()
                except Exception as e:
                    print(e)
                
            self.msgs[0]=msg[1:-1]
        else:
            msg=0
            
            while not msg:
                try:
                    msg=device.readline().decode('utf-8').rstrip()
                except:
                    pass
            self.msgs[self.addresses.index(device.port)]=msg
    
    
    def readAll(self):
        msgs = ['g',0,0]
        while True:
            try:
                msgs[0]=msgs[0] + PicoCon.readline().decode('utf-8').rstrip()
            except Exception as e:
                pass
            try:
                msgs[1]=CamCons[0].readline().decode('utf-8').rstrip()
            except:
                pass
            try:
                msgs[2]=CamCons[1].readline().decode('utf-8').rstrip()
            except:
                pass
            if msgs[0].count
            
    def victim(self,senso,nk):
        self.write('g0')
        time.sleep(.3)
        self.write('v'+('s' if senso else 'd')+ str(nk))
        for i in range(5):
            GPIO.output(10,GPIO.LOW)
            time.sleep(.5)
            GPIO.output(10,GPIO.HIGH)
            time.sleep(.5)
        self.write('g1')
    def waitFullLenghtMessage(self):
        while True:
            self.read(self.PicoCon)
            try:
                self.msgs[0][10]
                break
            except IndexError:
                pass
    def start(self):
        self.write('s')
        time.sleep(.3)
        self.write('g1')
    class GetMsgs(threading.Thread):
        def __init__(self,_class,device):
            threading.Thread.__init__(self)
            self._class = _class
            self.device = device
        def run(self):
            while True:
                self._class.read(self.device)
                time.sleep(0.1)
if __name__ == '__main__':
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(10,GPIO.OUT)
    GPIO.setup(21,GPIO.OUT)
    GPIO.output(21, GPIO.LOW)
    GPIO.setup(22,GPIO.OUT)
    GPIO.output(22, GPIO.HIGH)
    Ser=ser()
    Ser.write('s')
    print('done')
    direction = 0
    oldtime = 0
    
    Ser.waitFullLenghtMessage()
    
    while True:
        Ser.read(Ser.CamCons[0])
        while (not int(Ser.msgs[0][5])):
               Ser.read(Ser.PicoCon)
        if int(Ser.msgs[0][5]):
            
            match Ser.msgs[1]:
                case 'r':
                    Ser.victim(False,2)
                case 'y':
                    Ser.victim(False,1)
                case 'g':
                    Ser.victim(False,0)
            
            while (int(Ser.msgs[0][5])):
                Ser.read(Ser.PicoCon)
            Ser.CamCons[0].reset_input_buffer()
        '''Ser.read(Ser.PicoCon)
            print(round(float(Ser.msgs[0][(Ser.msgs[0].index('d')+1):])/90)%4)
            Ser.write('g0')
            time.sleep(5)
            Ser.write('g1')
            time.sleep(10)
            try:
                direction = int(Ser.msgs[0][7])
                if(int(Ser.msgs[0][5])):
                    
                    if(not centro):
                        centro = True
                        if(int(Ser.msgs[0][0]) == 0):
                            direction = (direction+1)%4
                            
                        elif(int(Ser.msgs[0][2]) == 0):
                            direction = (direction-1)%4
                        Ser.write('d'+ str(direction))
                else:
                    centro=False
            except IndexError:
                pass'''
        
            
            
            
      
      
