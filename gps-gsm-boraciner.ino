#include <TimerOne.h>
#include <TinyGPS.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

TinyGPS gps;

SoftwareSerial gsmSerial(2, 3);
SoftwareSerial ss(5,6);

boolean debug = true;

String inData = String();
boolean inputAvailable = false;
String strWhichMsg = String();
String NewMessageDEF = String();
String MessageReadString = String();
String readingFromInbox = String();
String sendReadRequest = String();
String PASSWORD = String("20012001");
String RemoveCommand = String("AT+CMGD=");
String ADMIN_PHONE_NUMBER = String("05558230165");
String DurumBilgisiStr = String();
String recievedNumber = String();

int callback_counter=0;
int indexofMsgStr = 0;
float flat, flon;
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
  
  NewMessageDEF = "+CMTI: \"SM\",";
  strWhichMsg = "";
  MessageReadString = "at+cmgr=";
  readingFromInbox = "REC UNREAD";

  
  
  Serial.print("ADMIN_PHONE_NUMBER =");
  Serial.println(ADMIN_PHONE_NUMBER);
  
  
  if(debug)
  Serial.println("Setup..! wait for 1 sec");
  delay(1000);
  Serial.print("_SS_MAX_RX_BUFF = ");
  Serial.println(_SS_MAX_RX_BUFF);
  removeSms();  
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
  }
  inData="";
  
  //UPDATE GPS DATA INTO EEPROM
  if(callback_counter >= 10)
  {
    Serial.print("callback ok!");
    callback_counter = 0;
    TAKEGPSDATA();
  }  
  
}
void printGPSDATA(){
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.println(flat,6);

    if( flat > 0.0 ) 
    flat_store = flat;   

    if( flon > 0.0 ) 
    flon_store = flon;   

    Serial.print("LON=");
    Serial.println(flon,6);
}

void TAKEGPSDATA(){  
 if(debug)Serial.println("---->TAKEGPSDATA"); 
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
 if(debug)Serial.println("<----TAKEGPSDATA");     
}

void processData(){
  Serial.println(inData);  
  
  if(IsRinging())
  { // telefon caliyor
    Serial.println("telefon caliyor");
    AramayiMesguleCevir();    
    indexofMsgStr = inData.indexOf("+CLIP: \"+9");
    indexofMsgStr += 10;
    recievedNumber = inData.substring(indexofMsgStr , indexofMsgStr+11); 
    Serial.print("recieved number=");
    Serial.println(recievedNumber);
    
    if(ADMIN_PHONE_NUMBER == recievedNumber)
    {
      TAKEGPSDATA();
      KoordinatBilgisiGonder();        
    }
  }
  else
  {
    Serial.println("ELSE");  
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
  atSendNumber += ADMIN_PHONE_NUMBER;
  atSendNumber += "\"";
  
  gsmSerial.println(atSendNumber);
  gsmSerial.print("LAT =");
  gsmSerial.print(flat_store,6);
  gsmSerial.print("LON =");
  gsmSerial.print(flon_store,6);
  
  gsmSerial.write(26);
}

void removeSms(){
if(debug)Serial.println("---->removeSms");  
gsmSerial.println("AT+CMGD=1,4");
delay(100);
gsmSerial.write(26);
if(debug)Serial.println("<----removeSms");
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
int ReadFromInbox()
{
  if (inData.indexOf(readingFromInbox) >= 0 )
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
}