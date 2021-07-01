#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#define MSG_BUFFER_SIZE  (50) 

const int typeDHT = DHT22; //khai báo loại sensor độ ẩm không khí
const char* ssid = "HA GIANG"; //khai báo id và password của wifi
const char* password = "hoilamgi";
const char* mqtt_server = "broker.hivemq.com"; //khai báo tên broker mqtt
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;
const char* serverName1 = "http://192.168.1.3:5000";

unsigned long lastTime = 0;
unsigned long lastMsg = 0;
unsigned long timerDelay = 60000;
char msg[MSG_BUFFER_SIZE];
int value = 0;
int pump = 0;
int led = 0;
String deviceid;
int digSoilPin = 4; // GPIO4 ~ D2 //khai báo chân thiết bị
int anaSoilPin = A0;
int DHT22Pin = 5; // GPIO5 ~ D1
int relayPumpPin = 2; // GPIO2 ~ D4
int redLedPin =  12  ;    // GPIO ~ D6
int greenLedPin = 13  ;      // GPIO ~ D7
WiFiClient espClient;


PubSubClient client(espClient);
DHT dht(DHT22Pin, typeDHT);

// Hàm setup các thông tin wifi và khởi động kết nối wifi
void setup_wifi() {
 Serial.printf(" ESP8266 Chip id = %08X\n", ESP.getChipId());
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.printf(" ESP8266 Chip id = %08X\n", ESP.getChipId());
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Hàm xác định các topic và nội dung topic mqtt
void callback(char* topic, byte* payload, unsigned int length) {
  String command = "";
  String stt ;
  String  a ;
  Serial.print(" Message arrived ["); // \a phat ra tieng keu
  Serial.print(topic);
  Serial.print("]:  ");
  for (int i = 0; i < length; i++) {
    command += (char)payload[i];
  }
  Serial.println(command);
  if ((char)payload[0] == '1'){
    digitalWrite(relayPumpPin, HIGH);
    Serial.println("=== Turn on Pump Machine == ");
    pump = 1;
  }
  if ((char)payload[0] == '0'){
     digitalWrite(relayPumpPin, LOW);
      Serial.println("=== Turn off Pump Machine == ");
      pump = 0;
  }
  if ((char)payload[1] == '1'){
    digitalWrite(redLedPin, HIGH);
    led = 1;
    Serial.println("=== Turn on LED == ");
  }
  if ((char)payload[1] == '0'){
    digitalWrite(redLedPin, LOW);
    led = 0;
    Serial.println("=== Turn off LED == ");
  }
}

//Hàm reconnect wifi khi có sự cố về đường truyền
void reconnect() {
  // Loop until we're reconnected
 
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266HaiClient-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("IOTTuanlinh"); //Khai báo topic mà esp subcribe
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
 
  }
}

//Hàm khởi động các thiết bị vào/ra
void setup() {
  deviceid = ESP.getChipId();
  Serial.printf(" ESP8266 Chip id = %08X\n", deviceid);
  pinMode(digSoilPin, INPUT);
  pinMode(anaSoilPin, INPUT);
  pinMode(relayPumpPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  dht.begin();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

//Hàm gửi nhận tín hiệu từ sensor
void loop() {
 
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  unsigned long now = millis();
  if (now - lastMsg > 6000) {
    lastMsg = now;
    deviceid = ESP.getChipId();
    Serial.printf(" ESP8266 Chip id = %08X\n", deviceid);
    int value = analogRead(anaSoilPin);
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int sttPump = 0;
    int sttSpray = 0;
    int valSoil = 100 - map ( value, 0, 1023, 0, 100);
    
    Serial.print("Analog Do am dat: ");
  //  Serial.print(value);
    Serial.print(valSoil);
  
  
//    Serial.print("Digital Do am dat: "); 
//    Serial.println(digitalRead(digSoilPin));  // 0 la co nuoc ( đèn hiệu trên Sensor sáng )------ 1 la khong co nuoc
  
    Serial.print("Nhiet do: ");
    Serial.println(t);
    Serial.print("Do am khong khi: ");
    Serial.println(h);
    if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(espClient,serverName1);

      // Specify content-type header
      http.addHeader("Data Type", "application/json");
      http.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJfaWQiOiJhMTI3ZDBkNy01ZGYxLTQwN2ItYWJhZC1iNmQ4YzA0ZWU1YjEiLCJpYXQiOjE2MjUwNzE3MDJ9.7igs8rJukeZfpaKhmaDK495cvI5T0Ab9u1bORzxgnVc");
      // Data to send with HTTP POST
      String doamkhongkhi = String(h);
      String nhietdokhongkhi = String(t);
      String doamdat = String (valSoil);
      String httpRequestData1 = "doamkk="+doamkhongkhi+"";
      String httpRequestData2 = "nhietdokk="+nhietdokhongkhi+"";
      String httpRequestData3 = "doamdat="+doamdat+"";      
      // Send HTTP POST request
      Serial.println(httpRequestData1);
      Serial.println(httpRequestData2);
      Serial.println(httpRequestData3);
      //int httpResponseCode1 = http.POST(httpRequestData1);
      int httpResponseCode2 = http.POST(httpRequestData2);
      //int httpResponseCode3 = http.POST(httpRequestData3);
      Serial.print("HTTP Response code: ");
      //Serial.println(httpResponseCode1);
      Serial.println(httpResponseCode2);
      //Serial.println(httpResponseCode3);
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
    Serial.println("------------------------");

    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "soil=%d,temp=%0.2f,hum=%0.2f", valSoil, t, h);
    Serial.print("Publish message: ");
    Serial.println(msg);
    Serial.println("================================================================");
  }
}
