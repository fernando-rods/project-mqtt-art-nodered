# Demo JaCaMo + Node-RED + MQTT

Este projeto apresenta uma integração entre **JaCaMo**, **Node-RED**, **Mosquitto MQTT** e **ESP32**, utilizando um artefato CArtAgO específico para intermediar a comunicação entre o agente e um dispositivo IoT via MQTT.

A implementação permite quatro tipos de interação:
* **Communication** — recebimento de mensagens destinadas ao agente;
* **Sensing** — recebimento de leituras do sensor do dispositivo;
* **Monitoring** — recebimento de informações sobre o estado do atuador;
* **Acting** — encaminhamento de ações do agente para o dispositivo.

---

## 1. Arquitetura

A arquitetura atual é composta pelos seguintes elementos:

```text
                         ┌──────────────────┐
                         │    Agent Bob     │
                         │     (Jason)      │
                         └────────┬─────────┘
                                  │
                              CArtAgO
                                  │
                         ┌────────▼─────────┐
                         │     MQTTArt      │
                         │    Artifact      │
                         └────────┬─────────┘
                                  │
                              HTTP/REST
                                  │
                         ┌────────▼─────────┐
                         │     Node-RED     │
                         └────────┬─────────┘
                                  │
                               MQTT
                                  │
                         ┌────────▼─────────┐
                         │     Mosquitto    │
                         │   MQTT Broker    │
                         └────────┬─────────┘
                                  │
                              MQTT/Wi-Fi
                                  │
                         ┌────────▼─────────┐
                         │      ESP32       │
                         │ Sensor + LED     │
                         └──────────────────┘
```

O **Node-RED** funciona como camada de integração entre o artefato CArtAgO e o broker MQTT.

O fluxo de comunicação é bidirecional:

```text
ESP32 → Mosquitto → Node-RED → Artifact → Agent
```

e:

```text
Agent → Artifact → Node-RED → Mosquitto → ESP32
```

---

## 2. Componentes

O projeto utiliza:

* **JaCaMo 1.1** — plataforma para execução do sistema multiagente;
* **Jason** — linguagem utilizada para implementação do agente Bob;
* **CArtAgO** — framework utilizado para criação do artefato `MQTTArt`;
* **Node-RED 1.0.4** — integração entre HTTP/REST e MQTT;
* **Eclipse Mosquitto 2** — broker MQTT;
* **ESP32** — dispositivo utilizado no experimento;
* **Python + Paho MQTT** — cliente auxiliar para publicação e assinatura MQTT;
* **Docker Compose** — orquestração dos serviços JaCaMo, Node-RED e Mosquitto.

---

## 3. Estrutura do projeto

```text
project-mqtt-art-nodered/
│
├── app_jacamo/
│   ├── main.jcm
│   ├── bob.asl
│   ├── logging.properties
│   └── src/
│       └── env/
│           └── net/
│               └── MQTTArt.java
│
├── node-red/
│   ├── flows/
│   │   └── flows.json
│   └── ...
│
├── mosquitto/
│   └── config/
│       ├── mosquitto.conf
│       └── passwd
│
├── sketchESPtoMQTTjson/
│   └── sketchESPtoMQTTjson.ino
│
├── AppPubSubMQTT.py
├── architecture
├── docker-compose.yml
└── readme.md
```

---

## 4. Aplicação JaCaMo

A aplicação JaCaMo está definida em:

```text
app_jacamo/main.jcm
```

O arquivo cria o agente Bob e o workspace `wmqtt`:

```text
mas main {

    agent bob {
        focus: ns1::wmqtt.mqtt
    }

    workspace wmqtt {
        artifact mqtt: net.MQTTArt("http://nodered:1880/act")
    }

    platform: jacamo.rest.JCMRest("--rest-port 8080")
              jacamo.platform.EnvironmentWebInspector("false")
}
```

O agente Bob utiliza o artefato:

```text
mqtt: net.MQTTArt(...)
```

implementado pela classe:

```text
net.MQTTArt
```

O endpoint:

```text
http://nodered:1880/act
```

é utilizado pelo artefato para encaminhar as ações produzidas pelo agente ao Node-RED.

---

## 5. Agente Bob

O agente está implementado em:

```text
app_jacamo/bob.asl
```

O agente inicia com:

```text
!start.
```

e apresenta:

```text
Bob is running
```

O agente possui planos relacionados às interações de comunicação, sensing, monitoring e acting.

### 5.1 Communication

O agente percebe mensagens por meio da propriedade:

```text
message(Source, Message)
```

Quando recebe:

```text
turn_on
```

produz:

```text
action(esp_01,setLed,on)
```

Quando recebe:

```text
turn_off
```

produz:

```text
action(esp_01,setLed,off)
```

---

### 5.2 Sensing

As leituras recebidas pelo artefato são percebidas pelo agente como:

```text
read(Device, Variable, Value)
```

O agente transforma essas informações em crenças:

```text
sensor(Device, Variable, Value)
```

Por exemplo:

```text
read(esp_01,lightSensor,72)
```

é registrado como:

```text
sensor(esp_01,lightSensor,72)
```

---

### 5.3 Monitoring

As informações de estado do atuador são percebidas como:

```text
status(Device, Actuator, Mode)
```

e registradas como:

```text
actuator(Device, Actuator, Mode)
```

Por exemplo:

```text
status(esp_01,led,on)
```

resulta em:

```text
actuator(esp_01,led,on)
```

---

### 5.4 Acting

As ações são representadas por termos Jason:

```text
action(Device, Actuator, Command)
```

Atualmente são utilizados:

```text
action(esp_01,setLed,on)
```

e:

```text
action(esp_01,setLed,off)
```

Esses termos são encaminhados ao artefato por meio da operação:

```text
acting(...)
```

---

## 6. Artefato CArtAgO MQTTArt

O artefato está implementado em:

```text
app_jacamo/src/env/net/MQTTArt.java
```

O `MQTTArt` possui quatro operações:

```text
communication()
sensing()
monitoring()
acting()
```

e três propriedades observáveis:

```text
message
read
status
```

---

### 6.1 Communication

A operação:

```java
communication(String json)
```

recebe uma mensagem JSON contendo:

```json
{
  "source": "APP_01",
  "content": "turn_on"
}
```

Os campos são extraídos e utilizados para atualizar:

```text
message(source, content)
```

O agente Bob percebe:

```text
message(Source, Message)
```

---

### 6.2 Sensing

A operação:

```java
sensing(String json)
```

recebe uma leitura no formato:

```json
{
  "device": "esp_01",
  "variable": "lightSensor",
  "value": 72
}
```

A informação é disponibilizada pela propriedade:

```text
read(device, variable, value)
```

O agente percebe:

```text
read(esp_01,lightSensor,72)
```

---

### 6.3 Monitoring

A operação:

```java
monitoring(String json)
```

recebe informações sobre o estado do atuador:

```json
{
  "device": "esp_01",
  "actuator": "led",
  "mode": "on"
}
```

Essas informações são disponibilizadas por:

```text
status(device, actuator, mode)
```

O agente percebe:

```text
status(esp_01,led,on)
```

---

### 6.4 Acting

A operação:

```java
acting(String action)
```

recebe um termo Jason, por exemplo:

```text
action(esp_01,setLed,on)
```

O artefato utiliza o mecanismo `DummyArt` para encaminhar a ação ao endpoint HTTP do Node-RED:

```text
http://nodered:1880/act
```

---

## 7. Node-RED

Os fluxos Node-RED estão armazenados em:

```text
node-red/flows/flows.json
```

A implementação possui quatro fluxos principais:

```text
Communication
Sensing
Monitoring
Acting
```

---

### 7.1 Communication

O Node-RED recebe mensagens no tópico:

```text
mqtt/jacamo/agent/bob
```

O fluxo converte o payload JSON e realiza uma requisição HTTP para:

```text
http://mas:8080/workspaces/wmqtt/artifacts/mqtt/operations/communication/execute
```

O fluxo é:

```text
MQTT IN
   │
   ▼
JSON
   │
   ▼
Build post agent
   │
   ▼
HTTP POST
   │
   ▼
MQTTArt.communication()
```

O payload utilizado atualmente é:

```json
{
  "source": "APP_01",
  "content": "turn_on"
}
```

---

### 7.2 Sensing

O ESP32 publica leituras no tópico:

```text
mqtt/jacamo/device/sensor
```

O Node-RED recebe a mensagem e chama:

```text
MQTTArt.sensing()
```

por meio de:

```text
http://mas:8080/workspaces/wmqtt/artifacts/mqtt/operations/sensing/execute
```

Fluxo:

```text
ESP32
   │
   ▼
MQTT Broker
   │
   ▼
MQTT IN
   │
   ▼
JSON
   │
   ▼
Build post sensor
   │
   ▼
HTTP POST
   │
   ▼
MQTTArt.sensing()
   │
   ▼
read(Device, Variable, Value)
   │
   ▼
Agent Bob
```

---

### 7.3 Monitoring

O ESP32 publica o estado do LED no tópico:

```text
mqtt/jacamo/device/status
```

O Node-RED encaminha a informação para:

```text
MQTTArt.monitoring()
```

por meio de:

```text
http://mas:8080/workspaces/wmqtt/artifacts/mqtt/operations/monitoring/execute
```

Fluxo:

```text
ESP32
   │
   ▼
MQTT Broker
   │
   ▼
MQTT IN
   │
   ▼
JSON
   │
   ▼
Build post actuator
   │
   ▼
HTTP POST
   │
   ▼
MQTTArt.monitoring()
   │
   ▼
status(Device, Actuator, Mode)
   │
   ▼
Agent Bob
```

---

### 7.4 Acting

Quando Bob produz:

```text
action(esp_01,setLed,on)
```

o artefato encaminha a ação para:

```text
http://nodered:1880/act
```

O Node-RED extrai:

```text
Device
Actuator
Command
```

e gera:

```json
{
  "device": "esp_01",
  "actuator": "setLed",
  "mode": "on"
}
```

O tópico MQTT é construído dinamicamente:

```text
mqtt/jacamo/device/{Device}
```

Assim:

```text
action(esp_01,setLed,on)
```

resulta em:

```text
mqtt/jacamo/device/esp_01
```

O ESP32 está inscrito exatamente nesse tópico.

Fluxo:

```text
Agent Bob
   │
   ▼
MQTTArt.acting()
   │
   ▼
HTTP POST /act
   │
   ▼
Node-RED
   │
   ▼
Extract action command
   │
   ▼
MQTT OUT
   │
   ▼
mqtt/jacamo/device/esp_01
   │
   ▼
Mosquitto
   │
   ▼
ESP32
```

---

## 8. Broker Mosquitto

O broker MQTT utiliza a imagem:

```text
eclipse-mosquitto:2
```

e é configurado em:

```text
mosquitto/config/mosquitto.conf
```

A porta utilizada é:

```text
1883
```

A configuração exige autenticação:

```text
listener 1883
allow_anonymous false
password_file /mosquitto/config/passwd
```

O broker participa da rede Docker:

```text
jacamo-nodered-net
```

---

## 9. ESP32

O código do ESP32 está em:

```text
sketchESPtoMQTTjson/sketchESPtoMQTTjson.ino
```

O dispositivo utiliza:

* Wi-Fi;
* `PubSubClient`;
* `ArduinoJson`;
* LED integrado no GPIO 2.

O identificador MQTT do dispositivo é:

```text
esp_01
```

---

### 9.1 Tópicos MQTT

O ESP32 publica leituras em:

```text
mqtt/jacamo/device/sensor
```

Publica estados do LED em:

```text
mqtt/jacamo/device/status
```

e recebe ações em:

```text
mqtt/jacamo/device/esp_01
```

---

### 9.2 Sensing

O sensor é atualmente simulado por valores aleatórios entre 0 e 100.

A informação publicada possui o formato:

```json
{
  "device": "esp_01",
  "variable": "lightSensor",
  "value": 72
}
```

---

### 9.3 Acting

O ESP32 recebe ações no tópico:

```text
mqtt/jacamo/device/esp_01
```

O payload possui o formato:

```json
{
  "device": "esp_01",
  "actuator": "setLed",
  "mode": "on"
}
```

ou:

```json
{
  "device": "esp_01",
  "actuator": "setLed",
  "mode": "off"
}
```

O dispositivo verifica:

```text
device == esp_01
```

e:

```text
actuator == setLed
```

antes de executar a ação.

---

### 9.4 Monitoring

Depois da atuação, o ESP32 publica o estado do LED em:

```text
mqtt/jacamo/device/status
```

com o formato:

```json
{
  "device": "esp_01",
  "actuator": "led",
  "mode": "on"
}
```

ou:

```json
{
  "device": "esp_01",
  "actuator": "led",
  "mode": "off"
}
```

---

## 10. Cliente Python

O arquivo:

```text
AppPubSubMQTT.py
```

implementa um cliente MQTT utilizando:

```text
paho.mqtt.client
```

O programa:

1. conecta ao broker MQTT;
2. assina o tópico do ESP32;
3. recebe mensagens publicadas pelo dispositivo;
4. solicita `source` e `content` pelo terminal;
5. publica a mensagem no tópico do agente Bob.

O tópico utilizado para publicação é:

```text
mqtt/jacamo/agent/bob
```

O tópico utilizado para assinatura é:

```text
mqtt/jacamo/device/esp_01
```

A mensagem publicada possui atualmente o formato:

```json
{
  "source": "APP_01",
  "content": "turn_on"
}
```

O valor de `content` pode ser utilizado pelo agente para produzir as ações:

```text
turn_on → action(esp_01,setLed,on)
```

ou:

```text
turn_off → action(esp_01,setLed,off)
```

---

## 11. Docker Compose

Os serviços são definidos em:

```text
docker-compose.yml
```

### JaCaMo

```text
image: jomifred/jacamo:1.1
```

Portas:

```text
8080
3272
```

### Node-RED

```text
image: nodered/node-red:1.0.4
```

Porta:

```text
1880
```

### Mosquitto

```text
image: eclipse-mosquitto:2
```

Porta:

```text
1883
```

Os três serviços utilizam a rede:

```text
jacamo-nodered-net
```

---

## 12. Execução

Na raiz do projeto, execute:

```bash
docker compose up
```

Os principais serviços estarão disponíveis em:

```text
Node-RED:
http://localhost:1880

JaCaMo REST:
http://localhost:8080

Jason:
http://localhost:3272

MQTT:
localhost:1883
```

Para encerrar os containers:

```bash
docker compose down
```

---

## 13. Fluxos completos

### 13.1 Communication

```text
Python App
    │
    │ MQTT
    ▼
mqtt/jacamo/agent/bob
    │
    ▼
Mosquitto
    │
    ▼
Node-RED
    │ HTTP
    ▼
MQTTArt.communication()
    │
    ▼
message(Source, Content)
    │
    ▼
Agent Bob
```

---

### 13.2 Sensing

```text
ESP32
    │
    │ MQTT
    ▼
mqtt/jacamo/device/sensor
    │
    ▼
Mosquitto
    │
    ▼
Node-RED
    │ HTTP
    ▼
MQTTArt.sensing()
    │
    ▼
read(Device, Variable, Value)
    │
    ▼
Agent Bob
```

---

### 13.3 Monitoring

```text
ESP32
    │
    │ MQTT
    ▼
mqtt/jacamo/device/status
    │
    ▼
Mosquitto
    │
    ▼
Node-RED
    │ HTTP
    ▼
MQTTArt.monitoring()
    │
    ▼
status(Device, Actuator, Mode)
    │
    ▼
Agent Bob
```

---

### 13.4 Acting

```text
Agent Bob
    │
    │ action(esp_01,setLed,on)
    ▼
MQTTArt.acting()
    │
    │ HTTP
    ▼
Node-RED /act
    │
    ▼
Extract action command
    │
    ▼
mqtt/jacamo/device/esp_01
    │
    ▼
Mosquitto
    │
    ▼
ESP32
    │
    ├── executa ação
    │
    └── publica status
```

---

## 14. Modelo conceitual atual

O arquivo `architecture` apresenta o modelo conceitual atual da integração:

```text
 _____________________________________
|                 THING               |
|                   │                 |
|        ┌──────────┼──────────┐      |
|        │          │          │      |
|     Property    Action      Event   |
|        │          │          │      |
| lightSensor    setLed     ledStatus |
|_____________________________________|
```

A correspondência conceitual apresentada no projeto é:

| JSON type     | Semântica         |
| ------------- | ----------------- |
| `request`     | Action            |
| `observation` | Property          |
| `status`      | Event             |
| `description` | Thing Description |

O modelo também apresenta:

```text
Communication: request (action)
    target = esp_01
    action = turn_on
```

```text
Sensing: read (property)
    target = esp_01
    property = lightSensor
    value = 72
```

```text
Monitoring: status (event)
    target = esp_01
    event = ledStatus
    data = on
```

```text
Acting:
    target = esp_01
    action = setLed
    command = on
```

As representações utilizadas no modelo são:

```text
request(esp_01,setLed,on)

read(esp_01,lightSensor,72)

status(esp_01,led,on)

action(esp_01,setLed,on)
```

A descrição conceitual de uma Thing considera:

```text
properties
actions
events
```

---

## 15. Correspondência entre o modelo e a implementação

A implementação atual utiliza as seguintes operações no artefato:

| Operação          | Informação processada | Representação percebida pelo agente |
| ----------------- | --------------------- | ----------------------------------- |
| `communication()` | Mensagem              | `message(Source, Content)`          |
| `sensing()`       | Propriedade           | `read(Device, Variable, Value)`     |
| `monitoring()`    | Evento/status         | `status(Device, Actuator, Mode)`    |
| `acting()`        | Ação                  | `action(Device, Actuator, Command)` |

O fluxo completo pode ser resumido como:

```text
Communication
    ↓
message(Source, Content)

Sensing
    ↓
read(Device, Variable, Value)

Monitoring
    ↓
status(Device, Actuator, Mode)

Acting
    ↓
action(Device, Actuator, Command)
```

---

## 16. Resumo

A implementação integra:

```text
JaCaMo
   │
   ├── Jason Agent
   │
   └── CArtAgO MQTTArt
           │
           │ HTTP/REST
           ▼
       Node-RED
           │
           │ MQTT
           ▼
       Mosquitto
           │
           │ MQTT/Wi-Fi
           ▼
         ESP32
```

O artefato `MQTTArt` organiza a interação em quatro operações:

```text
communication()
sensing()
monitoring()
acting()
```

Essas operações estabelecem a integração entre:

```text
mensagens
leituras
estados
ações
```

por meio de:

```text
Jason ↔ CArtAgO ↔ Node-RED ↔ MQTT ↔ ESP32
```

A implementação atual utiliza o dispositivo:

```text
esp_01
```

e os principais tópicos MQTT são:

```text
mqtt/jacamo/agent/bob
mqtt/jacamo/device/sensor
mqtt/jacamo/device/status
mqtt/jacamo/device/esp_01
```

Esta documentação descreve a implementação e o modelo conceitual presentes na versão atual do projeto.

