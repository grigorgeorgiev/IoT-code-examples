#include "WiFiClientSecure.h"
#include "Camera_Exp.h"
CAMERA cam;
char ssid[] = "";
char pass[] = "";  
#define SENSOR 19
#define SERVER     ""
#define PORT     443
#define BOUNDARY     "--------------------------133747188241686651551404"  
#define TIMEOUT      20000

void setup() 
{
 pinMode(SENSOR,INPUT);
  Serial.begin(115200);
  Serial.println("\r\nHello Line Notify");
  cam.setFrameSize(CAMERA_FS_QVGA);
  cam.setMirror(false);
  cam.setVflip(false);
  cam.setWhiteBalance(true);
  esp_err_t err = cam.init();
  if (err != ESP_OK)
  {
    Serial.println("Camera init failed with error =" + String( err));
    return;
  }
  WiFi.begin(ssid, pass);
  unsigned char led_cnt=0;
  while (WiFi.status() != WL_CONNECTED) 
  {
     Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() 
{
    while(!digitalRead(SENSOR)){
      Serial.println("Undetect");
    }
  String res;
   if(digitalRead(SENSOR))
   {
      Serial.println("Send Picture");
      esp_err_t err;
      err = cam.capture();
      if (err == ESP_OK)
      {
        res = sendImage("asdw","A54S89EF5",cam.getfb(),cam.getSize());
        Serial.println(res);
      }
      else
        Serial.println("Camera Error");

        while(digitalRead(SENSOR));
   }
}

//////

String sendImage(String token,String message, uint8_t *data_pic,size_t size_pic)
{
  String bodyTxt =  body("message",message);
  String bodyPic =  body("imageFile",message);
  String bodyEnd =  String("--")+BOUNDARY+String("--\r\n");
  size_t allLen = bodyTxt.length()+bodyPic.length()+size_pic+bodyEnd.length();
  String headerTxt =  header(token,allLen);
  WiFiClientSecure client;
   if (!client.connect(SERVER,PORT)) 
   {
    return("connection failed");   
   }

   client.print(headerTxt+bodyTxt+bodyPic);
   client.write(data_pic,size_pic);
   client.print("\r\n"+bodyEnd);

   delay(20);
   long tOut = millis() + TIMEOUT;
   while(client.connected() && tOut > millis()) 
   {
    if (client.available()) 
    {
      String serverRes = client.readStringUntil('\r');
        return(serverRes);
    }
   }
}
String header(String token,size_t length)
{
  String  data;
      data =  F("POST /ln/bot.php HTTP/1.1\r\n");
      data += F("cache-control: no-cache\r\n");
      data += F("Content-Type: multipart/form-data; boundary=");
      data += BOUNDARY;
      data += "\r\n";
      data += F("User-Agent: PostmanRuntime/6.4.1\r\n");
      data += F("Accept: */*\r\n");
      data += F("Host: ");
      data += SERVER;
      data += F("\r\n");
      data += F("accept-encoding: gzip, deflate\r\n");
      data += F("Connection: keep-alive\r\n");
      data += F("content-length: ");
      data += String(length);
      data += "\r\n";
      data += "\r\n";
    return(data);
}
String body(String content , String message)
{
  String data;
  data = "--";
  data += BOUNDARY;
  data += F("\r\n");
  if(content=="imageFile")
  {
    data += F("Content-Disposition: form-data; name=\"imageFile\"; filename=\"picture.jpg\"\r\n");
    data += F("Content-Type: image/jpeg\r\n");
    data += F("\r\n");
  }
  else
  {
    data += "Content-Disposition: form-data; name=\"" + content +"\"\r\n";
    data += "\r\n";
    data += message;
    data += "\r\n";
  }
   return(data);
}
