//https://www.ekit-tech.com/?p=3434

#include <WiFi.h>
#include <WiFiClientSecure.h>

// WiFi Setting
const char* ssid     = "AP01-01";
const char* password = "1qaz2wsx";

// LINE Notify Setting
const char* host = "notify-api.line.me";
const char* token = "CfkpRbDPnZAd9Z5NBgVgjePFtHCd0ytd2vo6F0pH5jD"; //Your API key
const char* message = "玄関の鍵がしまりました！";

#define echoPin 33
#define trigPin 32
#define bzPIN 25
#define BEAT 500
#define SO 391.995

int swPin = 32;
bool state = 0;

double Duration = 0; //受信した間隔
double Distance = 0; //距離

bool switchTriggered = false;


void setup(){
  Serial.begin(115200);
  //pinMode(swPin,INPUT);
  pinMode(echoPin,INPUT);
  pinMode(trigPin,OUTPUT);
  connectWiFi();
  ledcSetup(0, 12000, 8); //ここは変えなくていい
  ledcAttachPin(bzPIN, 0);
}

void loop(){
  if (!switchTriggered) {
  delay(500);
  kyori();
    if (Distance >= 3.5) {
      delay(50);
      send_line(); 
      switchTriggered = true;
    }
   }
  }
  
void kyori(){
  digitalWrite(trigPin, LOW);//出てなくても一尾消すため
  delayMicroseconds(2);//2マイクロセック
  digitalWrite(trigPin, HIGH); //超音波を出力
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  Duration = pulseIn(echoPin, HIGH); //センサからの入力
  Duration = Duration/2; //往復距離を半分　　なので２で割っている
  Distance = Duration*340*100/1000000; //音速を340m/sに設定 　センチにしたいから
  Serial.print("Distance:");
  Serial.print(Distance);
  Serial.println("cm");//ln 改行入る
  delay(500);

}


void connectWiFi(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //Start WiFi connection
  WiFi.begin(ssid, password);

  //Check WiFi connection status
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void playmusic(){
  ledcWriteTone(0, SO);
   delay(BEAT);
  ledcWriteTone(0, 0); // no sound
   delay(BEAT);
}

void send_line() {
  //Access to HTTPS (SSL communication)
  WiFiClientSecure client;
  //Required when connecting without verifying the server certificate
  client.setInsecure();

  Serial.println("Try");

  //SSL connection to Line's API server (port 443: https)
  if (!client.connect(host, 443)) {
    Serial.println("Connection failed");
    return;
  }
  Serial.println("Connected");
  playmusic();

  // Send request
  String query = String("message=") + String(message);
  String request = String("") +
    "POST /api/notify HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Authorization: Bearer " + token + "\r\n" +
    "Content-Length: " + String(query.length()) +  "\r\n" + 
    "Content-Type: application/x-www-form-urlencoded\r\n\r\n" +
    query + "\r\n";
  client.print(request);
 
  // Wait until reception is complete
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  
  String line = client.readStringUntil('\n');
  Serial.println(line);
}
