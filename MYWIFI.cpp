#include "Arduino.h"
#include "MYWIFI.h"
#include "string.h"
#include "SoftwareSerial.h"

SoftwareSerial mySerial(10,11); // RX, TX

MYWIFI::MYWIFI()
{
	init();
}

void MYWIFI::init(void)
{
	mySerial.begin(9600);
	while(!mySerial)
	{
		;
	}
	Serial.println("Connect To ESP8266!");
}

void MYWIFI::Reset(void)
{
	if (Conf("AT+RST","ready",10))
	{
		Serial.println("Reset ESP8266 Success!");
		while(!Conf("ATE0",5))
		{
			;
		}
	}
	else
	{
		Serial.println("Reset ESP8266 Fail!");
	}
	
}

bool MYWIFI::Begin(String ssid,String pwd)
{
	
	String AT = "AT+CWJAP=\"" + ssid + "\",\"" + pwd + "\"";
	if (Conf(AT,"OK","FAIL",10))
	{
		Serial.println("Connect To " + ssid + " Success!");
	}
	else
	{
		Serial.println("Connect To " + ssid + " Fail!");
	}
}

String MYWIFI::GetIP()
{
	String data = "";
	mySerial.println("AT+CIFSR");
	while(1)
	{
		if (mySerial.available())
		{
			char c = mySerial.read();
			data = data + String(c);
		}
		if (data.indexOf("STAIP,\"")!=-1)
		{
			String ip = "";
			while(1)
			{
				if (mySerial.available())
				{
					char c = mySerial.read();
					ip = ip + String(c);
				}
				if (ip.indexOf("\"")!=-1)
				{
					ip = ip.substring(0,ip.length()-1);
					return ip;
				}
			}
		}
	}
}

bool MYWIFI::TCPSERVER(void)
{
	if (Conf("AT+CWMODE=1",5))
	{
		Serial.println("Set AT+CWMODE=1 Success!");
		if (Conf("AT+CIPMUX=1",5))
		{
			Serial.println("Set AT+CIPMUX=1 Success!");
			if (Conf("AT+CIPSERVER=1","OK","ERROR",5))
			{
				Serial.println("Listening On Port 333!");
			}
			else
			{
				Serial.println("Set TCP Server Fail!");
			}
		}
		else
		{
			Serial.println("Set AT+CIPMUX=1 Fail!");
		}
	}
	else
	{
		Serial.println("Set AT+CWMODE=1 Fail!");
	}
}

bool MYWIFI::TCPSERVER(int port)
{
	if (Conf("AT+CWMODE=1",5))
	{
		Serial.println("Set AT+CWMODE=1 Success!");
		if (Conf("AT+CIPMUX=1",5))
		{
			Serial.println("Set AT+CIPMUX=1 Success!");
			if (Conf("AT+CIPSERVER=1," + String(port),"OK","ERROR",5))
			{
				Serial.println("Listening On Port " + String(port) + "!");
			}
			else
			{
				Serial.println("Set TCP Server Fail!");
			}
		}
		else
		{
			Serial.println("Set AT+CIPMUX=1 Fail!");
		}
	}
	else
	{
		Serial.println("Set AT+CWMODE=1 Fail!");
	}
}

bool MYWIFI::TCPCLIENT(String ip,int port)
{
	if(Conf("AT+CIPMUX=0",5))
	{
		Serial.println("Set AT+CIPMUX=0 Success!");
		if (Conf("AT+CIPSTART=\"TCP\",\"" + ip + "\"," + String(port),"OK","CLOSED"))
		{
			Serial.println("Connect " + ip + ":" + String(port) + " Success!");
		}
		else
		{
			Serial.println("Connect " + ip + ":" + String(port) + " Fail!");
		}
	}
	else
	{
		Serial.println("Set AT+CIPMUX=0 Fail!");
	}
	
}

bool MYWIFI::UDPSERVER(int targetport,int localport)
{
	if (Conf("AT+CIPMUX=0",5))
	{
		Serial.println("Set AT+CIPMUX=0 Success!");
		if (Conf("AT+CIPSTART=\"UDP\",\"" + GetIP() + "\"," + String(targetport) + "," + String(localport), "OK","ERROR",10))
		{
			Serial.println("Listening On Udp Port " + String(localport) + " Success!");
		}
		else
		{
			Serial.println("Listening On Udp Port Fail!");
		}
	}
	else
	{
		Serial.println("Set AT+CIPMUX=0 Fail!");
	}
}

bool MYWIFI::UDPSEND(String ip,int port,String msg)
{
	int length = msg.length();
	String data = "";
	mySerial.println("AT+CIPSEND=" + String(length) + ",\"" + ip + "\"," + String(port));
	long start = millis();
	while(1)
	{
		if (mySerial.available())
		{
			char c = mySerial.read();
			data = data + c;
		}
		if (data.indexOf(">")!=-1)
		{
			delay(100);
			mySerial.println(msg);
			data = "";
		}
		if (data.indexOf("ERROR")!=-1)
		{
			Serial.println("Send Fail!");
			return false;
		}
		if (data.indexOf("SEND OK")!=-1)
		{
			Serial.print("Send UDP Message:\"" + msg + "\"");
			Serial.println("To " + ip + ":" + String(port) + " Success!");
			return true;
		}
		if ((millis()-start)>=10*1000)
		{
			Serial.println("Out Of Delay Time 10s!");
			return false;
		}
	}
}

void MYWIFI::ShowAP()
{
	String data = "";
	mySerial.println("AT+CWLAP");
	while(1)
	{
		if (mySerial.available())
		{
			char c = mySerial.read();
			data = data + String(c);
		}
		if (data.indexOf(")")!=-1)
		{
			String AP = data.substring(data.indexOf("("),data.length());
			Serial.println(AP);
			data = "";
		}
		if (data.indexOf("OK")!=-1)
		{
			Serial.println("DONE");
			break;
		}
	}
}

int MYWIFI::TCPListening()
{
	if (mySerial.available())
	{
		long start = millis();
		String data = "";
		while(1)
		{
			if (mySerial.available())
			{
				char c = mySerial.read();
				data = data + String(c);
				start = millis();
			}
			if (data.indexOf("CONNECT")!=-1)
			{
				int id = int(data.charAt(data.indexOf(",")-1));
				return id;
			}
			if ((millis()-start) >= 1000)
			{
				return -1;
			}
		}
	}
	else
	{
		return -1;
	}
}

int MYWIFI::UDPListening()
{
	long start = millis();
	char data[1050];
	int i = 0;
	while(1)
	{
		char c;
		if (mySerial.available())
		{
			c = mySerial.read();
			
			data[i] = c;
			i++;
		}
		else
		{
			i = 0;
			continue;
		}
		if (c == '}')
		{
			Serial.println("\nEND\t");
			for (int i = 0; i < 1050; ++i)
			{
				Serial.write(data[i]);
			}
			break;
		}
		if (millis()-start>=10*1000)
		{
			break;
		}
		
	}
	
}

char MYWIFI::readFromESP()
{
	if (mySerial.available())
	{
		char c = mySerial.read();
		return c;
	}
	else
	{
		return NULL;
	}
}

bool MYWIFI::TCPSEND(int id,String msg)
{
	int length = msg.length();
  	String data = "";
  	mySerial.println("AT+CIPSEND=" + String(char(id)) + "," + String(length));
  	long start = millis();
  	while(1)
  	{
    	if (mySerial.available())
    	{
      		char c = mySerial.read();
      		data = data + String(c);
    	}
    	if (data.indexOf(">")!=-1)
   	 	{
      		delay(100);
      		mySerial.println(msg);
      		data = "";
    	}
    	if (data.indexOf("SEND OK")!=-1)
    	{
      		Serial.print(char(10));
      		Serial.println("Send TCP Message:\"" + msg + "\" Success!");
      		return true;
    	}
    	if (data.indexOf("ERROR")!=-1)
		{
			Serial.println("Send Fail!");
			return false;
		}
		if ((millis()-start)>=10*1000)
		{
			Serial.println("Out Of Delay Time 10s!");
			return false;
		}
  	}
}	

bool MYWIFI::Conf(String AT,int delay)
{
	flush();
	String data = "";
	mySerial.println(AT);
	long start = millis();
	while(1)
	{
		if (mySerial.available())
		{
			char c = mySerial.read();
			data = data + String(c);
		}
		if ((data.indexOf("OK")!=-1))
		{	
			return true;
		}
		if ((data.indexOf("ERROR")!=-1))
		{
			return false;
		}
		if ((millis()-start)>=delay*1000)
		{
			Serial.println("Out Of Delay Time " + String(delay) + "s!");
			return false;
		}
	}
}

bool MYWIFI::Conf(String AT,String END,int delay)
{
	flush();
	String data = "";
	mySerial.println(AT);
	long start = millis();
	while(1)
	{
		if (mySerial.available())
		{
			char c = mySerial.read();
			data = data + String(c);
		}
		if ((data.indexOf(END)!=-1))
		{	
			return true;
		}
		if ((millis()-start)>=delay*1000)
		{
			Serial.println("Out Of Delay Time " + String(delay) + "s!");
			return false;
		}
	}
}

bool MYWIFI::Conf(String AT,String success,String fail,int delay)
{
	flush();
	String data = "";
	mySerial.println(AT);
	long start = millis();
	while(1)
	{
		if (mySerial.available())
		{
			char c = mySerial.read();
			data = data + String(c);
		}
		if ((data.indexOf(success)!=-1))
		{	
			return true;
		}
		if ((data.indexOf(fail)!=-1))
		{
			return false;
		}
		if ((millis()-start)>=delay*1000)
		{
			Serial.println("Out Of Delay Time " + String(delay) + "s!");
			return false;
		}
	}
}

void MYWIFI::flush(void)
{
	long start = millis();
	while(1)
	{
		int length = mySerial.available();
		if (length)
		{
			start = millis();
			for (int i = 0; i < length; ++i)
			{
				mySerial.read();
			}
		}
		if ((millis() - start)>=5*1000)
		{
			break;
		}
	}
}