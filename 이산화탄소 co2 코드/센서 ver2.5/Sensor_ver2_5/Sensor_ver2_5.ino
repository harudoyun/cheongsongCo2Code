/*
RTC moduel
D2 -> RST
D5 CLK
D7 DAT

SD card modeul
D4 CS
D11 MOSI
D12 MISO 
D13 SCK

CO2 sensor
D9 RX
D8 TX

DHT22
DATA 정면기준 2번째 발
D3

LCD
SDA A4
SCL A5
*/
#include <Wire.h>  
#include <RtcDS3231.h> // DS3231 라이브러리로 변경
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <DHT.h>
#include <SD.h> // SD 라이브러리 추가

#define dhtpin 3
#define dhttype DHT22
DHT dht(dhtpin, dhttype);

LiquidCrystal_I2C lcd(0x27, 16, 2);

RtcDS3231<TwoWire> Rtc(Wire); // DS3231 사용을 위한 객체 생성


#define countof(a) (sizeof(a) / sizeof(a[0]))

File myFile; // 파일 객체 생성

SoftwareSerial mySerial(8, 9); //rt11 tx13
unsigned char Send_data[4] = {0x11, 0x01, 0x01, 0xED};
unsigned char Receive_Buff[8];
unsigned char recv_cnt = 0;
unsigned int PPM_Value;

void Send_CMD(void) {
  unsigned int i;
  for (i = 0; i < 4; i++) {
    mySerial.write(Send_data[i]);
    delay(1);
  }
}

unsigned char Checksum_cal(void) {
  unsigned char count, SUM = 0;
  for (count = 0; count < 7; count++) {
    SUM += Receive_Buff[count];
  }
  return 256 - SUM;
}

String getDateTimeString(const RtcDateTime& dt) {
    char datestring[26];
    snprintf_P(datestring, countof(datestring), PSTR("%02u/%02u %02u:%02u:%02u"), 
    dt.Month(), dt.Day(), dt.Hour(), dt.Minute(), dt.Second());
    return String(datestring);
}


void setup() {
  pinMode(8, INPUT);
  pinMode(9, OUTPUT);
  dht.begin();
  Serial.begin(9600);
  mySerial.begin(9600);
  while (!mySerial);
  lcd.init();        // LCD 초기화
  lcd.backlight();   // 백라이트 켜기
  Wire.begin(); // DS3231을 위한 I2C 통신 시작
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.print(getDateTimeString(compiled));
  Serial.println();

  if (!Rtc.IsDateTimeValid())  {
    Rtc.SetDateTime(compiled);  }

  if (!Rtc.GetIsRunning())  {
    Rtc.SetIsRunning(true);  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)  {
    Rtc.SetDateTime(compiled);  }
}

void loop() {

  RtcDateTime now = Rtc.GetDateTime();
  Serial.print("CHECK : ");
  Serial.print(getDateTimeString(now + 9));

  String dateTimeStr = getDateTimeString(now + 9); // 현재 시간을 문자열로 변환합니다.

  if (!now.IsValid())  {
    Serial.println("RTC lost confidence in the DateTime!");
  }

  // LCD에 현재 날짜와 시간 출력
  lcd.setCursor(0, 0); // LCD의 첫 번째 줄, 첫 번째 위치로 커서를 설정
  lcd.print(dateTimeStr); // 변환된 날짜와 시간 문자열을 LCD에 출력

  Send_CMD();
  while (1) {
    if (mySerial.available()) {
      Receive_Buff[recv_cnt++] = mySerial.read();
      if (recv_cnt == 8) {
        recv_cnt = 0;
        break;
      }
    }
  }

  if (Checksum_cal() == Receive_Buff[7]) {
    PPM_Value = Receive_Buff[3] << 8 | Receive_Buff[4];
    Serial.write(" | PPM : ");
    Serial.print(PPM_Value);
  } else {
    Serial.write("CHECKSUM Error");
  }

  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  Serial.print(" | RH : ");
  Serial.print(h);
  Serial.print(" | T : ");
  Serial.println(t);

  lcd.setCursor(0, 1); // 커서 위치 설정  
  lcd.print("P:"); // PPM 레이블 출력
  lcd.print("    ");
  lcd.setCursor(2, 1);
  lcd.print(PPM_Value); // PPM 값 출력

  lcd.setCursor(6, 1); // 커서 위치 설정
  lcd.print("|"); // PPM 레이블 출력
  lcd.print(h); // PPM 값 출력

  lcd.setCursor(11, 1); // 커서 위치 설정
  lcd.print("|"); // PPM 레이블 출력  
  lcd.print(t); // PPM 값 출력

 
if (SD.begin(4)) {
    Serial.println("SD 카드 초기화 성공");
    myFile = SD.open("123.txt", FILE_WRITE);

    lcd.setCursor(14, 0); 
    lcd.print("|O");
    myFile.print(dateTimeStr); // 날짜와 시간을 SD 카드에 기록합니다.
    myFile.print(" | PPM : ");
    myFile.print(PPM_Value);
    myFile.print(" | RH : ");
    myFile.print(h);
    myFile.print(" | T : ");
    myFile.println(t);
    myFile.close();
} else {
    lcd.setCursor(14, 0); 
    lcd.print("|X");
    Serial.println("SD 카드 초기화 실패");
}



  delay(1000);

  }