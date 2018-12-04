#coding:utf-8
import struct
import time
import snap7
from snap7.snap7exceptions import Snap7Exception

#import snap7.client
import snap7client
from snap7.snap7types import *
from snap7.util import *
import sys

def create_plc_client():
	print "create_plc_client"
	#client = snap7.client.Client()
	client = snap7client.Client()
	return client

def destroy_plc_client(client):	
	print "destroy plc client"
	client.destroy()

def plc_connect(client,ip, rack=0, slot=1):
	try:
		client.connect(ip, rack, slot)
		print "plc_connect"
		return 0
	except Snap7Exception,error:
		print "Exception:",error
		return -1
        
def plc_con_close(client):
    client.disconnect()

def readI(client,nameStr):
    if nameStr[0]=='I' or nameStr[0]=='i':
        if nameStr[1]>='0' and nameStr[1]<='3':
            addr=int(nameStr.split('.')[0][1:])
            bit=int(nameStr.split('.')[1])
            if bit>7:
                print "I bit out of range"
                return -1,0
            m_data = client.read_area(0x81, 0, addr, 1)
            return  0,(struct.unpack('!B', m_data)[0]>>bit)&1
        else:
            print "input out of range"
            return -1,0
    else:
        print "input error"
        return -1,0

def readQ(client,nameStr):
    if nameStr[0]=='Q' or nameStr[0]=='q':
        if nameStr[1]>='0' and nameStr[1]<='2':
            addr=int(nameStr.split('.')[0][1:])
            bit=int(nameStr.split('.')[1])
            if bit>7:
                print "Q bit out of range"
                return -1,0
            m_data = client.read_area(0x82, 0, addr, 1)
            return  0,(struct.unpack('!B', m_data)[0]>>bit)&1
        else:
            print "output out of range"
            return -1,0
    else:
        print "input error"
        return -1,0

def readM(client,nameStr):
    if nameStr[0]=='M' or nameStr[0]=='m':
        if nameStr[1]>='0' and nameStr[1]<='9':
            addr=int(nameStr.split('.')[0][1:])
            if addr >=0 and addr<=31:
                bit=int(nameStr.split('.')[1])
                if bit>7:
                    print "M bit out of range"
                    return -1,0
                m_data = client.read_area(0x83, 0, addr, 1)
                return  0,(struct.unpack('!B', m_data)[0]>>bit)&1
            else:
                print "address out of range"
                return -1,0    
        elif nameStr[1]=="B" or nameStr[1]=='b':
            addr=int(nameStr[2:])
            if addr >=0 and addr <=31:
                m_data=client.read_area(0x83, 0, addr, 1)
                return 0,struct.unpack('!B', m_data)[0]
            else:
                print "address out of range"
                return -1,0           
        elif nameStr[1]=="W" or nameStr[1]=='w':
            addr=int(nameStr[2:])
            if addr >=0 and addr <=30:
                m_data=client.read_area(0x83, 0, addr, 2)
                return 0,struct.unpack('!H', m_data)[0]
            else:
                print "address out of range"
                return -1,0
        elif nameStr[1]=="D" or nameStr[1]=='d':
            addr=int(nameStr[2:])
            if addr >=0 and addr <=28:
                m_data=client.read_area(0x83, 0, addr, 4)
                return 0,struct.unpack('!I', m_data)[0]
            else:
                print "address out of range"
                return -1,0                   
    else:
        print "input error"
        return -1,0


def readV(client,nameStr):
    if nameStr[0]=='V' or nameStr[0]=='v':
        if nameStr[1]>='0' and nameStr[1]<='9':
            addr=int(nameStr.split('.')[0][1:])
            # if addr >=0 and addr<=16359:
            bit=int(nameStr.split('.')[1])
            if bit>7:
                print "M bit out of range"
                return -1,0
            m_data = client.read_area(0x84, 1, addr, 1)
            return  0,(struct.unpack('!B', m_data)[0]>>bit)&1
            # else:
            #     print "address out of range"
            #     return 
        elif nameStr[1]=="B" or nameStr[1]=='b':
            addr=int(nameStr[2:])
            m_data=client.read_area(0x84, 1, addr, 1)#area, dbnumber, start, amount
            return 0,struct.unpack('!B', m_data)[0]
        elif nameStr[1]=="W" or nameStr[1]=='w':
            addr=int(nameStr[2:])
            m_data=client.read_area(0x84, 1, addr, 2)
            return 0,struct.unpack('!H', m_data)[0]
        elif nameStr[1]=="D" or nameStr[1]=='d':
            addr=int(nameStr[2:])
            m_data=client.read_area(0x84, 1, addr, 4)
            return 0,struct.unpack('!I', m_data)[0]          
    else:
        print "input error"
        return -1,0


def read(client,nameStr):
	try:
		if nameStr[0]=='M' or nameStr[0]=='m':
			return readM(client,nameStr)
		elif nameStr[0]=='Q' or nameStr[0]=='q':
			return readQ(client,nameStr)
		elif nameStr[0]=='I' or nameStr[0]=='i':
			return readI(client,nameStr)
		elif nameStr[0]=='V' or nameStr[0]=='v':
			return readV(client,nameStr)
		else:
			return -1,0
	except Snap7Exception,error:
		print "Exception:",error
		return -1,0


if __name__ == "__main__":
    client_fd=create_plc_client()   
    if plc_connect(client_fd,'192.168.1.234'): 
        result=readI(client_fd,"i0.3")
        print 'i0.3=',result
        result=readI(client_fd,"i1.4")
        print 'i1.4=',result
        result=readI(client_fd,"i2.5")
        print 'i2.5=',result

        result=readQ(client_fd,"q0.6")
        print 'q0.6=',result
        result=readQ(client_fd,"q1.7")
        print 'q1.7=',result

        result=readM(client_fd,"M2.1")
        print 'M2.1=',result

        result=readM(client_fd,"Mb13")
        print 'Mb13=',result

        result=readM(client_fd,"Mw5")
        print 'Mw5=',result 

        result=readM(client_fd,"Md22")
        print 'Md22=',result

        result=readV(client_fd,"v23.1")
        print 'v23.1=',result

        result=readV(client_fd,"vb133")
        print 'vb133=',result

        result=readV(client_fd,"vw56")
        print 'vw56=',result 

        result=readV(client_fd,"vd228")
        print 'vd228=',result


        plc_con_close(client_fd)
    else:
        print 'link error'
        sys.exit(1)
   
