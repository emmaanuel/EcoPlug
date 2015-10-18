#!/usr/bin/python

import RPi.GPIO as GPIO
import time
import sys

pinUp=15
pinDown=13

def storeClose():    
		global pinDown
        print time.strftime("%Y-%m-%d %H:%M : ") + "STORE CLOSE"                                           
        GPIO.output(pinDown, GPIO.HIGH)                                                                                 
        time.sleep(0.1)                                                                                    
        GPIO.output(pinDown, GPIO.LOW)                                                                         
                                                                                                          
def storeOpen():      
		global pinUp                                                                                             
        print time.strftime("%Y-%m-%d %H:%M : ") + "STORE OPEN"                     
        GPIO.output(pinUP, GPIO.HIGH)                                                                         
        time.sleep(0.1)                                                                                    
        GPIO.output(pinUP, GPIO.LOW)                                                                                  
                                                                                                          
GPIO.setmode(GPIO.BOARD)                                                                                           
GPIO.setup(pinDown, GPIO.OUT)                                                                                  
GPIO.setup(pinUP, GPIO.OUT)                                                                                   
GPIO.output(pinUP, GPIO.LOW)                                                                                  
GPIO.output(pinDown, GPIO.LOW)  