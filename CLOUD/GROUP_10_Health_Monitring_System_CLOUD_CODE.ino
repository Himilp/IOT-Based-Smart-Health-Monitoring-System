#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>
#include <DHT.h>

#define REPORTING_PERIOD_MS     1000
#define WIFISSID "Him123" // Put your WifiSSID here
#define PASSWORD "123456789" // Put your wifi password here
#define TOKEN "BBUS-JCYhuY6O2KWHDrZwEV9W59lyhrC8JR" // Put your Ubidots' TOKEN
#define DEVICE_LABEL "health-monitoring-system" // Put the device label
#define VARIABLE_LABEL_1 "temperature" // Put the variable label for DHT11
#define VARIABLE_LABEL_2 "humidity" // Put the variable label for DHT11
#define VARIABLE_LABEL_3 "pulseRate"
#define MQTT_CLIENT_NAME "EI_OXMO" // MQTT client Name, put a Random ASCII

DHT dht(D4, DHT11);  // Assuming DHT11 is connected to pin D4
#define PULSE_SENSOR_PIN A0
uint32_t tsLastReport = 0;
char mqttBroker[] = "industrial.api.ubidots.com";
char payload[700];
char topic[150];
char str_val_1[6];
char str_val_2[6];
char str_val_3[6];
int flag = 0;
ESP8266WiFiMulti WiFiMulti;
WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length) {
  // MQTT callback function
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Reconnect to MQTT broker
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  WiFiMulti.addAP(WIFISSID, PASSWORD);
  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);

  // Initialize DHT sensor
  dht.begin();
}

void loop(){
  if (flag == 0)
  {
    client.connect(MQTT_CLIENT_NAME, TOKEN, "");
    Serial.println("MQTT connected again");
    flag = 1;
  }
  if (!client.connected()) {
    Serial.print("Reconnecting ... ");
    reconnect();
  }

  // Read DHT11 sensor values
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Print sensor values to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C    Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  int pulseValue = analogRead(PULSE_SENSOR_PIN);


   float pulseRate = map(pulseValue, 500, 700, 60, 120);

  Serial.print("Calculated Pulse Rate: ");
  Serial.println(pulseRate);
  // Convert sensor values to strings
  dtostrf(temperature, 4, 2, str_val_1);
  dtostrf(humidity, 4, 2, str_val_2);
dtostrf(pulseRate, 4, 2, str_val_3);


  // Publish sensor values to Ubidots
  sprintf(topic, "%s", "");
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", "");
sprintf(payload, "{\"%s\": {\"value\": %s},", VARIABLE_LABEL_1, str_val_1);
sprintf(payload, "%s \"%s\": {\"value\": %s},", payload, VARIABLE_LABEL_2, str_val_2);
sprintf(payload, "%s \"%s\": {\"value\": %s}}", payload, VARIABLE_LABEL_3, str_val_3);




  Serial.println(payload);
  Serial.println(topic);

  client.publish(topic, payload);
  client.loop();

  // Add a delay before the next iteration
  delay(REPORTING_PERIOD_MS);
}
