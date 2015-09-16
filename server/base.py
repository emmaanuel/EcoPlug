#!/usr/bin/env python2

import RFM69
from RFM69registers import *
import datetime
import time
import pycurl
import json
from io import BytesIO
import sys

def processMsg(msg, sender, rssi):
	if (msg.find('|')>-1):
		msgtype = msg.split('|')[0]
		if (msgtype == "T"):
			temp = msg.split('|')[1]
			rh = msg.split('|')[2]
			l = msg.split('|')[3]
			c = pycurl.Curl()
			c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/temp')
			c.setopt(pycurl.POST, 1)
			c.setopt(pycurl.POSTFIELDS, '{"n":"'+str(sender)+'","t":"'+temp+'","h":"'+rh+'","l":"'+l+'","r":"'+str(rssi)+'"}')
			c.perform()
			c.close() 
			if (l != "" ):
				processLight(l)
		if (msgtype == "P"):
			power = msg.split('|')[1]
			tf = msg.split('|')[2]
			hc = msg.split('|')[3]
			hp = msg.split('|')[4]
			c = pycurl.Curl()
			c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/edf')
			c.setopt(pycurl.POST, 1)
			c.setopt(pycurl.POSTFIELDS, '{"pw":"'+power+'","tf":"'+tf+'","hc":"'+hc+'","hp":"'+hp+'"}')
			c.perform()
			c.close()
		if (msgtype == "E"):
			event = msg.split('|')[1]
			print time.strftime("%Y-%m-%d %H:%M : ") + "EVENT: " + event
			if (event == "MOTION"):
				c = pycurl.Curl()
				c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/motion')
				c.setopt(pycurl.POST, 1)
				c.setopt(pycurl.POSTFIELDS, '{"n":"'+str(sender)+'","r":"'+str(rssi)+'"}')
				c.perform()
				c.close()

def processLight(light):
	global lastDayStatus,currentDayStatus,newDayStatus
	if (int(light) >50):
		if (lastDayStatus == "DAY"):
			newDayStatus = "DAY"
			print time.strftime("%Y-%m-%d %H:%M : ") + "newDayStatus change : " + newDayStatus
		else:
			lastDayStatus = "DAY"
			print time.strftime("%Y-%m-%d %H:%M : ") +"lastDayStatus change : " + lastDayStatus
	elif (int(light) <50):
		if (lastDayStatus == "NIGHT"):
			newDayStatus = "NIGHT"
			print time.strftime("%Y-%m-%d %H:%M : ") +"newDayStatus change : " + newDayStatus
		else:
			lastDayStatus = "NIGHT"
			print time.strftime("%Y-%m-%d %H:%M : ") +"lastDayStatus change : " + lastDayStatus
	if (newDayStatus != currentDayStatus):
		print time.strftime("%Y-%m-%d %H:%M : ") + "DAY_STATUS change : " + newDayStatus
		currentDayStatus = newDayStatus
		if (newDayStatus == "DAY"):
			action = "STORE_OPEN|"
		else:
			action = "STORE_CLOSE|"
		textToSend ="A|" + action.encode() + "|"
		if (radio.sendWithRetry(5, textToSend, 3, 40)):
			c = pycurl.Curl()
			c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/action/log')
			c.setopt(pycurl.POST, 1)
			c.setopt(pycurl.POSTFIELDS, '{"a":"'+action+'"}')
			c.perform()
			c.close()
	

radio = RFM69.RFM69(RF69_868MHZ, 1, 100, False)
radio.setHighPower(False)
radio.encrypt("XXXXXXXXXXX")
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
				c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/action/next')
				buffer = BytesIO()
				c.setopt(c.WRITEFUNCTION, buffer.write)
				c.perform()
				body = buffer.getvalue()
				data = json.loads(body)
				if ( len(data["action"])>0):
					identifiant = data["action"][0]["id"]
					textToSend = "A|" + data["action"][0]["action"]
					print "Send Action"
					if (radio.sendWithRetry(5, textToSend, 3, 40)):
						print "Ok"
						c = pycurl.Curl()
						c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/action/process')
						c.setopt(pycurl.POST, 1)
						c.setopt(pycurl.POSTFIELDS, '{"id":"' + str(identifiant) + '"}')
						c.perform()
						c.close()
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
