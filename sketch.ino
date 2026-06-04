#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

#define TRIG_PIN 5
#define ECHO_PIN 18
#define LED_VERMELHO 26
#define LED_VERDE 27
#define BOTAO_PIN 4

#define LIMIAR_DANGER 50
#define LIMIAR_WARNING 150

LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);

float distancia = 0;
String nivelRisco = "SAFE";
int totalAlertas = 0;
bool alertaManual = false;

struct Alerta {
  String descricao;
  String nivel;
  unsigned long timestamp;
};

Alerta historico[10];
int totalHistorico = 0;

float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duracao = pulseIn(ECHO_PIN, HIGH, 38000);
  if (duracao == 0) return 999;
  return duracao * 0.0343 / 2.0;
}

String calcularRisco(float dist) {
  if (dist < LIMIAR_DANGER) return "DANGER";
  if (dist < LIMIAR_WARNING) return "WARNING";
  return "SAFE";
}

void atualizarLEDs(String nivel) {
  if (nivel == "DANGER" || nivel == "WARNING" || alertaManual) {
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(LED_VERDE, LOW);
  } else {
    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(LED_VERDE, HIGH);
  }
}

void atualizarLCD(float dist, String nivel) {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  if (alertaManual) {
    lcd.print("! ALERTA MANUAL!");
  } else if (dist >= 999) {
    lcd.print("Dist: ---cm");
  } else {
    lcd.print("Dist: ");
    lcd.print((int)dist);
    lcd.print("cm");
  }
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  if (alertaManual) {
    lcd.print("Risco: WARNING");
  } else {
    lcd.print("Risco: ");
    lcd.print(nivel);
  }
}

void registrarAlerta(String desc, String nivel) {
  if (totalHistorico < 10) {
    historico[totalHistorico] = { desc, nivel, millis() };
    totalHistorico++;
    totalAlertas++;
  }
}

void handleStatus() {
  StaticJsonDocument<200> doc;
  doc["distancia_cm"] = distancia >= 999 ? -1 : distancia;
  doc["nivel_risco"] = alertaManual ? "WARNING" : nivelRisco;
  doc["total_alertas"] = totalAlertas;
  doc["alerta_manual"] = alertaManual;
  doc["sistema"] = "OPERACIONAL";
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleAlertas() {
  StaticJsonDocument<1024> doc;
  JsonArray arr = doc.createNestedArray("alertas");
  for (int i = 0; i < totalHistorico; i++) {
    JsonObject obj = arr.createNestedObject();
    obj["descricao"] = historico[i].descricao;
    obj["nivel"] = historico[i].nivel;
    obj["timestamp_ms"] = historico[i].timestamp;
  }
  doc["total"] = totalHistorico;
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleConfig() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"erro\":\"Metodo nao permitido\"}");
    return;
  }
  StaticJsonDocument<200> doc;
  deserializeJson(doc, server.arg("plain"));
  String json = "{\"status\":\"configurado\",\"limiar_danger\":" + String(LIMIAR_DANGER) + ",\"limiar_warning\":" + String(LIMIAR_WARNING) + "}";
  server.send(200, "application/json", json);
}

void handleDashboard() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<title>Space Sense IoT</title>";
  html += "<style>body{background:#0A0E1A;color:#F1F5F9;font-family:sans-serif;padding:20px;}";
  html += "h1{color:#4F8EF7;} .card{background:#111827;border-radius:12px;padding:16px;margin:10px 0;border:1px solid #1E2A3A;}";
  html += ".DANGER{color:#EF4444;} .WARNING{color:#F59E0B;} .SAFE{color:#10B981;}";
  html += "</style></head><body>";
  html += "<h1>🛰 Space Sense IoT</h1>";
  html += "<div class='card'><b>Distância:</b> " + String((int)distancia) + " cm</div>";
  html += "<div class='card'><b>Nível de Risco:</b> <span class='" + nivelRisco + "'>" + nivelRisco + "</span></div>";
  html += "<div class='card'><b>Total de Alertas:</b> " + String(totalAlertas) + "</div>";
  html += "<div class='card'><b>Alerta Manual:</b> " + String(alertaManual ? "SIM ⚠️" : "NAO") + "</div>";
  html += "<p style='color:#475569'>Atualiza automaticamente a cada 2s</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(BOTAO_PIN, INPUT_PULLUP);

  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(LED_VERDE, LOW);

  Wire.begin(21, 22);
  delay(100);
  lcd.init();
  delay(100);
  lcd.backlight();
  delay(100);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Space Sense");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(2000);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando WiFi");
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi OK!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    Serial.println("\nWiFi falhou!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi falhou!");
  }

  server.on("/status", handleStatus);
  server.on("/alerts", handleAlertas);
  server.on("/config", handleConfig);
  server.on("/", handleDashboard);
  server.begin();
}

void loop() {
  server.handleClient();

  distancia = medirDistancia();
  nivelRisco = calcularRisco(distancia);

  alertaManual = digitalRead(BOTAO_PIN) == LOW;

  Serial.print("Dist: ");
  Serial.print((int)distancia);
  Serial.print("cm | Risco: ");
  Serial.print(nivelRisco);
  Serial.print(" | Alerta Manual: ");
  Serial.println(alertaManual ? "SIM" : "NAO");

  atualizarLEDs(nivelRisco);
  atualizarLCD(distancia, nivelRisco);

  if (alertaManual) {
    registrarAlerta("Alerta manual acionado pelo operador", "WARNING");
  }

  if (nivelRisco == "DANGER") {
    registrarAlerta("Detrito a " + String((int)distancia) + "cm — risco critico", "DANGER");
  }

  delay(500);
}