#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <HDC2080.h>
#include <Adafruit_SSD1306.h>
#include "time.h"

int pin = 2;
String CurrentTime, CurrentDate, CurrentWeek;  //宣告字串變數

// 設定HDC2080
#define ADDR 0x40
HDC2080 sensor(ADDR);

float temperature = 0, humidity = 0;

// 設定WIFI
const char* ssid = "xxx";  //無線分享器的名稱
const char* password = "xxx";    //密碼

const char* ntpServer = "pool.ntp.org";  //從pool.ntp.org請求時間
  // Worldwide : pool.ntp.org
  // Asia : asia.pool.ntp.org
  // Europe : europe.pool.ntp.org
  // North America : north-america.pool.ntp.org
  // South America : south-america.pool.ntp.org
  // Oceania : oceania.pool.ntp.org

  // Set offset time in seconds to adjust for your timezone, for example:
  // 世界標準時間(UTC) / 格林威治時間(GMT)
  // UTC/GMT 0 = 0
  // UTC/GMT +1 = 3600
  // UTC/GMT +8 = 28800
const long  gmtOffset_sec = 8 * 3600;  //台灣時區要用28800
const int   daylightOffset_sec = 0;  //定義夏令時的偏移量（以秒為單位）。一般是一小時，相當於3600秒

// 設定OLED
#define SCREEN_WIDTH 128 // OLED 寬度像素
#define SCREEN_HEIGHT 64 // OLED 高度像素
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);  //set UART Baud rate

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);

  //WiFi.mode(val);
  //WIFI_OFF : 關閉網路，可用於網路不正常時，重啟網路
  //WIFI_STA : 以工作站（Station）模式啟動，用來上網讀取資料，此為預設模式
  //WIFI_AP : 以熱點（Access Point）模式啟動，讓其他裝置連入ESP32
  //WIFI_AP_STA : 混合模式，同時當熱點也當作工作站
  Serial.print("Connecting to ");  // Connect to Wi-Fi
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);  //設定關閉STA模式
  WiFi.begin(ssid, password);  //WIFI連線
  while (WiFi.status() != WL_CONNECTED) {  // 等待WiFi連線
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  digitalWrite(pin, HIGH);

  // Init and get the time
  // 從網路時間服務器上獲取並設定時間
  // 獲取成功後晶片會使用RTC時鐘保持時間的更新
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  //使用之前定義的設置配置時間
  //printLocalTime();  //使用printLocalTime()函數在串口監視器中打印時間

  // Initialize I2C communication
  sensor.begin();

  // Begin with a device reset
  sensor.reset();

  // Set up the comfort zone
  sensor.setHighTemp(28);         // High temperature of 28C
  sensor.setLowTemp(22);          // Low temperature of 22C
  sensor.setHighHumidity(55);     // High humidity of 55%
  sensor.setLowHumidity(40);      // Low humidity of 40%

  // Configure Measurements
  sensor.setMeasurementMode(TEMP_AND_HUMID);  // Set measurements to temperature and humidity
  sensor.setRate(ONE_HZ);                     // Set measurement frequency to 1 Hz
  sensor.setTempRes(FOURTEEN_BIT);
  sensor.setHumidRes(FOURTEEN_BIT);

  //begin measuring
  sensor.triggerMeasurement();

  // 偵測是否安裝好OLED了
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // SSD1306 OLED的位址為0x3C
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // 清除畫面
  display.clearDisplay();
  delay(1000);         // 停1秒

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);  //斷線 (初始化的意思)
  WiFi.mode(WIFI_OFF);    //設定關閉WIFI模式
  Serial.println("WiFi disconnected!");
  digitalWrite(pin, LOW);
}

void loop() {
  printLocalTime();

  Serial.print("Temperature (C): "); Serial.print(sensor.readTemp());
  Serial.print("\t\tHumidity (%): "); Serial.println(sensor.readHumidity());
  Serial.println();

  testdrawstyles();    // 測試文字
  delay(1000);         // 停1秒
}

void testdrawstyles(void) {
  display.clearDisplay();
  display.setTextColor(1);        // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)

  display.setTextSize(3);               // 設定文字大小
  display.setCursor(21,4);              // 設定起始座標
  display.print(CurrentTime);           // 要顯示的字串

  display.setTextSize(1);               // 設定文字大小
  display.setCursor(29,32);             // 設定起始座標
  display.print(CurrentDate);           // 要顯示的字串

  display.setTextSize(1);               // 設定文字大小
  display.setCursor(38,44);             // 設定起始座標
  display.print(CurrentWeek);           // 要顯示的字串

  display.setTextSize(1);               // 設定文字大小
  display.setCursor(4,56);              // 設定起始座標
  display.print("T:");                  // 要顯示的字串
  display.setCursor(16,56);             // 設定起始座標
  display.print(sensor.readTemp());     // 要顯示的字串
  display.setCursor(41,56);             // 設定起始座標
  display.print(" C");                  // 要顯示的字串

  display.setCursor(68,56);             // 設定起始座標
  display.print("H:");                  // 要顯示的字串
  display.setCursor(80,56);             // 設定起始座標
  display.print(sensor.readHumidity()); // 要顯示的字串
  display.setCursor(105,56);            // 設定起始座標
  display.print(" %");                  // 要顯示的字串

  display.display();                    // 顯示暫存資料
}

void printLocalTime(){
  struct tm timeinfo;
  // 創建一個名為 timeinfo 的時間結構 (struct tm)，其中包含有關時間的所有詳細信息 (分鐘、秒、小時等)
  // tm 結構包含一個日曆日期和時間，分解為以下組件：
  // tm_sec：一分鐘後的秒數 : 0-61
  // tm_min：小時後的分鐘數 : 0-59
  // tm_hour：從午夜開始的小時數 : 0-23
  // tm_mday：一個月中的第幾天 : 1-31
  // tm_year：自 1900 年以來的年數
  // tm_wday：自星期日起的天數 : 0-6
  // tm_yday：自1月1日起的天數 : 0-365
  // tm_isdst：夏令時標誌 : 如果夏令時有效，則大於零，如果夏令時無效，則為零，如果信息不可用，則小於零。

  if(!getLocalTime(&timeinfo)){  //獲取有關日期和時間的所有詳細信息並將它們保存在 timeinfo 結構中
    Serial.println("Failed to obtain time");  //獲取失敗
    return;
  }

  char h_m[15];  //15 chars should be enough
  char m_d_y[15];  //15 chars should be enough
  char day[15];  //15 chars should be enough
  strftime(h_m, sizeof(h_m), "%H:%M", &timeinfo);
  CurrentTime = h_m;
  strftime(m_d_y, sizeof(m_d_y), "%B %d %Y", &timeinfo);
  CurrentDate = m_d_y;
  strftime(day, sizeof(day), "%A", &timeinfo);
  CurrentWeek = day;
  Serial.println(h_m);
  Serial.println(m_d_y);
  Serial.println(day);
}
