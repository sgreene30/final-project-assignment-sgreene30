import RPi.GPIO as GPIO
import signal
import sys
import time
import subprocess


drum_channel = 11
hat_channel = 13

def signal_handler(sig, frame):
    GPIO.cleanup()
    sys.exit(0)

def poll_inputs(trigger):
    if GPIO.input(drum_channel) == 0:
        trigger = True
        subprocess.call(["./drum"])
    elif GPIO.input(hat_channel) == 0:
        trigger = True
        subprocess.call(["./hat"])
    return trigger



if __name__ == '__main__':
    
    trig = False
    GPIO.setmode(GPIO.BOARD) 
    GPIO.setup(drum_channel, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(hat_channel, GPIO.IN, pull_up_down=GPIO.PUD_UP)

    signal.signal(signal.SIGINT, signal_handler)

    while(1):
        trig = poll_inputs(trig)
        if trig:
            print("event")
            time.sleep(1)
            trig = False

        time.sleep(0.05)



