//https://www.haruirosoleil.com/entry/2020/02/02/101937
//→【arduino_IDE】googlesheetに書き込んでみた

#include <WiFi.h>
#include <WiFiClientSecure.h>


// 接続先のSSIDとパスワード
const char* ssid = "AP01-01"; //無線ルーターのssidを入力
const char* password = "1qaz2wsx"; //無線ルーターのパスワードを入力

// LINE Notify Setting
const char* host = "notify-api.line.me";
const char* token = "CfkpRbDPnZAd9Z5NBgVgjePFtHCd0ytd2vo6F0pH5jD"; //Your API key
const char* message = "戸締りOKです！";

#define swPin 32  
#define bzPIN 25
#define BEAT 500
#define SO 391.995
#define on HIGH
#define off LOW

int swState = 0;
bool state = 0;

void setup() {
  Serial.begin(115200);
  pinMode(swPin, INPUT); 
  ledcSetup(0, 12000, 8); 
  ledcAttachPin(bzPIN, 0);
}


void loop() {
  sendData();
  delay(500);
  swState = digitalRead(swPin);
}

void playmusic(){
  ledcWriteTone(0, SO);
   delay(BEAT);
  ledcWriteTone(0, 0); // no sound
   delay(BEAT);
}


//WiFiに接続
void connectWiFi(){

  Serial.print("ssid:");
  Serial.print(ssid);
  Serial.println(" に接続します。");

  WiFi.begin(ssid, password);

  Serial.print("WiFiに接続中");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("接続しました。");

  //IPアドレスの表示
  Serial.print("IPアドレス:");
  Serial.println(WiFi.localIP());

}

//測定データの送信
void sendData(){

  WiFiClientSecure sslclient;

  const char* server = "script.google.com";
  String url = "https://script.google.com/macros/s/AKfycbwsnezzU9UU_B8QXUAQhKbqqVahQhywbZ67REto6_moyHDw7uubK0VKoOAg58BHBHAs7g/exec";  //googlescript web appのurlを入力

  
 float sensor_data1 = digitalRead(swPin);
  if (swState == on) {
    playmusic();
    Serial.println("押されました");
    delay(200);
    connectWiFi();
  }

  // float sensor_data2 = time();
  

  // Serial.println(sensor_data1);
  // Serial.println(sensor_data2);

  //urlの末尾に測定値を加筆
  url += "?";
  url += "&1_cell=";
  url += sensor_data1;


  // サーバーにアクセス
  Serial.println("サーバーに接続中...");
  sslclient.setInsecure();//skip verification
  
  //データの送信
  if (!sslclient.connect(server, 443)) {
    Serial.println("接続していません");
    Serial.println("");//改行
    return;
  }

  Serial.println("サーバーに接続しました");

  sslclient.println("GET " + url);
  delay(1000); //私の環境ではここに待ち時間を入れないとデータが送れないことがある
  sslclient.stop();

  Serial.println("データ送信完了");
  playmusic();
  Serial.println("");//改行
  

  //SSL connection to Line's API server (port 443: https)
  if (!sslclient.connect(host, 443)) {
    Serial.println("押されていません");
    return;
  }
  // Serial.println("Connected");


  String query = String("message=") + String(message);
  String request = String("") +
    "POST /api/notify HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Authorization: Bearer " + token + "\r\n" +
    "Content-Length: " + String(query.length()) +  "\r\n" + 
    "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
    query + "\r\n";
  sslclient.print(request);
 
  // Wait until reception is complete
  while (sslclient.connected()) {
    String line = sslclient.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  
  String line = sslclient.readStringUntil('\n');
  Serial.println(line);

  //WiFiを切断
  WiFi.mode(WIFI_OFF);

}
