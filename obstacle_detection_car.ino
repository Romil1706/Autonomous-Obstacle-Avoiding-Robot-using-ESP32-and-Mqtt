#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ===== WiFi =====
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ===== MQTT =====
const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);

bool robotEnabled = true;

// Motor Pins
#define IN1 14
#define IN2 27
#define IN3 26
#define IN4 25

#define TRIG 33
#define ECHO 32

Servo myServo;

// ===== MQTT callback =====
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  if (msg == "OFF") robotEnabled = false;
  if (msg == "ON") robotEnabled = true;

  Serial.println("MQTT Control: " + msg);
}

// ===== WiFi =====
void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

// ===== MQTT reconnect =====
void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Robot")) {
      client.subscribe("robot/control");
    } else {
      delay(2000);
    }
  }
}

// ===== Distance =====
int getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 20000);
  int d = duration * 0.034 / 2;

  if (d < 3 || d > 200) return 100;
  return d;
}

// ===== Forward =====
void forwardSlow() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(100);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  delay(60);
}

// ===== Stop =====
void stopCar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// ===== Turns =====
void left() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void right() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// ===== Reverse =====
void reverseShort() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(250);
  stopCar();
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  robotEnabled = false;
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  myServo.attach(13);
  myServo.write(90);
}

// ===== Loop =====
void loop() {

  if (!client.connected()) reconnect();
  client.loop();

  // MQTT OFF control
  if (!robotEnabled) {
    stopCar();
    return;
  }

  forwardSlow();

  int front = getDistance();

  Serial.print("Front: ");
  Serial.println(front);

  if (front < 30) {

    stopCar();
    delay(150);

    // Reverse a bit
    reverseShort();
    delay(150);

    // LEFT
    myServo.write(180);
    delay(2000);
    int leftDist = getDistance();

    Serial.print("Left Distance: ");
    Serial.println(leftDist);

    // RIGHT
    myServo.write(0);
    delay(2000);
    int rightDist = getDistance();

    Serial.print("Right Distance: ");
    Serial.println(rightDist);

    // CENTER
    myServo.write(90);
    delay(200);

    String decision;

    if (leftDist > 30 && leftDist > rightDist + 5) {

      decision = "LEFT";
      Serial.println("Decision: LEFT");
      left();
      delay(450);

    } else if (rightDist > 30 && rightDist > leftDist + 5) {

      decision = "RIGHT";
      Serial.println("Decision: RIGHT");
      right();
      delay(450);

    } else {

      decision = "ESCAPE";
      Serial.println("Decision: ESCAPE TURN");
      right();   // consistent escape
      delay(500);
    }

    client.publish("robot/front", String(front).c_str());
    client.publish("robot/left", String(leftDist).c_str());
    client.publish("robot/right", String(rightDist).c_str());
    client.publish("robot/decision", decision.c_str());

    stopCar();
    delay(200);

    // Check again after turn
    int frontCheck = getDistance();

    Serial.print("Front After Turn: ");
    Serial.println(frontCheck);

    if (frontCheck < 30) {

      Serial.println("Still blocked -> turning same direction");

      if (leftDist > rightDist) {
        left();
      } else {
        right();
      }

      delay(250);
      stopCar();
      delay(200);
    }
  }

  delay(50);
}
