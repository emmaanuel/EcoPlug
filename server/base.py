#!/usr/bin/env python2

import RFM69
from RFM69registers import *
import datetime
import time
import pycurl
import json
from io import BytesIO
import sys
import RPi.GPIO as GPIO
import requests

n=0
tmpdata=[]
rooms=["","","juliette","salon","jardin","","garage","grenier"]
api_baseurl= "http://xxxx.com"

def pushOVH(metric, value):
	global n, tmpdata
	token_id = 'xxxxxxyy'                                                                                                        
	token_key = 'xxxxxx'                                                                                            
	end_point = 'https://opentsdb.iot.runabove.io/api/put'
	tmpdata.append({'metric': metric,'timestamp': long(time.time()),'value': value,'tags': {'source': 'ecoplug'}})
	n = n + 1
	if (n>20):
		try:                                                                                                    
			response = requests.post(end_point, data=json.dumps(tmpdata), auth=(token_id, token_key))          
			response.raise_for_status()                                                                     
			print('Send successful\nResponse code from server: {}'.format(response.status_code))          
		except requests.exceptions.HTTPError as e:                                                                        
			print('HTTP code is {} and reason is {}'.format(e.response.status_code, e.response.reason)) 
		n=0
		tmpdata=[]

def processMsg(msg, sender, rssi):
	global rooms
	if (msg.find('|')>-1):
		msgtype = msg.split('|')[0]
		if (msgtype == "T"):
			temp = msg.split('|')[1]
			rh = msg.split('|')[2]
			l = msg.split('|')[3]
			c = pycurl.Curl()
			c.setopt(pycurl.URL, api_baseurl + '/api/temp')
			c.setopt(pycurl.POST, 1)
			c.setopt(pycurl.POSTFIELDS, '{"n":"'+str(sender)+'","t":"'+temp+'","h":"'+rh+'","l":"'+l+'","r":"'+str(rssi)+'"}')
			c.perform()
			c.close() 
			pushOVH('home.temp.' + rooms[sender],float(temp))
			if (rh != ""): 
				pushOVH('home.rh.' + rooms[sender],float(rh)) 
			if (l != "" ):
				processLight(l)
				pushOVH('home.light.' + rooms[sender],int(l)) 
		if (msgtype == "P"):
			power = msg.split('|')[1]
			tf = msg.split('|')[2]
			hc = msg.split('|')[3]
			hp = msg.split('|')[4]
			c = pycurl.Curl()
			c.setopt(pycurl.URL, api_baseurl + '/api/edf')
			c.setopt(pycurl.POST, 1)
			c.setopt(pycurl.POSTFIELDS, '{"pw":"'+power+'","tf":"'+tf+'","hc":"'+hc+'","hp":"'+hp+'"}')
			c.perform()
			c.close()
			pushOVH('home.edf.power',int(power))
			pushOVH('home.edf.hc',int(hc))
			pushOVH('home.edf.hp',int(hp))
		if (msgtype == "E"):
			event = msg.split('|')[1]
			print time.strftime("%Y-%m-%d %H:%M : ") + "EVENT: " + event
			if (event == "MOTION"):
				print time.strftime("%Y-%m-%d %H:%M : ") +"MOTION Received  " 
				c = pycurl.Curl()
				c.setopt(pycurl.URL, api_baseurl + '/api/motion')
				c.setopt(pycurl.POST, 1)
				c.setopt(pycurl.POSTFIELDS, '{"n":"'+str(sender)+'","r":"'+str(rssi)+'"}')
				c.perform()
				c.close()
			if (event.startswith("HEATER")):
				c = pycurl.Curl()
				c.setopt(pycurl.URL, api_baseurl + '/api/heater')
				c.setopt(pycurl.POST, 1)
				heaterStatus = event.startswith("ON",7)
				c.setopt(pycurl.POSTFIELDS, '{"n":"'+str(sender)+'","r":"'+str(rssi)+'","s":"' + str(heaterStatus) + '"}')
				c.perform()
				c.close()

def processLight(light):
	global lastDayStatus,currentDayStatus,newDayStatus
	if (int(light) >30):
		if (lastDayStatus != "DAY"):
			lastDayStatus = "DAY"
			print time.strftime("%Y-%m-%d %H:%M : ") +"lastDayStatus change : " + lastDayStatus
			logAction("STORE_OPEN_AUTO");
			storeOpen() 
			 
	elif (int(light) <=30):
		if (lastDayStatus != "NIGHT"):
			lastDayStatus = "NIGHT"
			print time.strftime("%Y-%m-%d %H:%M : ") +"lastDayStatus change : " + lastDayStatus
			logAction("STORE_CLOSE_AUTO"); 
			storeClose()

pinUp=15
pinDown=13


def logAction(action):
	c = pycurl.Curl()                                                                                  
	c.setopt(pycurl.URL, api_baseurl + '/api/action/log')                                   
	c.setopt(pycurl.POST, 1)                                                                           
	c.setopt(pycurl.POSTFIELDS, '{"a":"'+action+'"}')                                                  
	c.perform()                                                                                        
	c.close() 	

def processAction(actionid):
	c = pycurl.Curl()
 	c.setopt(pycurl.URL, api_baseurl + '/api/action/process')
 	c.setopt(pycurl.POST, 1)
 	c.setopt(pycurl.POSTFIELDS, '{"id":"' + str(actionid) + '"}')
 	c.perform()
 	c.close()	
 	print "Action " + actionid + " processed"

def storeClose(auto=True):                                                                                          
	global pinDown
	print time.strftime("%Y-%m-%d %H:%M : ") + "STORE CLOSE"                                           
	GPIO.output(pinDown, GPIO.HIGH)                                                                                 
	time.sleep(0.1)                                                                                    
	GPIO.output(pinDown, GPIO.LOW)                                                                        
                                                                                                          
def storeOpen(auto=True):      
	global pinUp                                                                                             
	print time.strftime("%Y-%m-%d %H:%M : ") + "STORE OPEN"                     
	GPIO.output(pinUp, GPIO.HIGH)                                                                         
	time.sleep(0.1)                                                                                    
	GPIO.output(pinUp, GPIO.LOW)
                                                                                                          
GPIO.setmode(GPIO.BOARD)                                                                                           
GPIO.setup(pinDown, GPIO.OUT)                                                                                  
GPIO.setup(pinUp, GPIO.OUT)                                                                                   
GPIO.output(pinUp, GPIO.LOW)                                                                                  
GPIO.output(pinDown, GPIO.LOW)  

radio = RFM69.RFM69(RF69_868MHZ, 1, 100, False)
radio.setHighPower(False)
radio.encrypt("xxxxxx")
print "reading"
lastDayStatus = ""
currentDayStatus = ""
newDayStatus = ""
referenceTime = time.time()
while True:
	try:
		radio.receiveBegin()
		while not radio.receiveDone():
			if ((time.time() - referenceTime)>10):
				referenceTime = time.time()
				c = pycurl.Curl()
				c.setopt(pycurl.URL, api_baseurl + '/api/action/next')
				buffer = BytesIO()
				c.setopt(c.WRITEFUNCTION, buffer.write)
				c.perform()
				body = buffer.getvalue()
				data = json.loads(body)
				if ( len(data["action"])>0):
					print data["action"][0]["action"]
					identifiant = data["action"][0]["id"]
 					
					if(data["action"][0]["action"]=="STORE_OPEN|"):
						storeOpen(False)
						processAction(identifiant)
					if(data["action"][0]["action"]=="STORE_CLOSE|"): 
						storeClose(False)
						processAction(identifiant)
			time.sleep(.005)
		message= "".join([chr(letter) for letter in radio.DATA])
		sender = radio.SENDERID
		rssi = radio.RSSI
		if radio.ACKRequested():
			radio.sendACK()
		print time.strftime("%Y-%m-%d %H:%M : ") + "%s from %s RSSI:%s" % (message, sender, rssi)
		processMsg(message, sender, rssi)
	except IOError, e:
		print time.strftime("%Y-%m-%d %H:%M : ") + "Erreur : ", e
		sys.stdout.flush()
		reopen = True
	except Exception, e:
		print time.strftime("%Y-%m-%d %H:%M : ") + "Erreur : ", e
		msg=''
		sys.stdout.flush()

print "shutting down"
radio.shutdown()
