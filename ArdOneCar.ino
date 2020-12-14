//引入ESP8266.h头文件，建议使用教程中修改后的文件
#include "ESP8266.h"
//#include "dht11.h"
#include "SoftwareSerial.h"
#include "AFMotor.h"

//红外
#include <Servo.h>
const int trigPin = A5;
const int echoPin = A4;
// Variables for the duration and the distance
long duration;
int distance;
Servo myServo;  // create servo object to control a servo

AF_DCMotor motor1(1,MOTOR12_64KHZ);
AF_DCMotor motor2(2,MOTOR12_64KHZ);
AF_DCMotor motor3(3,MOTOR12_64KHZ);
AF_DCMotor motor4(4,MOTOR12_64KHZ);

//配置ESP8266WIFI设置
#define SSID "r"    //填写2.4GHz的WIFI名称，不要使用校园网
#define PASSWORD "456456456"//填写自己的WIFI密码
#define HOST_NAME "api.heclouds.com"  //API主机名称，连接到OneNET平台，无需修改
#define DEVICE_ID "642995351"       //填写自己的OneNet设备ID
#define HOST_PORT (80)                //API端口，连接到OneNET平台，无需修改
String APIKey = "GR6=PYb0DPL8Zt2i6dYJm0PCR78="; //与设备绑定的APIKey

#define INTERVAL_SENSOR 5000 //定义传感器采样及发送时间间隔

//创建dht11示例
//
//dht11 DHT11;
//
////定义DHT11接入Arduino的管脚
//#define DHT11PIN 4

//定义ESP8266所连接的软串口
/*********************
 * 该实验需要使用软串口
 * Arduino上的软串口RX定义为D3,
 * 接ESP8266上的TX口,
 * Arduino上的软串口TX定义为D2,
 * 接ESP8266上的RX口.
 * D3和D2可以自定义,
 * 但接ESP8266时必须恰好相反
 *********************/
SoftwareSerial mySerial(A3, A2);
ESP8266 wifi(mySerial);

int pos = 0;    // variable to store the servo position

void setup()
{
  mySerial.begin(115200); //初始化软串口
  Serial.begin(9600);     //初始化串口
  Serial.print("setup begin\r\n");
  analogReference(INTERNAL);  //调用板载1.1V基准源
  //以下为ESP8266初始化的代码
  Serial.print("FW Version: ");
  Serial.println(wifi.getVersion().c_str());

  if (wifi.setOprToStation()) {
    Serial.print("to station ok\r\n");
  } else {
    Serial.print("to station err\r\n");
  }

  //ESP8266接入WIFI
  if (wifi.joinAP(SSID, PASSWORD)) {
    Serial.print("Join AP success\r\n");
    Serial.print("IP: ");
    Serial.println(wifi.getLocalIP().c_str());
  } else {
    Serial.print("Join AP failure\r\n");
  }

//Serial.println("");
//  Serial.print("DHT11 LIBRARY VERSION: ");
//  Serial.println(DHT11LIB_VERSION);

  mySerial.println("AT+UART_CUR=9600,8,1,0,0");
  mySerial.begin(9600);
  Serial.println("setup end\r\n");

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(9600);
 myServo.attach(9);  // attaches the servo on pin 9 to the servo object

motor1.setSpeed(200);
motor2.setSpeed(200);
motor3.setSpeed(200);
motor4.setSpeed(200);
}

unsigned long net_time1 = millis(); //数据上传服务器时间


void loop(void){
 motor1.run(FORWARD);//FORWARD
delay(1000);
motor2.run(BACKWARD);//BACKWARD
delay(1000);
motor3.run(FORWARD);
delay(1000);
motor4.run(BACKWARD);
delay(1000); 
    
 for(int i=0;i<=180;i++){  
  myServo.write(i);
  delay(30);
  distance = calculateDistance();// Calls a function for calculating the distance measured by the Ultrasonic sensor for each degree
  Serial.print(i); // Sends the current degree into the Serial Port
  Serial.print(",distance"); // Sends addition character right next to the previous value needed later in the Processing IDE for indexing
  Serial.print(distance); // Sends the distance value into the Serial Port
  Serial.println("."); // Sends addition character right next to the previous value needed later in the Processing IDE for indexing
  delay(1000); // 控制刷新速度
  }
for(int i=180;i>0;i--){  
  myServo.write(i);
  delay(30);
distance = calculateDistance();
  Serial.print(i);
  Serial.print(",");
  Serial.print(distance);
  Serial.println(".");
  delay(1000); // 控制刷新速度
  }

  
  if (net_time1 > millis())
    net_time1 = millis();

  if (millis() - net_time1 > INTERVAL_SENSOR) //发送数据时间间隔
  {



  
    if (wifi.createTCP(HOST_NAME, HOST_PORT)) { //建立TCP连接，如果失败，不能发送该数据
      Serial.print("create tcp ok\r\n");
      char buf[10];
      //拼接发送data字段字符串
      String jsonToSend = "{\"Temperature\":";
      dtostrf(distance, 1, 2, buf);
      jsonToSend += "\"" + String(buf) + "\"";
      jsonToSend += ",\"Distance\":";
      dtostrf(distance, 1, 2, buf);
      jsonToSend += "\"" + String(buf) + "\"";
      jsonToSend += "}";

      //拼接POST请求字符串
      String postString = "POST /devices/";
      postString += DEVICE_ID;
      postString += "/datapoints?type=3 HTTP/1.1";
      postString += "\r\n";
      postString += "api-key:";
      postString += APIKey;
      postString += "\r\n";
      postString += "Host:api.heclouds.com\r\n";
      postString += "Connection:close\r\n";
      postString += "Content-Length:";
      postString += jsonToSend.length();
      postString += "\r\n";
      postString += "\r\n";
      postString += jsonToSend;
      postString += "\r\n";
      postString += "\r\n";
      postString += "\r\n";

      const char *postArray = postString.c_str(); //将str转化为char数组

      Serial.println(postArray);
      wifi.send((const uint8_t *)postArray, strlen(postArray)); //send发送命令，参数必须是这两种格式，尤其是(const uint8_t*)
      Serial.println("send success");
      if (wifi.releaseTCP()) { //释放TCP连接
        Serial.print("release tcp ok\r\n");
      } else {
        Serial.print("release tcp err\r\n");
      }
      postArray = NULL; //清空数组，等待下次传输数据
    } else {
      Serial.print("create tcp err\r\n");
    }

    Serial.println("");

    net_time1 = millis();
  }
}
int calculateDistance(){ 
  
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance= duration*0.034/2;
  return distance;
}
