#include <TimerOne.h>
#include <TinyGPS.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

TinyGPS gps;

SoftwareSerial gsmSerial(2, 3); //SoftwareSerial(rxPin, txPin);
SoftwareSerial ss(5,6);

boolean debug = true;

String inData = String(); // Allocate some space for the string
boolean inputAvailable = false;
String strWhichMsg = String();
String NewMessageDEF = String();
String MessageReadString = String();
String readingFromInbox = String();
String sendReadRequest = String();
String PASSWORD = String();
String RemoveCommand = String("AT+CMGD=");
String ADMIN_PHONE_NUMBER = String("05558230165");
String DurumBilgisiStr = String();

int callback_counter=0;
float flat, flon;
unsigned long age;
String flat_Str = String();
String flon_Str = String();
    
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

  ReadPasswordFromEEPROM();
  
  Serial.print("PASSWORD =");
  Serial.println(PASSWORD);
  
  if(debug)
  Serial.println("Setup..! wait for 1 sec");
  delay(1000);
  Serial.print("_SS_MAX_RX_BUFF = ");
  Serial.println(_SS_MAX_RX_BUFF);
//  removeSms();  
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
    Serial.println("callback okkk");    
    TAKEGPSDATA();
    callback_counter = 0;
  }  
  
}
void printGPSDATA(){
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.println(flat,6);
    
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
  Serial.println("---->processData");  
  Serial.println(inData);  
  
  if(thereIsNewMessage())
  { // yeni mesaj var
    Serial.println("yeni mesaj var");
    String sendReadRequest = "";
    sendReadRequest += MessageReadString;
    int indexofMsgStr = inData.indexOf(NewMessageDEF);
    indexofMsgStr += NewMessageDEF.length();
    strWhichMsg = inData.substring(indexofMsgStr , indexofMsgStr+1); 
    sendReadRequest +=strWhichMsg;
    
    Serial.print("gonderilen komut = ");
    Serial.println(sendReadRequest);
    gsmSerial.println(sendReadRequest); // xxxx inci mesaji oku
  }else if(IsRinging())
  { // telefon caliyor
    Serial.println("telefon caliyor");
    AramayiMesguleCevir();    
    TAKEGPSDATA();
    KoordinatBilgisiGonder();
  }else if(ReadFromInbox())
  {
    /*
    takeMessageBody();
    processMessageBody();
    removeSms();*/
  }else
  {
    if(debug)Serial.println("ELSE");  
  }
  
  Serial.println("<----processData");  
}

void AramayiMesguleCevir(){
  gsmSerial.println("AT H");
  delay(50);
}


void takeMessageBody(){
if(debug)Serial.println("---->takeMessageBody");  
int index_of_NewLine = 0;

int indexofPhoneStr = inData.indexOf("\",\"+9");
indexofPhoneStr += 5;
ADMIN_PHONE_NUMBER = inData.substring(indexofPhoneStr , indexofPhoneStr+11); 
if(debug)
{
  Serial.print("ADMIN PHONE NUMBER ");
  Serial.println(ADMIN_PHONE_NUMBER);
}
  index_of_NewLine = inData.indexOf("REC UNREAD");  
  index_of_NewLine += 53;
  inData = inData.substring(index_of_NewLine,index_of_NewLine+29);

if(debug)
{
  Serial.println("Message Body ===>");
  Serial.println(inData);
  Serial.println("<----takeMessageBody");   
}
}

void processMessageBody(){

if(debug)Serial.println("---->processMessageBody");  
boolean sendstatus = false;
if(passwordIsCorrect()){
    if(DurumSoruyor())
    {
      if(debug)Serial.println("Durum Soruyor !");   
      sendstatus = true;            
    }
    else if(SifreDegistir())
    {
      //YENISIFRE12345678 20012001           
      PASSWORD = inData.substring(9,17);
      if(debug)
      {
        Serial.println("New Password =>");   
        Serial.println(PASSWORD);   
      }
      WriteNewPasswordToEEPROM();
    }
    
    if(sendstatus){

    }
}
else
{
  if(debug)Serial.println("Hatali Sifre!!");     
}

if(debug)Serial.println("<----processMessageBody");    
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
  gsmSerial.print(flat,6);
  gsmSerial.print("LON =");
  gsmSerial.print(flon,6);
  
  gsmSerial.write(26);

}


void removeSms(){
if(debug)Serial.println("---->removeSms");  
gsmSerial.println("AT+CMGD=1,4");
delay(100);
gsmSerial.write(26);
if(debug)Serial.println("<----removeSms");
}


/***********BOOLEAN IF ELSE FUNCTIONS*********/
/*****************START***********************/
int DurumSoruyor()
{
  if (inData.indexOf("DURUM") >= 0 )
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

int thereIsNewMessage()
{
  if (inData.indexOf(NewMessageDEF) >= 0 )
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
int passwordIsCorrect()
{
  if (inData.indexOf(PASSWORD) >= 0 )
  {
    return 1;
  }
  else
  {
    return 0;
  }
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
int SifreDegistir()
{
  if (inData.indexOf("YENISIFRE") >= 0 )
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

void WriteNewPasswordToEEPROM(){
  for( int cnt = 0; cnt < 8; cnt ++)
    EEPROM.write(cnt, PASSWORD.charAt(cnt));
}

void ReadPasswordFromEEPROM(){
  PASSWORD="";
  for( int cnt = 0; cnt < 8; cnt ++)
    PASSWORD += (char) EEPROM.read(cnt);
}

void callback()
{
  Serial.println("callback");    
  callback_counter++;
}