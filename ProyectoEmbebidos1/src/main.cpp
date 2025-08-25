#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>

// ====== CONFIG WiFi ======
const char* ssid = "NETLIFE-CHAVEZ -2.4 G";
const char* password = "@raqueta2015";
//Lab. Telematica
//l4bt3l3m4tic@

// ====== CONFIG Firebase ======
const char* firebaseHost = "https://proyectoembebidos-16a3d-default-rtdb.firebaseio.com";

// ====== GPS ======
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

// ====== MPU6050 + LCD ======
#define TRIG_PIN 13
#define ECHO_PIN 12
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_MPU6050 mpu;

// Altura normal estimada del vehículo en cm
const int alturaNormal = 15;
const int umbralSubida = 25;
const int umbralBajada = 10;

float prev_ax = 0, prev_ay = 0, prev_az = 0;
float prev_pitch = 0;

// ====== Variables ======
unsigned long lastSendTime = 0;
unsigned long lastWiFiCheck = 0;
const unsigned long sendInterval = 5000; // 5 segundos entre reportes
const unsigned long wifiCheckInterval = 30000; // Verificar WiFi cada 30 segundos
int reporteCounter = 1000;
int reconnectAttempts = 0;
const int maxReconnectAttempts = 3;

// ====== FUNCIONES ======
long medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duracion = pulseIn(ECHO_PIN, HIGH, 30000);
  return duracion * 0.034 / 2; // cm
}


bool checkInternetConnectivity() {
  HTTPClient http;
  http.setTimeout(5000);
  http.begin("http://google.com");
  int httpCode = http.GET();
  http.end();
  
  if (httpCode > 0) {
    Serial.println("✓ Conectividad a internet OK");
    return true;
  } else {
    Serial.printf("✗ Sin conectividad a internet (HTTP %d)\n", httpCode);
    return false;
  }
}

void reconnectWiFi() {
  if (reconnectAttempts >= maxReconnectAttempts) {
    Serial.println("Máximo de intentos de reconexión alcanzado");
    return;
  }
  
  Serial.println("Intentando reconectar WiFi...");
  WiFi.disconnect();
  delay(1000);
  
  // Configurar DNS públicos
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8), IPAddress(8, 8, 4, 4));
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi reconectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    reconnectAttempts = 0; // Reset counter
  } else {
    Serial.println("\n✗ Falló reconexión WiFi");
    reconnectAttempts++;
  }
}

bool sendDataToFirebase(String reporteName, String field, String value) {
  HTTPClient http;
  String firebaseURL = String(firebaseHost) + "/reportes/" + reporteName + "/" + field + ".json";
  
  Serial.printf("Enviando a: %s\n", firebaseURL.c_str());
  
  // Timeout más largo y configuración robusta
  http.setTimeout(15000);
  http.begin(firebaseURL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "ESP32HTTPClient");
  
  int httpResponseCode = http.PUT(value);
  String response = http.getString();
  
  if (httpResponseCode > 0) {
    Serial.printf("✓ %s enviado: %s (HTTP %d)\n", field.c_str(), value.c_str(), httpResponseCode);
    if (response.length() > 0) {
      Serial.println("Respuesta: " + response);
    }
    http.end();
    return true;
  } else {
    Serial.printf("✗ Error enviando %s (HTTP %d)\n", field.c_str(), httpResponseCode);
    String error = http.errorToString(httpResponseCode);
    Serial.println("Error detallado: " + error);
    
    // Si es error de DNS, intentar reconectar
    if (httpResponseCode == -1) {
      Serial.println("Error de conectividad detectado, verificando WiFi...");
      if (WiFi.status() != WL_CONNECTED) {
        reconnectWiFi();
      }
    }
    
    http.end();
    return false;
  }
}

void sendReporteToFirebase(float latitude, float longitude, String ubicacion) {
  // Verificar estado WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado, intentando reconectar...");
    reconnectWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("No se pudo reconectar WiFi, abortando envío");
      return;
    }
  }
  
  // Verificar conectividad a internet
  if (!checkInternetConnectivity()) {
    Serial.println("Sin conectividad a internet, abortando envío");
    return;
  }

  String reporteName = "reporte" + String(reporteCounter);
  Serial.println("\n=== Iniciando envío de reporte ===");
  Serial.printf("Reporte: %s\n", reporteName.c_str());
  
  bool latOK = sendDataToFirebase(reporteName, "lat", String(latitude, 6));
  delay(500); // Mayor delay entre requests
  
  bool lngOK = sendDataToFirebase(reporteName, "lng", String(longitude, 6));
  delay(500);
  
  bool ubicOK = sendDataToFirebase(reporteName, "ubicacion", "\"" + ubicacion + "\"");

  if (latOK && lngOK && ubicOK) {
    Serial.println("\n=== Reporte enviado correctamente ===");
    Serial.printf("Reporte: %s\nLat: %.6f\nLng: %.6f\nUbicacion: %s\n", 
                  reporteName.c_str(), latitude, longitude, ubicacion.c_str());
    reporteCounter++;
    
    // Mostrar confirmación en LCD
    lcd.setCursor(0, 0);
    lcd.print("REPORTE ENVIADO ");
    lcd.setCursor(0, 1);
    lcd.print("#");
    lcd.print(reporteCounter - 1);
    lcd.print("            ");
    delay(2000);
  } else {
    Serial.println("\n✗ Error al enviar reporte completo");
    
    // Mostrar error en LCD
    lcd.setCursor(0, 0);
    lcd.print("ERROR ENVIO     ");
    lcd.setCursor(0, 1);
    lcd.print("Reintentando... ");
    delay(2000);
  }
}

void setup() {
  Wire.begin(21, 22);  // SDA, SCL
  Serial.begin(115200);
  
  // Esperar un momento para que el serial se estabilice
  delay(1000);
  Serial.println("\n=== Iniciando Sistema ===");

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  // MPU6050
  Serial.println("Inicializando MPU6050...");
  if (!mpu.begin()) {
    Serial.println("✗ Error inicializando MPU6050");
    lcd.setCursor(0, 1);
    lcd.print("MPU ERROR");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("✓ MPU6050 inicializado");

  // Pines Ultrasónico
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("✓ Sensor ultrasónico configurado");

  // WiFi con DNS configurado
  Serial.printf("Conectando a WiFi: %s\n", ssid);
  lcd.setCursor(0, 1);
  lcd.print("Conectando WiFi");
  
  // Configurar DNS antes de conectar
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8), IPAddress(8, 8, 4, 4));
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("DNS 1: ");
    Serial.println(WiFi.dnsIP(0));
    Serial.print("DNS 2: ");
    Serial.println(WiFi.dnsIP(1));
    
    lcd.setCursor(0, 1);
    lcd.print("WiFi OK         ");
    
    // Probar conectividad inicial
    checkInternetConnectivity();
  } else {
    Serial.println("\n✗ Error de conexión WiFi");
    lcd.setCursor(0, 1);
    lcd.print("WiFi ERROR      ");
  }

  // GPS
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  Serial.println("✓ GPS inicializado");

  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistema Activo");
  delay(2000);
  lcd.clear();
  
  Serial.println("=== Sistema listo ===\n");
}

void loop() {
  // Verificar WiFi periódicamente
  unsigned long currentTime = millis();
  if (currentTime - lastWiFiCheck >= wifiCheckInterval) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi desconectado, intentando reconectar...");
      reconnectWiFi();
    }
    lastWiFiCheck = currentTime;
  }

  // ==== LECTURA MPU ====
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  float dx = abs(ax - prev_ax);
  float dy = abs(ay - prev_ay);
  float dz = abs(az - prev_az);

  prev_ax = ax;
  prev_ay = ay;
  prev_az = az;

  float pitch = atan2(ax, sqrt(ay * ay + az * az)) * 180 / PI;
  float delta_pitch = abs(pitch - prev_pitch);
  prev_pitch = pitch;

   Serial.println(abs(dx));
   Serial.print("X");
    Serial.println(abs(dy));
    Serial.print("y");
     Serial.println(abs(dz));
     Serial.print("z");

  
  long distancia = medirDistancia();
// ==== UMBRALES OPTIMIZADOS PARA VEHÍCULO (VERSIÓN SIMPLIFICADA) ====
// ==== UMBRALES OPTIMIZADOS PARA VEHÍCULO (VERSIÓN SIMPLIFICADA) ====

// ==== UMBRALES OPTIMIZADOS PARA VEHÍCULO (BASADO EN DATOS REALES) ====

// UMBRALES PARA DETECCIÓN DE GOLPE (VALORES ALTOS SOSTENIDOS)
float umbralAlturaAlta = 15.0;    // Altura máxima para considerar golpe
float umbralBajada = -8.0;        // Bajada brusca que indica impacto
float umbralBruscoX = 6.0;        // Movimiento brusco eje X (observado: 6.71-9.18)
float umbralBruscoY = 9.0;        // Movimiento brusco eje Y (observado: 9.39-10.57)
float umbralBruscoZ = 3.2;        // Movimiento brusco eje Z (impacto vertical)

// UMBRALES PARA ESTADO QUIETO (AJUSTADOS CON DATOS REALES)
float umbralQuietoX = 0.12;       // Tolerancia máxima en X para quieto (máximo observado: 0.11)
float umbralQuietoY = 0.10;       // Tolerancia máxima en Y para quieto (máximo observado: 0.09)

// UMBRALES PARA MOVIMIENTO NORMAL (AJUSTADOS CON DATOS REALES)
float umbralMovimientoMinX = 0.50; // Cambio mínimo en X para detectar movimiento
float umbralMovimientoMaxY = 1.00; // Y máximo para movimiento suave (vs irregularidad)

// UMBRALES PARA IRREGULARIDADES (SACUDIDAS LATERALES INTERMITENTES)
float umbralIrregularidadY = 3.0; // Y que indica sacudida lateral (observado: 3.96-11.60)
float umbralIrregularidadYAlto = 8.0; // Y muy alto pero intermitente (vs golpe sostenido)
float umbralIrregularidadZ = 0.5;  // Cambios en Z por baches/desniveles

// ==== LÓGICA DE CLASIFICACIÓN OPTIMIZADA ====

bool golpePorAltura = (distancia > umbralAlturaAlta || distancia < umbralBajada);
bool golpePorMovimiento = (abs(dz) > umbralBruscoZ || (abs(dx) > umbralBruscoX && abs(dy) > umbralBruscoY));

String estado;

if (golpePorAltura && golpePorMovimiento) {
    estado = "POSIBLE GOLPE";
}
else if ((abs(dx) > umbralBruscoX && abs(dy) > umbralBruscoY)) {
    estado = "POSIBLE GOLPE"; // Golpe detectado solo por movimiento (sin altura)
}
else if (abs(dx) <= umbralQuietoX && abs(dy) <= umbralQuietoY) {
    estado = "Quieto";
}
else if (abs(dy) >= umbralIrregularidadY && abs(dx) < umbralBruscoX) {
    estado = "Irregularidad"; // Sacudida lateral (Y alto pero X moderado)
}
else if (abs(dx) >= umbralMovimientoMinX && abs(dy) <= umbralMovimientoMaxY) {
    estado = "Movimiento";
}
else {
    estado = "Movimiento"; // Por defecto, si hay cambios que no son quieto
}

  // Mostrar estado y conectividad
  lcd.setCursor(0, 0);
  lcd.print(estado);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("Alt:");
  lcd.print(distancia);
  lcd.print("cm ");
  
  // Indicador de WiFi
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("W");
  } else {
    lcd.print("X");
  }
  
  // Indicador de GPS
  if (gps.location.isValid()) {
    lcd.print("G");
  } else {
    lcd.print("-");
  }
  lcd.print("   ");

  // ==== LECTURA GPS ====
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isUpdated()) {
        Serial.printf("GPS: Lat=%.6f, Lng=%.6f, Sats=%d\n", 
                     gps.location.lat(), gps.location.lng(), gps.satellites.value());
      }
    }
  }

  // Solo enviar si hay golpe o irregularidad Y tenemos GPS válido
  if ((estado == "POSIBLE GOLPE" || estado == "Irregularidad") && 
      gps.location.isValid() && WiFi.status() == WL_CONNECTED) {
    
    if (currentTime - lastSendTime >= sendInterval) {
      float latitude = gps.location.lat();
      float longitude = gps.location.lng();
      String ubicacion = "Ceibos"; // Ubicación aproximada
      
      Serial.printf("\n>>> Detectado: %s <<<\n", estado.c_str());
      sendReporteToFirebase(latitude, longitude, ubicacion);
      lastSendTime = currentTime;
    }
  }

  delay(300);
}