/*				
*RTC moduel*
D2 RST -> white 
D5 SCL -> yellow 
D7 I/O -> brawn
VCC -> Red
GND -> Black

*SD card modeul*
GND : brawn
5V : Red
D10 CS -> Blue
D11 MOSI -> orange
D12 MISO -> yellow
D13 SCK -> green

*CO2 sensor with adapter*
SDA white -> A4 orange
SCL green -> A5 yellow
GND -> Black
5V -> Red


DHT22
DATA (정면기준 2번째 발) D3 -> brawn
5V -> red
GND -> black

LCD
GND -> brawn
5V -> Red
SDA -> A4 yellow
SCL -> A5 orange


*/
#include <cm1106_i2c.h>
#include <RtcDS1302.h>
#include <ThreeWire.h> 
#include <LiquidCrystal_I2C.h>
#include <Wire.h>  
#include <DHT.h>
#include <SD.h> // SD 라이브러리 추가

#define CM1107

#define dhtpin 3
#define dhttype DHT22
DHT dht(dhtpin, dhttype);

CM1106_I2C cm1106_i2c;

LiquidCrystal_I2C lcd(0x27, 16, 2);

ThreeWire myWire(7, 5, 2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

#define countof(a) (sizeof(a) / sizeof(a[0]))

File myFile; // 파일 객체 생성


String getDateTimeString(const RtcDateTime& dt) {
    char datestring[20];
    snprintf_P(datestring, countof(datestring), PSTR("%02u/%02u %02u:%02u:%02u"), 
    dt.Month(), dt.Day(), dt.Hour(), dt.Minute(), dt.Second());
    return String(datestring);
}


void setup() {
  cm1106_i2c.begin();
  cm1106_i2c.read_serial_number();
  cm1106_i2c.check_sw_version();
  dht.begin();
  Serial.begin(9600);
  lcd.init();        // LCD 초기화
  lcd.backlight();   // 백라이트 켜기

  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.print(getDateTimeString(compiled));
  Serial.println();

  if (!Rtc.IsDateTimeValid())  {
    Rtc.SetDateTime(compiled);  }

  if (Rtc.GetIsWriteProtected())  {
    Rtc.SetIsWriteProtected(false);  }

  if (!Rtc.GetIsRunning())  {
    Rtc.SetIsRunning(true);  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)  {
  Rtc.SetDateTime(compiled);  }
}

void loop() {
  uint8_t ret = cm1106_i2c.measure_result();

  RtcDateTime now = Rtc.GetDateTime();
  Serial.print("CHECK : ");
  Serial.print(getDateTimeString(now  + 8));

  String dateTimeStr = getDateTimeString(now + 8); // 현재 시간을 문자열로 변환합니다.

  if (!now.IsValid())  {
    Serial.println("RTC lost confidence in the DateTime!");
  }

  // LCD에 현재 날짜와 시간 출력
  lcd.setCursor(0, 0); // LCD의 첫 번째 줄, 첫 번째 위치로 커서를 설정
  lcd.print(dateTimeStr); // 변환된 날짜와 시간 문자열을 LCD에 출력

  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  Serial.print(" | CO2 : ");
  Serial.print(cm1106_i2c.co2);
  Serial.print(" | RH : ");
  Serial.print(h);
  Serial.print(" | T : ");
  Serial.println(t);

  lcd.setCursor(0, 1); // 커서 위치 설정  
  lcd.print("P:"); // PPM 레이블 출력
  lcd.print("    ");
  lcd.setCursor(2, 1);
  lcd.print(cm1106_i2c.co2); // PPM 값 출력

  lcd.setCursor(6, 1); // 커서 위치 설정
  lcd.print("|"); // PPM 레이블 출력
  lcd.print(h); // PPM 값 출력

  lcd.setCursor(11, 1); // 커서 위치 설정
  lcd.print("|"); // PPM 레이블 출력  
  lcd.print(t); // PPM 값 출력

 
if (SD.begin(10)) {
    Serial.println("SD 카드 초기화 성공");
    myFile = SD.open("123.txt", FILE_WRITE);

    lcd.setCursor(14, 0); 
    lcd.print("|O");
    myFile.print(dateTimeStr); // 날짜와 시간을 SD 카드에 기록합니다.
    myFile.print(" | PPM : ");
    myFile.print(cm1106_i2c.co2);
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



  delay(420);

  }
