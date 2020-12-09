#include<WiFiClientSecure.h>
    #include "esp_camera.h"
    #include "esp_timer.h"
    #include "img_converters.h"
    #include "Arduino.h"
    #include "fb_gfx.h"
    #include "fd_forward.h"
    #include "fr_forward.h"
    #include "FS.h"               
    #include "SD_MMC.h"           
    #include "soc/soc.h"           
    #include "soc/rtc_cntl_reg.h"  
    #include "driver/rtc_io.h"

    #define PWDN_GPIO_NUM     32
    #define RESET_GPIO_NUM    -1
    #define XCLK_GPIO_NUM      0
    #define SIOD_GPIO_NUM     26
    #define SIOC_GPIO_NUM     27
    #define Y9_GPIO_NUM       35
    #define Y8_GPIO_NUM       34
    #define Y7_GPIO_NUM       39
    #define Y6_GPIO_NUM       36
    #define Y5_GPIO_NUM       21
    #define Y4_GPIO_NUM       19
    #define Y3_GPIO_NUM       18
    #define Y2_GPIO_NUM        5
    #define VSYNC_GPIO_NUM    25
    #define HREF_GPIO_NUM     23
    #define PCLK_GPIO_NUM     22

    const char* ssid = "***";
    const char* password = "****";

    const char* host = "192.168.3.19";
    const int Port = 4567;
    const char* boundry = "dgbfhfh";

    WiFiClientSecure client;

    void setup() 
    {
         WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

         Serial.begin(115200);

         pinMode(4,OUTPUT);

         Serial.printf("Connecting to the Wifi [%s]...\r\n", ssid);
         WiFi.begin(ssid,password);
         while (WiFi.status() != WL_CONNECTED) 
         {
                delay(500);
                Serial.print(".");
         }
         Serial.println("WiFi connected");

         camera_config_t config;
         config.ledc_channel = LEDC_CHANNEL_0;
         config.ledc_timer = LEDC_TIMER_0;
         config.pin_d0 = Y2_GPIO_NUM;
         config.pin_d1 = Y3_GPIO_NUM;
         config.pin_d2 = Y4_GPIO_NUM;
         config.pin_d3 = Y5_GPIO_NUM;
         config.pin_d4 = Y6_GPIO_NUM;
         config.pin_d5 = Y7_GPIO_NUM;
         config.pin_d6 = Y8_GPIO_NUM;
         config.pin_d7 = Y9_GPIO_NUM;
         config.pin_xclk = XCLK_GPIO_NUM;
         config.pin_pclk = PCLK_GPIO_NUM;
         config.pin_vsync = VSYNC_GPIO_NUM;
         config.pin_href = HREF_GPIO_NUM;
         config.pin_sscb_sda = SIOD_GPIO_NUM;
         config.pin_sscb_scl = SIOC_GPIO_NUM;
         config.pin_pwdn = PWDN_GPIO_NUM;
         config.pin_reset = RESET_GPIO_NUM;
         config.xclk_freq_hz = 20000000;
         config.pixel_format = PIXFORMAT_JPEG; 

        if(psramFound())
        {
           config.frame_size = FRAMESIZE_SXGA; // FRAMESIZE_                                 +QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
          config.jpeg_quality = 10;
          config.fb_count = 2;
        } 
        else 
        {
           config.frame_size = FRAMESIZE_VGA;
           config.jpeg_quality = 12;
           config.fb_count = 1;
        }

      // Initialise Camera
      esp_err_t err = esp_camera_init(&config);
      if (err != ESP_OK) 
      {
           Serial.printf("Camera init failed with error 0x%x", err);
           return;
      }

      sendPhotoToServer(); 
    }

    void sendPhotoToServer()
    {
         Serial.printf("Connecting to %s:%d... ", host,Port);

         if (!client.connect(host,Port)) 
         {      
           Serial.println("Failure in connection with the server");      
           return;
         }

         sendDataToServer();
     }

     void sendDataToServer()
     {
        String start_request = "";
        String end_request = "";

        start_request = start_request + "--" + boundry + "\r\n";

        start_request = start_request + "Content-Disposition: form-data; name=\"file\"; filename=\"CAM.jpg\"\r\n";
        start_request = start_request + "Content-Type: image/jpg\r\n";
        start_request = start_request + "\r\n";

        end_request = end_request + "\r\n";


        end_request = end_request + "--" + boundry + "--" + "\r\n";

        /************************************************/  
        digitalWrite(4,HIGH);
        camera_fb_t * fb = NULL;
        // Take Picture with Camera
        fb = esp_camera_fb_get();  
        if(!fb) 
         {
           Serial.println("Camera capture failed");
           return;
         }
         digitalWrite(4,LOW);

         delay(1000);
        /************************************************/     

         int contentLength = (int)fb->len + start_request.length() + end_request.length();    

         String headers = String("POST https://192.168.3.19:4567/api/upload HTTP/1.1\r\n");
         headers = headers + "Host: " + host + "\r\n";
         headers = headers + "User-Agent: ESP32" + "\r\n";
         headers = headers + "Accept: */*\r\n";
         headers = headers + "Content-Type: multipart/form-data; boundary=" + boundry + "\r\n";
         headers = headers + "Content-Length: " + contentLength + "\r\n";
         headers = headers + "\r\n";
         headers = headers + "\r\n";

         Serial.println();
         Serial.println("Sending data to Server...");        

         Serial.print(headers);        
         client.print(headers);
         client.flush();

         Serial.print(start_request);
         client.print(start_request);
         client.flush();

       /*****************************************************/
       int iteration = fb->len / 1024;
       for(int i=0; i<iteration; i++)
       {
         client.write(fb->buf,1024);
         fb->buf += 1024;
         client.flush();
       }
       size_t remain = fb->len % 1024;
       client.write(fb->buf,remain);
       client.flush();
       /****************************************************/  

       Serial.print(end_request);
       client.print(end_request);
       client.flush();

       esp_camera_fb_return(fb);
      }

     void loop() 
      {

      }
