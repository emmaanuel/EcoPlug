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
                                        if (msgtype == "P"):
                                                hc = getValue(msg.split('|')[4])
                                                hp = getValue(msg.split('|')[5])
                                                power = getValue(msg.split('|')[2])
                                                tf = getValue(msg.split('|')[3])
                                                c = pycurl.Curl()
                                                c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/edf')
                                                c.setopt(pycurl.POST, 1)
                                                c.setopt(pycurl.POSTFIELDS, '{"pw":"'+power+'","tf":"'+tf+'","hc":"'+hc+'","hp":"'+hp+'"}')
                                                c.perform()
                                                c.close()
                                        elif (msgtype == "T"):
                                                temp = getValue(msg.split('|')[2])
                                                humi = getValue(msg.split('|')[3])
                                                light = getValue(msg.split('|')[4])
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
                                                c = pycurl.Curl()
                                                c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/temp')
                                                c.setopt(pycurl.POST, 1)
                                                c.setopt(pycurl.POSTFIELDS, '{"n":"'+node+'","t":"'+temp+'","l":"'+light+'","r":"'+rx+'"}')
                                                c.perform()
                                                c.close()
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
                                                        print "DAY_STATUS change : " + newDayStatus
                                                        currentDayStatus = newDayStatus
                                                        if (newDayStatus == "DAY"):
                                                                action = "STORE_OPEN"
                                                        else:
                                                                action = "STORE_CLOSE"
                                                        textToSend ="N:5|".encode() + action.encode() + "#"
                                                        sent = False
                                                        while (not sent):
                                                                print time.strftime("%Y-%m-%d %H:%M : ") +"Sending" + textToSend
                                                                ser.write(textToSend)
                                                                received = ser.readline()[:-2] +"#"
                                                                print time.strftime("%Y-%m-%d %H:%M : ") +"received: " + received
                                                                if (received == textToSend):
                                                                        sent = True
                                                        c = pycurl.Curl()
                                                        c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/action/log')
                                                        c.setopt(pycurl.POST, 1)
                                                        c.setopt(pycurl.POSTFIELDS, '{"a":"'+action+'"}')
                                                        c.perform()
                                                        c.close()
                                                        sys.stdout.flush()
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
                                                print time.strftime("%Y-%m-%d %H:%M : ") + "unknown: " + msg + "\r\n"
                                                sys.stdout.flush()
                                        msg = ''
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
                                textToSend = "N:5|".encode() + data["action"][0]["action"].encode() + "#"
                                sent = False
                                while (not sent):
                                        print time.strftime("%Y-%m-%d %H:%M : ") + "Sending" + textToSend
                                        ser.write(textToSend)
                                        received = ser.readline()[:-2] +"#"
                                        print time.strftime("%Y-%m-%d %H:%M : ") + "received: " + received
                                        if (received == textToSend):
                                                sent = True
                                c = pycurl.Curl()
                                c.setopt(pycurl.URL, 'http://domo.emmaanuel.com/api/action/process')
                                c.setopt(pycurl.POST, 1)
                                c.setopt(pycurl.POSTFIELDS, '{"id":"' + str(identifiant) + '"}')
                                c.perform()
                                c.close()
                                sys.stdout.flush()

        except IOError, e:
                                print time.strftime("%Y-%m-%d %H:%M : ") + "Erreur : ", e
                                sys.stdout.flush()
                                reopen = True
        except Exception, e:
                                print time.strftime("%Y-%m-%d %H:%M : ") + "Erreur : ", e
                                print time.strftime("%Y-%m-%d %H:%M : ") + "errormsg: ", msg
                                msg=''
                                sys.stdout.flush()
