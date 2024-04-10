import serial,time
import RPi.GPIO as GPIO


GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(21,GPIO.OUT)
PicoCon=serial.Serial('/dev/ttyAMA0', 1000000, timeout = 0)
PicoCon.reset_input_buffer()
GPIO.output(21,GPIO.LOW)
time.sleep(0.5)
GPIO.output(21,GPIO.HIGH)

while True:
    try:
        print(PicoCon.readline().decode('utf-8').rstrip())
    except UnicodeDecodeError:
        pass
    time.sleep(.1)
    