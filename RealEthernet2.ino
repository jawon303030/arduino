#include <SPI.h>
#include <Ethernet.h>
#include <Arduino_JSON.h>

// Ethernet 설정
#define CS_PIN 53
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// 고정 IP 설정
IPAddress staticIP(192, 168, 0, 178);   // 사용할 고정 IP
IPAddress gateway(192, 168, 0, 1);      // 게이트웨이
IPAddress subnet(255, 255, 255, 0);     // 서브넷 마스크

// 서버 정보
char server[] = "192.168.0.7";  // 서버 IP 주소
int server_port = 5501;         // 서버 포트 번호

// 변수 선언
EthernetClient client;
int player_1_position = 1;
int player_2_position = 1;

bool isCom3 = true;
int player1Ports[] = { 3, 5, 7, 9, 11, 13, 15, 17, 19, 23, 25, 27, 29, 31, 33, 35};
int player2Ports[] = { 2, 4, 6, 8, 10, 12, 14, 16, 18, 22, 24, 26, 28, 30, 32, 34};


char positionDataJson[20] = { 0, };


void GetPlayerPosition() //client function to send/receive GET request data.
{
  if (client.connect(server, server_port)) {  //starts client connection, checks for connection
    Serial.println("connected.");
    client.println("GET /player-positions HTTP/1.0"); //download text
    client.println(); //end of get request
  } 
  else {
    Serial.println("connection failed"); //error message if no client connect
    Serial.println();
  }

  while(client.connected() && !client.available()) delay(1); //waits for data
  bool seen_opening_bracelet = false;
  bool seen_closing_bracelet = false;
  char* ptr = positionDataJson;
  while (client.connected() || client.available()) { //connected or data available
    char c = client.read(); //gets byte from ethernet buffer
    if (c == '{') {
      seen_opening_bracelet = true;
    }
    if (seen_opening_bracelet && !seen_closing_bracelet) {
      *ptr++ = c;
    }
    if (c == '}') {
      seen_closing_bracelet = true;
    }
  }
  *ptr++ = '\0';

  JSONVar doc = JSON.parse(positionDataJson);

  player_1_position = (int)doc["0"];
  player_2_position = (int)doc["1"];

  Serial.println(player_1_position);
  Serial.println(player_2_position);
  client.stop(); //stop client

}

void setup() {
  Serial.begin(9600);
  Serial.println("aa");

  // CS 핀 초기화
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH); 
  Ethernet.init(CS_PIN);  // CS 핀 설정

  // Ethernet을 고정 IP로 설정
  Serial.println("Initializing Ethernet with static IP...");
  Ethernet.begin(mac, staticIP, gateway, gateway, subnet);  // DHCP 대신 고정 IP 설정

  Serial.print("Static IP assigned: ");
  Serial.println(Ethernet.localIP());
  delay(1000);
}

void loop() {
  GetPlayerPosition();

  for (int i = 0; i < 16; i++) {
    digitalWrite(player1Ports[i], LOW);
    digitalWrite(player2Ports[i], LOW);
  }
  if (isCom3) {
    if (player_1_position >= 17) {
      // 17~32
      digitalWrite(player1Ports[player_1_position - 17], HIGH);
    }
    if (player_2_position >= 17) {
      // 17~32
      digitalWrite(player2Ports[player_2_position - 17], HIGH);
    }
  } else {
    if (player_1_position < 17) {
      // 1~16
      digitalWrite(player1Ports[player_1_position - 1], HIGH);
    }
    if (player_2_position < 17) {
      // 1~16
      digitalWrite(player2Ports[player_2_position - 1], HIGH);
    }
  }

  delay(1000);
}

