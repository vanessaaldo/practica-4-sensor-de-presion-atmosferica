#include <WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h" // Librería de ThingSpeak
#include "DHT.h"        // Librería del sensor DHT

#define DHTPIN 4        // Pin del DHT22
#define DHTTYPE DHT22   // Tipo de sensor

#define PRESSURE_PIN A0 // Pin analógico del sensor HW-611

DHT dht(DHTPIN, DHTTYPE);

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
WiFiClient client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  dht.begin(); // Inicializa el sensor DHT
}

void loop() {
  // Conexión WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Conectando a WiFi: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
      delay(5000);
      Serial.print(".");
    }
    Serial.println("\nConectado a WiFi.");
  }

  // Leer datos del sensor DHT
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  // Leer presión del sensor HW-611
  int rawValue = analogRead(PRESSURE_PIN);
  float voltage = (rawValue / 4095.0) * 3.3;  // Convertir ADC a voltaje (ESP32 usa 12 bits, 3.3V referencia)
  float pressure_kPa = (voltage - 0.5) * 10;  // Conversión a kPa (Ejemplo, revisa la hoja de datos)

  // Verificar si la lectura es válida
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error al leer el sensor DHT!");
  } else {
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.print(" °C, Humedad: ");
    Serial.print(humedad);
    Serial.println(" %");

    Serial.print("Presión: ");
    Serial.print(pressure_kPa);
    Serial.println(" kPa");

    // Enviar datos a ThingSpeak
    ThingSpeak.setField(1, temperatura);
    ThingSpeak.setField(2, humedad);
    ThingSpeak.setField(3, pressure_kPa); // Agregar presión al Field 3

    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Datos enviados a ThingSpeak correctamente.");
    } else {
      Serial.println("Error al enviar datos. Código HTTP: " + String(x));
    }
  }

  delay(5000); // Esperar 5 segundos para la próxima actualización
}

