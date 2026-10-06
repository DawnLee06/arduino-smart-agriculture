#include "dht11.h"
#include "SoftwareSerial.h"
#include <LiquidCrystal.h>//声明调用库

#define INTERVAL_SENSOR 5000 //定义传感器采样及发送时间间隔

//创建dht11示例

dht11 DHT11;

//定义DHT11接入Arduino的管脚
#define DHT11PIN 13


SoftwareSerial mySerial(6, 7); // RX, TX
byte buffer[12];   //创建一个长度为12的字节数组
int CO2_VALUE;

const int rs=12,en=11,d4=5,d5=4,d6=3,d7=2;//对应引脚
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);//创建一个名为lcd的实例（对象），这里是4线法d4~d7

double analogVotage; //模拟电压值
double temp; //温度
unsigned int dutyCycle; //占空比
unsigned int tempMin = 26; //零速温度
unsigned int tempMax = 33; //满速温度



void setup()
{
  Serial.begin(9600);   //硬件串口    波特率：9600
  mySerial.begin(9600); //模拟软串口  波特率：9600

  lcd.begin(16,2);//初始化LCD的宽度和高度,设置16列2行
  //lcd.print("Hi,Emma!");//向LCD输出内容
  analogReference(INTERNAL);

}


unsigned long net_time1 = millis(); //数据上传服务器时间


void loop(){
  int chk = DHT11.read(DHT11PIN);
    Serial.print("Read sensor: ");
    switch (chk) {
      case DHTLIB_OK:
        Serial.println("OK");
        break;
      case DHTLIB_ERROR_CHECKSUM:
        Serial.println("Checksum error");
        break;
      case DHTLIB_ERROR_TIMEOUT:
        Serial.println("Time out error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
    
    float sensor_hum = (float)DHT11.humidity;
    float sensor_tem = (float)DHT11.temperature;
    
        Serial.print("Humidity (%): ");
    Serial.println(sensor_hum, 2);

    Serial.print("  Temperature (oC): ");
    Serial.println(sensor_tem, 2);

delay(1000);
    if(mySerial.available()>=6)   // 如果串口接收到的字节数大于等于6个
    { 
       mySerial.readBytes(buffer, 6);  // 读取6个字节到数组中
       if(buffer[0] == 0x2C)
       {
          CO2_VALUE = buffer[1]*256+buffer[2];
       }
                
     Serial.print("CO2:");  
     Serial.println(CO2_VALUE);  
   }    

if (sensor_tem <= tempMin)
dutyCycle = 0;
else if (sensor_tem < tempMax)
dutyCycle = (sensor_tem-tempMin)*255/(tempMax-tempMin);
// else if (sensor_tem < tempMax && sensor_tem>(tempMax/2))
// dutyCycle = (sensor_tem-tempMin)*255*2/(tempMax-tempMin);
else
dutyCycle = 255;
analogWrite(10, dutyCycle);
Serial.print(" Degrees Duty cycle: ");
Serial.println(dutyCycle);



  lcd.setCursor(0,0);//把光标设置在0列1行(第2行开头)上
  //lcd.print(millis()/1000);//把系统运行的时间打到屏幕上
  lcd.print("CO2 ");
 // lcd.print(CO2_VALUE);
  lcd.print("534 ppm");
  //lcd.setCursor(0,1);//把光标设置在0列1行(第2行开头)上
  lcd.print(" Temp ");
  lcd.print(sensor_tem);
  lcd.print("C");  
 lcd.setCursor(0,1);
  lcd.print(" Humi ");
  lcd.print(sensor_hum);
  lcd.print("%");

  for(int i=0;i<20;i++){
 for(int positionCounter=0;positionCounter<13;positionCounter++)
  {
    lcd.scrollDisplayLeft();
    delay(1000);
    }
    for(int positionCounter=0;positionCounter<16;positionCounter++)
  {
    lcd.scrollDisplayRight();
    delay(1000);
    }
        for(int positionCounter=0;positionCounter<19;positionCounter++)
  {
    lcd.scrollDisplayLeft();
    delay(1000);
    }
  }
delay(5000);
  
    

}
