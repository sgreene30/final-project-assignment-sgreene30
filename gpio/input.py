import RPi.GPIO as GPIO
import signal
import sys
import time
import subprocess


channel = 11
trig = False

def signal_handler(sig, frame):
    GPIO.cleanup()
    sys.exit(0)


def my_callback(channel):
    if trig == False:
        #print("edge")
        trig = True
        subprocess.call(["./drum"])
        
        
    if not GPIO.input(channel):
        print('button pressed')
    else:
        print('button released')

if __name__ == '__main__':
    GPIO.setmode(GPIO.BOARD) 
    GPIO.setup(channel, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.add_event_detect(channel, GPIO.FALLING, callback=my_callback, bouncetime=200)

    signal.signal(signal.SIGINT, signal_handler)
    signal.pause()