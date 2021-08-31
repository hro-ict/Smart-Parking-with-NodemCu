/*
 * 
 * 

CONNECTIONS
lcd display:
slc->d1;
sda->d2;
red_led entrance->d0
rode_led exit->d8
infrared sensor entrance->d6
infrared sensor exit->d7
servo entrance->d4
servo exit-> d5
*/



#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#define BLYNK_PRINT Serial
#include <SPI.h>
#include <BlynkSimpleEsp8266.h>
//#include <SimpleTimer.h>
#include <Wire.h>
#include <Servo.h> 
Servo myservo_in;
Servo myservo_uit;
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
#include <ESP_Mail_Client.h>
bool mail_state=true;
bool led_state=true;
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "email"
#define AUTHOR_PASSWORD "password"

////Mail text///////////////////
String text_1= "Dear driver thank you for your reservation. Your access code is: ";






SMTPSession smtp;
void smtpCallback(SMTP_Status status);
char auth[] = "COEjZju3Tc7oqCciVxkbnzHa-s1GavXJ";
// Ledjes//////////////////////////
int red_entrance= D0;
int red_exit=D8;
///////Servo motors//////////////
int open_entrance=60;
int close_exit=180;
int open_entrance=180;
int close_exit=60;
/////infrared sensors////////////
int sensor_entrance=D6;
int sensor_exit=D7;

//availabilty
int counter=4;
int entrance ;
int exit;
//reserving counter
int reserving=0;

//mail en code from BLYNK app
String mail=""; 
String code="";

String full="FULL";

//wifi 
const char* ssid = "SSID";      
const char* password = "password";


StaticJsonDocument<200> res_list;




   BLYNK_WRITE(V1) //writing to V0 from input test
{
    mail = param.asString(); 
 
}

   BLYNK_WRITE(V2) //writing to V0 from input test
{
    code = param.asString(); 
 
}


void setup() {
Serial.begin(9600);

lcd.begin();
myservo_in.attach(D4);
myservo_exit.attach(D5);
Blynk.begin(auth, ssid, password);
pinMode(red_in, OUTPUT);
pinMode(red_uit, OUTPUT);
digitalWrite(red_in,0);
myservo_in.write(close_entrance);
myservo_uit.write(close_exit);
}

void write_lcd(){
  //lcd.clear();
lcd.setCursor(0,0);
lcd.print("Welcome");
lcd.setCursor(0,1);
if (counter<=0){
  lcd.print("Availibilty: "+vol);
  //in=1;
  counter=0;
}
else if (counter>4){
  counter=4;
   myservo_exit.write(close_exit);
   
   
  }
else{
lcd.print("Availibilty: "+String(counter));
}
}

void loop() {
Blynk.run();
Blynk.virtualWrite(V5, counter);
in= digitalRead(sensor_entrance);
exit= digitalRead(sensor_exit);


Serial.print("LED STATE: "); Serial.println(led_state);
Serial.print("Access code:  ");Serial.println(code);


//write lcd scherm
lcd.setCursor(0,0);
lcd.print("Welcome");
lcd.setCursor(0,1);
if (counter<=0){
  lcd.print("Availibilty: "+full);
  in=1;
  counter=0;
  write_lcd();
}
else{
lcd.print("Availibilty: "+String(counter));
}

//when all places are taken
if (counter==0){
  counter=0;
  digitalWrite(red_in, 1);
  myservo_in.write(close_entrance);

  //if there is no reservation
  if (reserving==0){
    led_state=false;
  }
  //als er een reservering is
  if (reserving>0){
    led_state=true;  
  } 
  }

 //when the entrance sensor detects a movement
 if (in==0){
  counter-=1;
  digitalWrite(red_entrance, 1);
  digitalWrite(red_exit,1);
  myservo_in.write(open_entrance);
  write_lcd();
  Blynk.virtualWrite(V5, counter);
  delay(5000);
  digitalWrite(red_entrance, 0);
  digitalWrite(red_exit,0);
  led_state=true;
  myservo_in.write(close_in);
}

 //if the exit sensor detects a movement
if (uit==0 and counter!=4){
  digitalWrite(red_entrance, 1);
  digitalWrite(red_exit, 1);
  myservo_uit.write(open_exit);
  counter+=1;
  lcd.clear();
  write_lcd();
  
  Blynk.virtualWrite(V5, counter);
  delay(5000);
  digitalWrite(red_entrance, 0);
   digitalWrite(red_exit, 0);
   led_status=true;
  myservo_uit.write(close_exit);
}

//if there is a reservation request
if (mail.length()>6 and counter!=0){
  if (reserve<4 and reserve>=0){ 
    int res_code= random(10000,99999);
    send_mail(mail.c_str(),text+String(res_code));
    if (mail_state==true){
    reserving+=1;
    res_list[String(res_code)]="Reserve no: "+String(reserving);
    counter-=1;
     mail= ""; } 
     else
     mail_state=true;
    } 
    else
    mail="";
  }

//if user sends reservation code
if (code.length()==5){
  if (res_list.containsKey(code) and led_status==true){
    res_list.remove(code);
    reserving-=1;
    //counter+=1; 
    myservo_in.write(open_entrance);
    digitalWrite(red_entrance,1);
    digitalWrite(red_exit,1);
    Serial.println("Red on");
    write_lcd();
    delay(5000);
    Serial.println("Delay");
    code="";
    myservo_in.write(close_entrance);
    digitalWrite(red_entrance,0);
    digitalWrite(red_exit,1);
    led_state=true;
    
    }
  
  }

  serializeJson(res_list, Serial);
  Serial.println(res_list.size());
}


void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      //localtime_r(&result.timesstamp, &dt);

      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      Serial.printf("Recipient: %s\n", result.recipients);
      Serial.printf("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}

/////////////////////////////////////////////////////////


void send_mail(const char* client_mail,String mail_text){
  

  smtp.debug(1);


  smtp.callback(smtpCallback);


  ESP_Mail_Session session;

 
  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  //session.login.user_domain = "mydomain.net";

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "ESP Mail";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Reservering bevestigen HYM Smart Parking";
  message.addRecipient(" ", client_mail);

  String textMsg = mail_text;
  message.text.content = textMsg.c_str();


  message.text.charSet = "us-ascii";


  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Set the custom message header */
  message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message)){
    mail_status=false;
    Serial.println("Error sending Email, " + smtp.errorReason());
  }
  }
