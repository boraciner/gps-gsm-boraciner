#include <TimerOne.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>
#define ONE_MINUTE 8

String inData = "";
boolean inputAvailable = false;
String recievedNumber = "";
String gpsMesajIcerik = "";
int callback_counter=0;
int indexofMsgStr = 0;
float flat, flon, fkmph;
unsigned long age;
char okunanKarakter = '*';
float flat_store = 0.0;
float flon_store = 0.0;
int buzzer = 7;
boolean set_alarm = false;
float alarm_lat = 0.0 , alarm_lon = 0.0;
boolean alarm_coordinates_ok = false , alarm_admin_called = false;
int callback_max = ONE_MINUTE * 20;
float GPS_ALARM_TOLERANCE = 0.002;

SoftwareSerial gsmSerial(2,3);
SoftwareSerial gpsSerial(4,5);
TinyGPS gps;

void setup()
{
  

  
	pinMode(buzzer, OUTPUT);
	Serial.begin(9600);
	gpsSerial.begin(9600);
	gsmSerial.begin(9600);

	Timer1.initialize(8388480); //yaklasik 8.3 saniye
	Timer1.attachInterrupt(callback);

	delay(2000);
	gsmSerial.listen();

	delay(1000);
	gsmSerial.listen();

	fastbuzzer();
}

void loop() // run over and over
{
	while(gsmSerial.available())
	{
		inputAvailable = true;
		okunanKarakter = (char) gsmSerial.read();
		inData += okunanKarakter;
		if(okunanKarakter == ','){
			break;
		}
	}

	if(inputAvailable)
	{
		inputAvailable=false;
		processData();
		
		inData="";
	}

	if(set_alarm)
	{
		TAKEGPSDATA();
		set_alarm=false;
	}
}


void TAKEGPSDATA(){
	boolean gps_valid = false;
	gpsSerial.listen();
	delay(1000);


	for(int i =0 ; i < 200 ; i++)
	{
		if(!gps_valid)
		{
			while (gpsSerial.available())
			{
				char c = gpsSerial.read();
				if (gps.encode(c)) // Did a new valid sentence come in?
				{
					printGPSDATA();
					gps_valid = true;
					break;
				}
			}
			delay(5);
		}
	}

	gsmSerial.listen();
	delay(1000);
}

void processData(){
	Serial.println(inData);
	
	if(IsRinging())
	{ // telefon caliyor
		indexofMsgStr = inData.indexOf("05");
		recievedNumber = inData.substring(indexofMsgStr , indexofMsgStr+11);
		
		if(IsAdminNumber())   // BORAAAAAAAAAA
		{
			TAKEGPSDATA();
			if(currentGpsDataOk())
			{
				KoordinatBilgisiGonder();
			}
			else
			{
				if(storedGpsDataOk())
				{
					KoordinatBilgisiGonder();
				}
				else
				{
					HazirDegilBilgisiGonder();
				}
			}
		}
	}
}


void printGPSDATA()
{
	gps.f_get_position(&flat, &flon, &age);

	if( flat > 0.0 && flon > 0.0)
	{
		flat_store = flat;
		flon_store = flon;
		
		if(!alarm_coordinates_ok)
		{
			alarm_lat = flat_store;
			alarm_lon = flon_store;  
			alarm_coordinates_ok = true;
		}
		else
		{
			if( alarm_lat < flat_store - GPS_ALARM_TOLERANCE || alarm_lat > flat_store + GPS_ALARM_TOLERANCE ||
					alarm_lon < flon_store - GPS_ALARM_TOLERANCE || alarm_lon > flon_store + GPS_ALARM_TOLERANCE   )
			{
				if(!alarm_admin_called)
				{
					gsmSerial.println("ATD+905558230165;");
					alarm_admin_called = true;
				}
			}
		}

	}
	

	fkmph = gps.f_speed_kmph(); // speed in km/hr

}

int IsAdminNumber()
{
	if(recievedNumber.equals("05558230165"))
	{
		return 1;
	}
	else
	{
		if(recievedNumber.equals("05073624078"))
		{
			return 1;
		} 
		else
		{
			if(recievedNumber.equals("05323342412"))
			{
				return 1;
			}
			else
			{
				if(recievedNumber.equals("05396933288"))
				{
					return 1;
				}
				else
				{
					if(recievedNumber.equals("05532764121"))
					{		
						return 1;
					}                                       
					else
					{
						return 0;
					}  
				}
			}
		}
	}
}
void KoordinatBilgisiGonder(){

	gpsMesajIcerik = "AT+CMGS=\"";
	gpsMesajIcerik+= "+9";
	gpsMesajIcerik+=recievedNumber;
	gpsMesajIcerik+= "\"";

	gsmSerial.println("AT+CMGF=1");
	delay(2000);
	gsmSerial.println(gpsMesajIcerik);
	delay(1000);

	gsmSerial.print("ENLEM =");
	gsmSerial.println(flat_store,6);
	gsmSerial.print("BOYLAM =");
	gsmSerial.println(flon_store,6);

	gsmSerial.print("HIZ =");
	gsmSerial.print(fkmph);
	gsmSerial.println(" km/saat");

	gsmSerial.println("LINK =");
	gsmSerial.print("https://maps.google.com/maps?q=");
	gsmSerial.print(flat_store,6);
	gsmSerial.print(",");
	gsmSerial.print(flon_store,6);
	gsmSerial.print("+(Frontera)");
	gsmSerial.write(26);
}


void HazirDegilBilgisiGonder()
{
	gpsMesajIcerik = "AT+CMGS=\"";
	gpsMesajIcerik+= "+9";
	gpsMesajIcerik+=recievedNumber;
	gpsMesajIcerik+= "\"";

	gsmSerial.println("AT+CMGF=1");
	delay(2000);
	gsmSerial.println(gpsMesajIcerik);
	delay(1000);

	gsmSerial.println("Gps Verisi Hazir Degil!");
	gsmSerial.write(26);
}

int IsRinging()
{
	if (inData.indexOf("CLIP:") >= 0 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


int currentGpsDataOk()
{
	if(( flat > 0.0 ) && ( flon > 0.0 ) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
int storedGpsDataOk()
{
	if(( flat_store > 0.0 ) && ( flon_store > 0.0 ) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}



void callback()
{
	callback_counter++;
	if(callback_counter >= callback_max)
	{
		callback_max = ONE_MINUTE * 8;
		callback_counter = 0;
		if(!set_alarm)
		{ 
			set_alarm=true;
		}
	}
}	

void fastbuzzer()
{
	for(int i = 0; i< 10; i++)
	{
		digitalWrite(buzzer, HIGH);   // turn the LED on (HIGH is the voltage level)
		delay(100);               // wait for a second
		digitalWrite(buzzer, LOW);    // turn the LED off by making the voltage LOW
		delay(100); 
	}
}

