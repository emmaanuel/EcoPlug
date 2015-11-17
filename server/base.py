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
api_domain = "http://tom.emmaanuel.com"

def pushOVH(metric, value):
	global n, tmpdata
	token_id = 'xxxxxx'                                                                                                        
	token_key = 'xxxxxxx'                                                                                            
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
			c.setopt(pycurl.URL, api_domain + '/api/temp')
			c.setopt(pycurl.POST, 1)
			c.setopt(pycurl.POSTFIELDS, '{"n":"'+str(sender)+'","t":"'+temp+'","h":"'+rh+'","l":"'+l+'","r":"'+str(rssi)+'"}')
			c.perform()
			c.close() 
			pushOVH('home.temp.' + rooms[sender],float(temp))
			if (rh != ""): 
				pushOVH('home.rh.' + rooms[sender],float(rh)) 
		if (msgtype == "P"):
			power = msg.split('|')[1]
			tf = msg.split('|')[2]
			hc = msg.split('|')[3]
			hp = msg.split('|')[4]
			c = pycurl.Curl()
			c.setopt(pycurl.URL, api_domain + '/api/edf')
			c.setopt(pycurl.POST, 1)
			c.setopt(pycurl.POSTFIELDS, '{"pw":"'+power+'","tf":"'+tf+'","hc":"'+hc+'","hp":"'+hp+'"}')
			c.perform()
			c.close()
			pushOVH('home.edf.power',int(power))
			pushOVH('home.edf.hc',int(hc))
			pushOVH('home.edf.hp',int(hp))

                                                                                                          
GPIO.setmode(GPIO.BOARD)                                                                                           
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
	except Exception, e:
		print time.strftime("%Y-%m-%d %H:%M : ") + "Erreur : ", e
		msg=''
		sys.stdout.flush()

print "shutting down"
radio.shutdown()
