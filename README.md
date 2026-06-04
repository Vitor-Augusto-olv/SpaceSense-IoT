# 🛰 Space Sense — IoT

> Protótipo de monitoramento de detritos espaciais com ESP32 — FIAP Global Solution 2025

---

## 👥 Equipe

| Nome | RM | Turma |
|---|---|---|
| Vitor Augusto Oliveira de Abreu | RM564227 | 2TDSPJ |
| André Bellandi Vital Rodrigues | RM564662 | 2TDSPJ |
| Gabriel Garcia Mayo Delatore | RM563298 | 2TDSPJ |

---

## 🎥 Vídeo de Apresentação

📺 [Assista no YouTube](https://www.youtube.com/link-do-video)

---

## 🔗 Simulação no Wokwi

▶️ [Abrir projeto no Wokwi](https://wokwi.com/projects/465565763637172225)

---

## 💡 Sobre o Projeto

O **Space Sense IoT** é um protótipo funcional de monitoramento de detritos espaciais utilizando ESP32. O sistema simula sensores embarcados em um satélite, detectando a proximidade de detritos e emitindo alertas em tempo real.

O protótipo integra:
- Sensor ultrassônico HC-SR04 simulando radar de proximidade
- Display LCD exibindo distância e nível de risco
- LEDs indicando status de segurança
- Botão de alerta manual de emergência
- Servidor Web com API REST para comunicação via Wi-Fi

---

## 🔌 Componentes

| Componente | Função | Pino ESP32 |
|---|---|---|
| HC-SR04 | Sensor de distância (radar) | TRIG: D5 / ECHO: D18 |
| LCD 1602 I2C | Interface local de status | SDA: D21 / SCL: D22 |
| LED Vermelho | Alerta de risco | D26 |
| LED Verde | Status seguro | D27 |
| Push Button | Alerta manual de emergência | D4 |

---

## 📡 Endpoints REST

Base URL: `http://<IP_ESP32>`

### GET /status
Retorna o status atual do sistema.

**Resposta:**
```json
{
  "distancia_cm": 120,
  "nivel_risco": "WARNING",
  "total_alertas": 3,
  "alerta_manual": false,
  "sistema": "OPERACIONAL"
}
```

---

### GET /alerts
Retorna o histórico de alertas registrados.

**Resposta:**
```json
{
  "alertas": [
    {
      "descricao": "Detrito a 45cm — risco critico",
      "nivel": "DANGER",
      "timestamp_ms": 5000
    }
  ],
  "total": 1
}
```

---

### POST /config
Define o limiar de distância para acionamento de alertas.

**Body:**
```json
{
  "limiar_danger": 50,
  "limiar_warning": 150
}
```

**Resposta:**
```json
{
  "status": "configurado",
  "limiar_danger": 50,
  "limiar_warning": 150
}
```

---

## 🚦 Níveis de Risco

| Nível | Distância | LED | LCD |
|---|---|---|---|
| DANGER | < 50cm | Vermelho | `Risco: DANGER` |
| WARNING | 50cm — 150cm | Vermelho | `Risco: WARNING` |
| SAFE | > 150cm | Verde | `Risco: SAFE` |

---

## 📊 Dashboard

Acesse o dashboard visual em: [Space Sense IoT Dashboard](https://vitor-augusto-olv.github.io/SpaceSense-IoT/dashboard/index.html)

O dashboard exibe em tempo real:
- Distância detectada pelo sensor
- Nível de risco atual
- Total de alertas registrados
- Histórico de alertas
- Status do alerta manual

---

## 🚀 Como Executar

1. Acesse o projeto no Wokwi pelo link acima
2. Clique em **Play** para iniciar a simulação
3. Interaja com o sensor HC-SR04 movendo o slider de distância
4. Pressione o botão para acionar alerta manual
5. Observe o LCD e os LEDs respondendo em tempo real

---

## 🔧 Bibliotecas Utilizadas

- `LiquidCrystal I2C` — controle do display LCD
- `ArduinoJson` — serialização dos endpoints JSON
- `WiFi.h` — comunicação via Wi-Fi
- `WebServer.h` — servidor HTTP no ESP32

---

<p align="center">Desenvolvido com 🚀 para FIAP Global Solution 2025</p>