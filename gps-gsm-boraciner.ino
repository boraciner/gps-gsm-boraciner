#include <TimerOne.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>

TinyGPS gps;

SoftwareSerial gsmSerial(2, 3);
SoftwareSerial ss(5,6);

String inData = "";
boolean inputAvailable = false;

String ADMIN_PHONE_NUMBER_1 = "05558230165";
String ADMIN_PHONE_NUMBER_2 = "05323342412";
String ADMIN_PHONE_NUMBER_3 = "05396933288";
String ADMIN_PHONE_NUMBER_4 = "05373624078";

String recievedNumber = "";
String  bora =  "";
int callback_counter=0;
int indexofMsgStr = 0;
float flat, flon, fkmph;
unsigned long age;
char okunanKarakter = '*';
float flat_store = 0.0;
float flon_store = 0.0;
boolean gsmdinle = false;

void setup()
{
  delay(30000);
  Serial.begin(9600);
  ss.begin(9600);
  gsmSerial.begin(9600);

  Timer1.initialize(8388480); //about 8.3 seconds
  Timer1.attachInterrupt(callback);

  delay(2000);
  gsmSerial.listen();

  //Serial.println("basla");

  gsmSerial.println("AT+CLIP=1");
   
  delay(1000);
  gsmSerial.println("ATE=0");
  
  delay(1000);
  
  gsmSerial.listen();
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
}
void printGPSDATA(){
  gps.f_get_position(&flat, &flon, &age);

  if( flat > 0.0 )
    flat_store = flat;

  if( flon > 0.0 )
    flon_store = flon;

  fkmph = gps.f_speed_kmph(); // speed in km/hr

}

void TAKEGPSDATA(){
  //Serial.println("---->TAKEGPSDATA");
  boolean gps_valid = false;
  ss.listen();
  delay(1000);
  
  
  for(int i =0 ; i < 200 ; i++)
  {
    if(!gps_valid)
    {
      while (ss.available())
      {
        char c = ss.read();
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
  //Serial.println("<----TAKEGPSDATA");
  
}

void processData(){
  //Serial.println(inData);

  if(IsRinging())
  { // telefon caliyor
    //Serial.println("telefon caliyor");
    indexofMsgStr = inData.indexOf("05");
    recievedNumber = inData.substring(indexofMsgStr , indexofMsgStr+11);
    //Serial.print("recieved number=");
    //Serial.println(recievedNumber);

    if(IsAdminNumber())
    {
      //Serial.println("admin ok!");
      TAKEGPSDATA();
      KoordinatBilgisiGonder();
    }

  }
}

void KoordinatBilgisiGonder(){
  
  
  bora = "AT+CMGS=\"";
  bora+= "+9";
  bora+=recievedNumber;
  bora+= "\"";
  
  gsmSerial.println("AT+CMGF=1");
  delay(2000);
  gsmSerial.println(bora);
  delay(1000);

  gsmSerial.print("ENLEM =");
  gsmSerial.println(flat_store,6);
  gsmSerial.print("BOYLAM =");
  gsmSerial.println(flon_store,6);
  
  gsmSerial.print("HIZ =");
  gsmSerial.print(fkmph);
  gsmSerial.println(" km/saat");
  
  gsmSerial.println("LINK =");
  gsmSerial.print("http://maps.google.com/?ie=UTF8&hq=&ll=");
  gsmSerial.print(flat_store,6);
  gsmSerial.print(",");
  gsmSerial.print(flon_store,6);
  gsmSerial.print("&z=20");
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
int IsAdminNumber()
{

  if(ADMIN_PHONE_NUMBER_1 == recievedNumber)
  {
    return 1;
  }
  else if(ADMIN_PHONE_NUMBER_2 == recievedNumber)
  {
    return 1;
  }
  else if(ADMIN_PHONE_NUMBER_3 == recievedNumber)
  {
    return 1;
  }
  else if(ADMIN_PHONE_NUMBER_4 == recievedNumber)
  {
    return 1;
  }
  return 0;

}

void callback()
{
  callback_counter++;
  
  if(callback_counter >= 80)
  {
    //Serial.print("callback ok!");
    callback_counter = 0;
    TAKEGPSDATA();
  }
}
