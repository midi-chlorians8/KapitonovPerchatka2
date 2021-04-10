#include <Arduino.h>


#include "SdFunc.h"

// Блок MPU 6050

#include <Wire.h>
const int MPU_addr = 0x69; // I2C-адрес MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
int16_t oldAcX=0, oldAcY=0, oldAcZ=0;
int16_t Max; // Максимальное ускорение или по оси X или по оси Y или по оси Z 
#define TIME_DELAY_MPU_READ 20


// Блок MPU 6050

// Блок Ds3231
#include "RTClib.h"
RTC_DS3231 rtc;
String string_data_time;
// Блок Ds3231

void setup(){
    Serial.begin(115200);
    
    // Блок карты Сд
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        //NoReadSdCard = true;
        pinMode(13,OUTPUT);
        digitalWrite(13,HIGH);
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
/*
    listDir(SD, "/", 0);
    createDir(SD, "/mydir");
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");
    
    listDir(SD, "/", 2);
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    readFile(SD, "/hello.txt");
    deleteFile(SD, "/Log.txt");
*/
   // deleteFile(SD, "/Log.txt");
 
    appendFile(SD, "/Log.txt", "Esp32 Sniffer Mac Data list:\n");
    //renameFile(SD, "/hello.txt", "/Log.txt");
    readFile(SD, "/Log.txt");
    //testFileIO(SD, "/test.txt");
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n",  SD.usedBytes() / (1024 * 1024));
    /*
    appendFile(SD, "/Log.txt", "MyAdd\n");
    appendFile(SD, "/Log.txt", "MyNewAdd\n");
    appendFile(SD, "/Log.txt", "jjjjjjjjjjjjjjjjjjjj111111111111111111jjjjjjjjjjjjjjjjjjjj\n");
    */

   /*
    String myString;
    myString = "Bur\n";
    
    char* charBuf=new char[myString.length()];
    myString.toCharArray(charBuf, myString.length());
    appendFile(SD, "/Log.txt", charBuf); // Добавить к файлу

    delete[] charBuf; // Отчистить выделенную память в куче

    appendFile(SD, "/Log.txt", "\n"); // Перенос строки


    myString = "ChakaChaka2\n";
    charBuf=new char[myString.length()];
    myString.toCharArray(charBuf, myString.length());

    appendFile(SD, "/Log.txt", charBuf);

    readFile(SD, "/Log.txt");
*/
    // Блок карты Сд

// Блок MPU 6050
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // регистр PWR_MGMT_1
  Wire.write(0);     // установить в ноль (будит MPU-6050)
  Wire.endTransmission(true);
  Serial.println("Wrote to IMU");
// Блок MPU 6050

// Блок Ds3231
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
 // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  // Блок Ds3231

}

void WriteOnSd();
void mpu_read();
void processing();
void getTime();

void loop(){

mpu_read();
processing();
getTime();
  WriteOnSd();
 
}



void WriteOnSd(){
  // Блок запись на карту micro sd
  String myString;
    myString = "Tiime="+string_data_time+
     ", AccselMax="+
    String(Max);
    Serial.print("myString:");Serial.println(myString);
    
    char* charBuf=new char[myString.length()];
    myString.toCharArray(charBuf, myString.length());
    appendFile(SD, "/Log.txt", charBuf); // Добавить к файлу

    delete[] charBuf; // Отчистить выделенную память в куче

    appendFile(SD, "/Log.txt", "\n"); // Перенос строки
    // Блок запись на карту micro sd
}



// Блок MPU 6050


void mpu_read() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // начинаем с регистра 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // запросить всего 14 регистров
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  //Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
 // Serial.print("Accelerometer Values: \n");
 // Serial.print("AcX: "); Serial.print(AcX); Serial.print("\tAcY: "); Serial.print(AcY); Serial.print("\tAcZ: "); Serial.println(AcZ);
  //Serial.print("\nTemperature: " );  Serial.print(Tmp);
 // Serial.print("\nGyroscope Values: \n");
 // Serial.print("GyX: "); Serial.print(GyX); Serial.print("\nGyY: "); Serial.print(GyY); Serial.print("\nGyZ: "); Serial.print(GyZ);
 // Serial.print("\n");
   delay(TIME_DELAY_MPU_READ);
}
void processing(){
  //static unsigned long timing; 
  //if (millis() - timing > 10){ // Вместо 10000 подставьте нужное вам значение паузы 
    if(oldAcX != AcX or oldAcY != AcY or oldAcZ != AcZ ){

      int16_t arr[3];
      arr[0] = abs(AcX-oldAcX);
      arr[1] = abs(AcY-oldAcY);
      arr[2] = abs(AcZ-oldAcZ);
      Max = arr[0];
      
      for(int i=0; i<3;i++){
          if(arr[i]>Max){
              Max=arr[i];
           }
      }
      Serial.println(Max);
      
      oldAcX = AcX;
      oldAcY = AcY;
      oldAcZ = AcZ;
    }
    
//    timing = millis(); 
// }
}


// Блок MPU 6050

void getTime(){
      DateTime now = rtc.now();
      string_data_time = String(now.day() )+'/'+
                         String(now.month() )+'/'+
                         String(now.year() )  
                         +'/t'+
                         String(now.hour() )+':'+
                         String(now.minute() )+':'+
                         String(now.second() );
                         
                         Serial.print("string_data_time:");Serial.println(string_data_time);

}
























/*


+
    String(ppkt->rx_ctrl.rssi)+
    ", ADDR1="+
    String(hdr->addr1[0], HEX)+":"+String(hdr->addr1[1], HEX)+":"+String(hdr->addr1[2], HEX)+":"+String(hdr->addr1[3], HEX)+":"+String(hdr->addr1[4], HEX)+":"+String(hdr->addr1[5], HEX   )+
    ", ADDR2="+
    String(hdr->addr2[0], HEX)+":"+String(hdr->addr2[1], HEX)+":"+String(hdr->addr2[2], HEX)+":"+String(hdr->addr2[3], HEX)+":"+String(hdr->addr2[4], HEX)+":"+String(hdr->addr2[5], HEX   )+
    ", ADDR3="+
    String(hdr->addr3[0], HEX)+":"+String(hdr->addr3[1], HEX)+":"+String(hdr->addr3[2], HEX)+":"+String(hdr->addr3[3], HEX)+":"+String(hdr->addr3[4], HEX)+":"+String(hdr->addr3[5], HEX   )+
    " "


    */