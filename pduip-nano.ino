#include <UIPEthernet.h>
#include <PubSubClient.h>
#define CLIENT_ID "LB-PDU4-000139"  //REEMPLAZAR
#define TOPIC "/LB-PDU4-000139"    //REEMPLAZAR
#define PUBLISH_DELAY 1000

uint8_t mac[6] = { 0x00, 0x01, 0x02, 0x01, 0x03, 0x09};//REEMPLAZAR por Ultimos numeros del TOPIC 000145  0x01, 0x04, 0x05}

IPAddress mqttServer(192, 168, 1, 10); //REEMPLAZAR
IPAddress ip(192, 168, 1, 1);//REEMPLAZAR
IPAddress dnsServer(192, 168, 1, 1);//REEMPLAZAR
IPAddress gw(192, 168, 1, 1);//REEMPLAZAR
IPAddress mask(255, 255, 255, 0);//REEMPLAZAR

EthernetClient ethClient;
PubSubClient mqttClient;

long previousMillis;

void OnOff(int gpio, int command)
{
  digitalWrite(gpio, command);
  delay(500);

}

// Pines a los que están conectados los relés de la PDU
int pinRelay[] = { 4, 5, 6, 7 };

void initRelay()
{
  for (int i = 0; i < 4; i++)
  {
    pinMode(pinRelay[i], OUTPUT);
  }
}

long lastReconnectAttempt = 0;

boolean reconnect()
{
  if (mqttClient.connect(CLIENT_ID))
  {
    mqttClient.subscribe(TOPIC);
  }

  return mqttClient.connected();
}

void callback(char *topic, byte *payload, unsigned int length)
{
  char *section;
  char *state;
  char *value;

  
  section = strtok(payload, "::");
  state = strtok(NULL, "::");
  value = strtok(NULL, "::");
  if (strcmp(section, "RELAY") == 0)
  {
    int gpio = atoi(value);
    int command = atoi(state);
    OnOff(gpio, command);
  }
}

void setup()
{
  // setup serial communication
  Serial.begin(115200);
  initRelay();
  Ethernet.begin(mac, ip, dnsServer, gw, mask);
  Serial.println(Ethernet.localIP());
  Serial.println();

  mqttClient.setClient(ethClient);
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(callback);
  previousMillis = millis();
  lastReconnectAttempt = 0;
}

void loop()
{
  if (!mqttClient.connected())
  {
    long now = millis();
    if (now - lastReconnectAttempt > 5000)
    {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect())
      {
        lastReconnectAttempt = 0;
      }
    }
  }
  else
  {
    // Client connected
    if (millis() - previousMillis > PUBLISH_DELAY)
    {
      sendData();
      previousMillis = millis();
    }

    mqttClient.loop();
  }
}

void sendData()
{
  char buffer[126];

  snprintf(buffer, sizeof(buffer),
    "{\"nserie\":\"%s\",\"version\":\"2\",\"tomas\":{\"1\":\"%s\",\"2\":\"%s\",\"3\":\"%s\",\"4\":\"%s\"}}",
    CLIENT_ID,
    digitalRead(4) == 0 ? "ON" : "OFF",
    digitalRead(5) == 0 ? "ON" : "OFF",
    digitalRead(6) == 0 ? "ON" : "OFF",
    digitalRead(7) == 0 ? "ON" : "OFF");

  

  if (mqttClient.connect(CLIENT_ID))
  {
    mqttClient.publish("/servidor", buffer);
  }
}
