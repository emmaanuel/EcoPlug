#!/usr/bin/python
import serial
import sys
import time
import pycurl
from io import BytesIO
import json

def getValue(str):
        return str.split(':')[1]

try:
        time.sleep(2)
        ser = serial.Serial('/dev/ttyUSB0',57600)
        msg = ''
except Exception, e:
	print('Nous avons une erreur : %s !' % e)        
	sys.exit(1)
referenceTime = time.time()
reopen = False
lastDayStatus = ""
currentDayStatus = ""
newDayStatus = ""
while (1==1):
        try:
                if (reopen):
                        ser.close()
                        ser.open()
                        reopen = False
                if (ser.inWaiting()>0):
                        current = ser.read()
                        if ((current != chr(10)) and (current!=chr(13))):
                                msg= msg + current
                        if (current==chr(13)):
                                        msgtype = ""
                                        if (msg.find('|')>-1):
                                                msgtype = msg.split('|')[1]
                                        if (msgtype == "PW"):
                                                hc = getValue(msg.split('|')[2])
                                                hp = getValue(msg.split('|')[3])
                                                power = getValue(msg.split('|')[4])
                                                tf = getValue(msg.split('|')[5])
                                                c = pycurl.Curl()
                                                c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/edf')
                                                c.setopt(pycurl.POST, 1)
                                                c.setopt(pycurl.POSTFIELDS, '{"pw":"'+power+'","tf":"'+tf+'","hc":"'+hc+'","hp":"'+hp+'"}')
                                                c.perform()
                                                c.close()
                                        elif (msgtype == "TH"):
                                                node = getValue(msg.split('|')[0])
                                                temp = getValue(msg.split('|')[2])
                                                humi = getValue(msg.split('|')[3])
                                                rx = getValue(msg.split('|')[4])
                                                c = pycurl.Curl()
                                                c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/temp')
                                                c.setopt(pycurl.POST, 1)
                                                c.setopt(pycurl.POSTFIELDS, '{"n":"'+node+'","t":"'+temp+'","h":"'+humi+'","r":"'+rx+'"}')
                                                c.perform()
                                                c.close()
                                        elif (msgtype == "TL"):
                                                node = getValue(msg.split('|')[0])
                                                temp = getValue(msg.split('|')[2])
                                                light = getValue(msg.split('|')[3])
                                                rx = getValue(msg.split('|')[4])
                                                print int(light)
                                                if (int(light) >50):
                                                        if (lastDayStatus == "DAY"):
                                                                newDayStatus = "DAY"
                                                                print "newDayStatus change : " + newDayStatus
                                                        else:
                                                                lastDayStatus = "DAY"
                                                                print "lastDayStatus change : " + lastDayStatus
                                                else:
                                                        if (lastDayStatus == "NIGHT"):
                                                                newDayStatus = "NIGHT"
                                                                print "newDayStatus change : " + newDayStatus
                                                        else:
                                                                lastDayStatus = "NIGHT"
                                                                print "lastDayStatus change : " + lastDayStatus
                                                if (newDayStatus != currentDayStatus):
                                                        print "DAT_STATUS change : " + newDayStatus
                                                        currentDayStatus = newDayStatus
                                                        if (newDayStatus == "DAY"):
                                                                textToSend ="N:15STORE_OPEN".encode() + "#"
                                                        else:
                                                                textToSend ="N:5|STORE_CLOSE".encode() + "#"
                                                        sent = False
                                                        while (not sent):
                                                                print "Sending" + textToSend
                                                                ser.write(textToSend)
                                                                received = ser.readline()[:-2] +"#"
                                                                print "received: " + received
                                                                if (received == textToSend):
                                                                        sent = True
                                                c = pycurl.Curl()
                                                c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/temp')
                                                c.setopt(pycurl.POST, 1)
                                                c.setopt(pycurl.POSTFIELDS, '{"n":"'+node+'","t":"'+temp+'","r":"'+rx+'","l":"'+light+'"}')
                                                c.perform()
                                                c.close()
                                        elif (msgtype == "MO"):
                                                node = getValue(msg.split('|')[0])
                                                rx = getValue(msg.split('|')[2])
                                                c = pycurl.Curl()
                                                c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/motion')
                                                c.setopt(pycurl.POST, 1)
                                                c.setopt(pycurl.POSTFIELDS, '{"n":"'+node+'","r":"'+rx+'"}')
                                                c.perform()
                                                c.close()
                                        elif (msgtype == "T"):
                                                node = getValue(msg.split('|')[0])
                                                temp = getValue(msg.split('|')[2])
                                                c = pycurl.Curl()
                                                c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/temp')
                                                c.setopt(pycurl.POST, 1)
                                                c.setopt(pycurl.POSTFIELDS, '{"n":"'+node+'","t":"'+temp+'"}')
                                                c.perform()
                                                c.close()    
                                        else :
                                                print("unknown: " + msg + "\r\n")
                                        msg = ''
                if ((time.time() - referenceTime)>5):
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
                                textToSend = "N:5|".encode() + data["action"][0]["action"].encode() + "#"
                                sent = False
                                while (not sent):
                                        print "Sending" + textToSend
                                        ser.write(textToSend)
                                        received = ser.readline()[:-2] +"#"
                                        print "received: " + received
                                        if (received == textToSend):
                                                sent = True
                                c = pycurl.Curl()
                                c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/action/process')
                                c.setopt(pycurl.POST, 1)
                                c.setopt(pycurl.POSTFIELDS, '{"id":"' + str(identifiant) + '"}')
                                c.perform()
                                c.close()

        except IOError, e:
                                print "Erreur : %s \r\n", e
                                reopen = True
        except Exception, e:
                                print "Erreur : %s \r\n", e
                                print "msg: " + msg+ "\r\n"
                                msg=''
                                sys.stdout.flush()
