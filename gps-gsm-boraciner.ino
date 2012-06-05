#include <TimerOne.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>

TinyGPS gps;

SoftwareSerial gsmSerial(2, 3);
SoftwareSerial ss(5,6);

boolean debug = true;

String inData = String();
boolean inputAvailable = false;
boolean admin_called = false;
String RemoveCommand = String("AT+CMGD=");

String ADMIN_PHONE_NUMBER_1 = String("05558230165");
String ADMIN_PHONE_NUMBER_2 = String("05323342412");
String ADMIN_PHONE_NUMBER_3 = String("05396933288");
String ADMIN_PHONE_NUMBER_4 = String("05373624078");

String recievedNumber = String();

int callback_counter=0;
int removesms_counter=0;
int indexofMsgStr = 0;
float flat, flon, fkmph;
unsigned long age;

float flat_store = 0.0;    
float flon_store = 0.0;

void setup()  
{
  Serial.begin(9600);
  ss.begin(9600);
  gsmSerial.begin(9600);  
  
  Timer1.initialize(8388480); //about 8.3 seconds
  Timer1.attachInterrupt(callback);  
  
  delay(3000);
  inData = ""; 
  
  //Serial.println("basla...");     
  
}

void loop() // run over and over
{
  gsmSerial.listen();
  while(gsmSerial.available())
  {
    inputAvailable = true;
    int tmp = gsmSerial.read();
    inData += (char) tmp;
    delay(7);
  }
  if(inputAvailable){
    inputAvailable=false;
    processData();
    inData="";
  }
  
  //UPDATE GPS DATA INTO EEPROM
  if(callback_counter >= 80)
  {
    //Serial.print("callback ok!");
    callback_counter = 0;
    TAKEGPSDATA();
  }  

  //REMOVE ALL SMS MESSAGES
  if(removesms_counter >= 10000)
  {
    removesms_counter = 0;
    removeSms();
  }
  
  
}
void printGPSDATA(){
    gps.f_get_position(&flat, &flon, &age);

    if( flat > 0.0 ) 
    flat_store = flat;   

    if( flon > 0.0 ) 
    flon_store = flon;   

    fkmph = gps.f_speed_kmph(); // speed in km/hr

    //Serial.print("HIZ=");
    //Serial.println(fkmph);
    //Serial.print("LAT=");
    //Serial.println(flat,6);
    //Serial.print("LON=");
    //Serial.println(flon,6);
}

void TAKEGPSDATA(){  
    //Serial.println("---->TAKEGPSDATA"); 
    ss.listen(); 
    delay(100);
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
      while (ss.available())
      {
        char c = ss.read();
        if (gps.encode(c)) // Did a new valid sentence come in?
          { 
            printGPSDATA();
          }
      }
    }
    gsmSerial.listen();  
    //Serial.println("<----TAKEGPSDATA");     
}

void processData(){
  //Serial.println(inData);  
  
  if(IsRinging())
  { // telefon caliyor
    //Serial.println("telefon caliyor");
    AramayiMesguleCevir();    
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

void AramayiMesguleCevir(){
  gsmSerial.println("AT H");
  delay(50);
}



/***********DURUM BILGISI GONDER*********/
/*****************START*******************/
void KoordinatBilgisiGonder(){
  String atSendNumber = String(23);
  /////////////SEND SMS////////////
  gsmSerial.println("AT+CMGF=1");
  
  atSendNumber = "AT+CMGS=\"+9";
  atSendNumber += recievedNumber;
  atSendNumber += "\"";

  gsmSerial.println(atSendNumber);
  
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

void removeSms(){
//Serial.println("---->removeSms");  
gsmSerial.println("AT+CMGD=1,4");
delay(100);
gsmSerial.write(26);
//Serial.println("<----removeSms");
}

int IsRinging()
{
  if (inData.indexOf("VOICE") >= 0 )
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
  removesms_counter++;
}
