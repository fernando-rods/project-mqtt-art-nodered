# JaCaMo + Node-RED + MQTT

Este projeto apresenta uma integração multiagente para Internet das Coisas (IoT) com **JaCaMo**, **Node-RED**, **Mosquitto MQTT** e **ESP32**, utilizando o artefato `MQTTArt` como elemento intermediário entre o agente e dispositivos IoT e serviços conectados via MQTT.

A versão atual constitui a **baseline funcional de integração entre agente e Thing**, implementando quatro tipos de interação:

- **Communication** — recebimento de requisições destinadas ao agente;
- **Reporting** — recebimento de valores de propriedades reportados por uma Thing;
- **Acting** — encaminhamento de ações produzidas pelo agente para uma Thing;
- **Monitoring** — recebimento de eventos gerados por uma Thing.

As representações utilizadas na integração são:

```text
request(Target, Operation, Data)
report(Thing, Property, Value)
action(Thing, Action, Input)
event(Thing, Event, Data)
```

A Thing utilizada no experimento é o dispositivo IoT `esp_01`, que disponibiliza:

```text
Thing esp_01
│
├── Properties
│   ├── lightSensor
│   └── led
│
├── Action
│   └── setLed
│
└── Events
    ├── underLight
    └── overLight
```

A infraestrutura atual fornece uma base funcional para futuras etapas relacionadas à descoberta de recursos, descrição semântica de Things, seleção de informações e construção e utilização de conhecimento pelo agente.

---

## 1. Arquitetura

A arquitetura atual é composta pelos seguintes elementos:

```text
                         ┌──────────────────────┐
                         │      Agent Bob       │
                         │        Jason         │
                         └──────────┬───────────┘
                                    │
                                  CArtAgO
                                    │
                         ┌──────────▼───────────┐
                         │       MQTTArt        │
                         │       Artifact       │
                         └──────────┬───────────┘
                                    │
                                 HTTP/REST
                                    │
                         ┌──────────▼───────────┐
                         │       Node-RED       │
                         └──────────┬───────────┘
                                    │
                                   MQTT
                                    │
                         ┌──────────▼───────────┐
                         │       Mosquitto      │
                         │      MQTT Broker     │
                         └──────────┬───────────┘
                                    │
                                 MQTT/Wi-Fi
                                    │
                         ┌──────────▼───────────┐
                         │        ESP32         │
                         │       esp_01         │
                         └──────────────────────┘
```

O **Node-RED** realiza a integração entre as interfaces HTTP/REST do JaCaMo e o broker MQTT.

Os fluxos de informação são bidirecionais:

```text
Thing → Mosquitto → Node-RED → MQTTArt → Agent
```

e:

```text
Agent → MQTTArt → Node-RED → Mosquitto → Thing
```

A arquitetura também permite a participação de aplicações externas ou outros clientes MQTT, como o cliente Python utilizado para gerar requisições destinadas ao agente.

---

## 2. Componentes

O projeto utiliza:

- **JaCaMo 1.1** — plataforma para execução do sistema multiagente;
- **Jason** — linguagem utilizada para implementação do agente Bob;
- **CArtAgO** — framework utilizado para criação do artefato `MQTTArt`;
- **Node-RED 1.0.4** — integração entre HTTP/REST e MQTT;
- **Eclipse Mosquitto 2** — broker MQTT;
- **ESP32** — Thing utilizada no experimento;
- **Python + Paho MQTT** — cliente auxiliar para testes de Communication;
- **Docker Compose** — orquestração dos serviços JaCaMo, Node-RED e Mosquitto.

---

## 3. Estrutura do projeto

A estrutura principal do projeto é:

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
├── docker-compose.yml
└── README.md
```

O projeto também pode conter arquivos auxiliares gerados durante a execução do JaCaMo, Node-RED e Python.

Os arquivos de configuração contendo credenciais devem ser configurados individualmente no ambiente de execução e não devem expor credenciais reais quando o projeto for compartilhado.

---

## 4. Aplicação JaCaMo

A aplicação JaCaMo está definida em:

```text
app_jacamo/main.jcm
```

O agente Bob utiliza o workspace `wmqtt` e o artefato:

```text
mqtt: net.MQTTArt("http://nodered:1880/act")
```

O endpoint:

```text
http://nodered:1880/act
```

é utilizado pelo `MQTTArt` para encaminhar ao Node-RED as ações produzidas pelo agente.

A API REST do JaCaMo é disponibilizada na porta:

```text
8080
```

e o servidor Jason na porta:

```text
3272
```

---

## 5. Modelo conceitual de interação

A arquitetura distingue a comunicação com o agente das interações da Thing:

```text
                         Sistema
                            │
              ┌─────────────┴─────────────┐
              │                           │
        Communication              Thing Interaction
              │                           │
           request              ┌─────────┼─────────┐
                                │         │         │
                             Property   Action     Event
                                │         │         │
                              report    action     event
```

As representações utilizadas são:

### Communication

```text
request(Target, Operation, Data)
```

Exemplo:

```text
request(esp_01,setLed,on)
```

### Reporting

```text
report(Thing, Property, Value)
```

Exemplos:

```text
report(esp_01,lightSensor,45)
report(esp_01,led,on)
```

### Acting

```text
action(Thing, Action, Input)
```

Exemplo:

```text
action(esp_01,setLed,on)
```

### Monitoring

```text
event(Thing, Event, Data)
```

Exemplos:

```text
event(esp_01,underLight,12)
event(esp_01,overLight,91)
```

Na camada conceitual da Thing:

```text
Thing esp_01
│
├── Property
│   ├── lightSensor
│   └── led
│
├── Action
│   └── setLed
│
└── Event
    ├── underLight
    └── overLight
```

> **Nota:** `lightSensor` e `led` são tratados como **Properties** da Thing. `setLed` é uma **Action** e `underLight`/`overLight` são **Events**. Os termos `sensorState(...)`, `actuatorState(...)` e `eventState(...)` são representações internas do estado/conhecimento operacional do agente e não categorias de interação da WoT.

---

## 6. Agente Bob

O agente está implementado em:

```text
app_jacamo/bob.asl
```

O objetivo inicial é:

```text
!start.
```

produzindo:

```text
Bob is running
```

O agente possui planos para as quatro formas de interação.

### 6.1 Communication

O artefato disponibiliza ao agente:

```text
request(Target, Operation, Data)
```

Quando a operação é:

```text
setLed
```

Bob produz uma ação:

```text
action(Target,setLed,Data)
```

Por exemplo:

```text
request(esp_01,setLed,on)
```

gera:

```text
action(esp_01,setLed,on)
```

Para outras requisições, o agente possui um plano genérico de tratamento e impressão da mensagem.

---

### 6.2 Reporting

O agente percebe:

```text
report(Thing, Property, Value)
```

Para a propriedade:

```text
lightSensor
```

o valor é armazenado como:

```text
sensorState(Thing, Property, Value)
```

e submetido à classificação:

```text
0 ≤ Value < 30       → LOW
30 ≤ Value < 70      → NORMAL
70 ≤ Value ≤ 100     → HIGH
Value < 0 ou > 100   → OUTSIDE RANGE
```

Exemplo:

```text
report(esp_01,lightSensor,36)
```

resulta em:

```text
sensorState(esp_01,lightSensor,36)
```

e:

```text
Device: esp_01 - sensor: lightSensor = 36 (Measurement NORMAL)
```

Para a propriedade:

```text
led
```

o agente mantém:

```text
actuatorState(Thing, Property, Value)
```

Exemplo:

```text
report(esp_01,led,on)
```

resulta em:

```text
actuatorState(esp_01,led,on)
```

A separação entre `sensorState` e `actuatorState` evita que uma atualização do sensor substitua uma atualização do estado do LED dentro da representação interna do agente.

---

### 6.3 Acting

As ações são representadas por:

```text
action(Thing, Action, Input)
```

Exemplos:

```text
action(esp_01,setLed,on)
action(esp_01,setLed,off)
```

Quando Bob recebe:

```text
request(esp_01,setLed,on)
```

o plano correspondente produz:

```text
action(esp_01,setLed,on)
```

A ação é enviada ao artefato pela operação:

```text
acting(...)
```

---

### 6.4 Monitoring

O agente percebe:

```text
event(Thing, Event, Data)
```

e mantém uma representação interna:

```text
eventState(Thing, Event, Data)
```

Exemplos:

```text
event(esp_01,underLight,12)
event(esp_01,overLight,91)
```

resultam, respectivamente, em:

```text
eventState(esp_01,underLight,12)
eventState(esp_01,overLight,91)
```

e são apresentados ao agente como alertas de evento.

Na versão atual, o evento é registrado e apresentado ao agente. A utilização do evento como gatilho para uma nova decisão ou ação pode ser incorporada em uma etapa posterior.

---

## 7. Artefato CArtAgO `MQTTArt`

O artefato está implementado em:

```text
app_jacamo/src/env/net/MQTTArt.java
```

O `MQTTArt` possui quatro operações:

```text
communication()
reporting()
acting()
monitoring()
```

e três propriedades observáveis utilizadas pelo agente:

```text
request
report
event
```

### 7.1 Communication

A operação:

```text
communication(String json)
```

recebe:

```json
{
  "target": "esp_01",
  "operation": "setLed",
  "data": "on"
}
```

e atualiza:

```text
request(Target, Operation, Data)
```

---

### 7.2 Reporting

A operação:

```text
reporting(String json)
```

recebe:

```json
{
  "thing": "esp_01",
  "property": "lightSensor",
  "value": 43
}
```

e atualiza:

```text
report(Thing, Property, Value)
```

---

### 7.3 Monitoring

A operação:

```text
monitoring(String json)
```

recebe:

```json
{
  "thing": "esp_01",
  "event": "overLight",
  "data": 91
}
```

ou:

```json
{
  "thing": "esp_01",
  "event": "underLight",
  "data": 12
}
```

e atualiza:

```text
event(Thing, Event, Data)
```

---

### 7.4 Acting

A operação:

```text
acting(String action)
```

recebe uma representação Jason:

```text
action(Thing, Action, Input)
```

e encaminha a ação para:

```text
http://nodered:1880/act
```

---

### 7.5 Conversão JSON → Jason

O artefato realiza a conversão dos valores JSON para termos Jason.

São tratados:

- números como números Jason;
- booleanos como átomos `true`/`false`;
- strings como átomos;
- objetos e outros valores JSON estruturados como strings Jason.

Essa conversão permite que valores numéricos de sensores sejam utilizados diretamente nas regras de comparação do agente.

Por exemplo:

```json
{
  "value": 43
}
```

é convertido para um número Jason, permitindo:

```text
Value >= 30
Value < 70
```

---

## 8. Node-RED

O Node-RED realiza a integração entre HTTP e MQTT.

Os fluxos atuais estão organizados em quatro funções principais.

### 8.1 Communication

```text
MQTT IN
mqtt/jacamo/agent/bob
        │
        ▼
      JSON
        │
        ▼
HTTP POST
        │
        ▼
MQTTArt.communication()
```

---

### 8.2 Reporting

```text
MQTT IN
mqtt/jacamo/device/properties
        │
        ▼
      JSON
        │
        ▼
HTTP POST
        │
        ▼
MQTTArt.reporting()
```

---

### 8.3 Monitoring

```text
MQTT IN
mqtt/jacamo/device/events
        │
        ▼
      JSON
        │
        ▼
HTTP POST
        │
        ▼
MQTTArt.monitoring()
```

---

### 8.4 Acting

```text
HTTP IN
/act
  │
  ▼
Parse action(Thing,Action,Input)
  │
  ▼
MQTT OUT
mqtt/jacamo/device/{Thing}
```

O tópico de destino é determinado dinamicamente pelo identificador da Thing.

Para:

```text
action(esp_01,setLed,on)
```

o tópico utilizado é:

```text
mqtt/jacamo/device/esp_01
```

---

## 9. Mosquitto MQTT

O broker utilizado é o **Eclipse Mosquitto 2**.

A configuração está em:

```text
mosquitto/config/mosquitto.conf
```

A configuração utilizada é:

```text
listener 1883
allow_anonymous false
password_file /mosquitto/config/passwd
```

O broker utiliza a porta:

```text
1883
```

A autenticação é realizada por usuário e senha.

O arquivo:

```text
mosquitto/config/passwd
```

deve ser configurado localmente. Credenciais reais não devem ser expostas em arquivos destinados à distribuição pública do projeto.

---

## 10. ESP32

O sketch do ESP32 está localizado em:

```text
sketchESPtoMQTTjson/sketchESPtoMQTTjson.ino
```

O firmware utiliza:

- `WiFi.h`;
- `PubSubClient.h`;
- `ArduinoJson.h`;
- LED integrado no GPIO 2.

O identificador da Thing é:

```text
esp_01
```

A leitura do sensor de luminosidade é atualmente simulada por valores aleatórios entre `0` e `100`.

O intervalo utilizado para a publicação periódica é:

```text
10 segundos
```

As credenciais de Wi-Fi e MQTT presentes no código são `placeholders`:

```text
YOUR_WIFI_SSID
YOUR_WIFI_PASSWORD
YOUR_MQTT_BROKER
YOUR_MQTT_USER
YOUR_MQTT_PASSWORD
```

Esses valores devem ser substituídos localmente para a execução do experimento.

---

### 10.1 Tópicos MQTT

O ESP32:

**publica Properties em:**

```text
mqtt/jacamo/device/properties
```

**recebe Actions em:**

```text
mqtt/jacamo/device/esp_01
```

**publica Events em:**

```text
mqtt/jacamo/device/events
```

---

### 10.2 Reporting do sensor

A leitura do sensor é publicada como:

```json
{
  "thing": "esp_01",
  "property": "lightSensor",
  "value": 43
}
```

O valor é numérico no JSON, permitindo que o agente Jason realize comparações como:

```text
Value >= 30
Value < 70
```

---

### 10.3 Reporting do estado do LED

O estado do LED é publicado pelo próprio ESP32 após uma atuação.

Para LED ligado:

```json
{
  "thing": "esp_01",
  "property": "led",
  "value": "on"
}
```

Para LED desligado:

```json
{
  "thing": "esp_01",
  "property": "led",
  "value": "off"
}
```

Essas mensagens são encaminhadas pelo fluxo de `Reporting` e percebidas pelo agente como:

```text
report(esp_01,led,on)
```

ou:

```text
report(esp_01,led,off)
```

---

### 10.4 Acting

O ESP32 está inscrito em:

```text
mqtt/jacamo/device/esp_01
```

e recebe ações no formato:

```json
{
  "thing": "esp_01",
  "action": "setLed",
  "input": "on"
}
```

ou:

```json
{
  "thing": "esp_01",
  "action": "setLed",
  "input": "off"
}
```

O dispositivo verifica:

```text
thing == esp_01
```

e:

```text
action == setLed
```

antes de executar a ação.

Após executar a ação, o ESP32 publica o novo estado do LED através do fluxo de Reporting.

Assim, o fluxo completo é:

```text
Agent
  │
  │ action(esp_01,setLed,on)
  ▼
ESP32
  │
  │ executa ação
  ▼
LED = ON
  │
  │ report
  ▼
Agent
```

---

### 10.5 Monitoring e geração de eventos

A função `checkSensors()` verifica a leitura do sensor.

Quando:

```text
sensorValue < 15
```

a Thing gera:

```text
underLight
```

com uma mensagem como:

```json
{
  "thing": "esp_01",
  "event": "underLight",
  "data": 12
}
```

Quando:

```text
sensorValue > 85
```

a Thing gera:

```text
overLight
```

com uma mensagem como:

```json
{
  "thing": "esp_01",
  "event": "overLight",
  "data": 91
}
```

O evento é publicado em:

```text
mqtt/jacamo/device/events
```

e percorre:

```text
ESP32
   ↓
Mosquitto
   ↓
Node-RED
   ↓
MQTTArt.monitoring()
   ↓
event(esp_01,Event,Data)
   ↓
Bob
```

Quando a leitura está entre os limites definidos para geração de alerta, nenhum evento de luminosidade é publicado.

---

## 11. Cliente Python

O arquivo:

```text
AppPubSubMQTT.py
```

implementa um cliente MQTT utilizando:

```text
paho.mqtt.client
```

Na versão atual, o cliente é utilizado para testar o fluxo de **Communication**.

O cliente:

1. conecta ao broker MQTT;
2. assina o tópico do dispositivo;
3. recebe mensagens publicadas pelo ESP32;
4. permite informar pelo terminal uma requisição;
5. publica a requisição no tópico do agente Bob;
6. aguarda a publicação MQTT;
7. verifica o código de retorno da publicação.

O tópico de publicação é:

```text
mqtt/jacamo/agent/bob
```

O tópico de assinatura é:

```text
mqtt/jacamo/device/esp_01
```

A entrada esperada no terminal é:

```text
target operation data
```

Por exemplo, para ligar o LED:

```text
esp_01 setLed on
```

gera:

```json
{
  "target": "esp_01",
  "operation": "setLed",
  "data": "on"
}
```

que é publicado em:

```text
mqtt/jacamo/agent/bob
```

E para desligar do LED:

```text
esp_01 setLed off
```

O resultado da publicação é verificado utilizando:

```python
info = client.publish(TOPIC_PUBLISH, json_message)
info.wait_for_publish(timeout=1)
```

e:

```python
if info.rc == mqtt.MQTT_ERR_SUCCESS:
```

---

## 12. Docker Compose

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

## 13. Execução

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

Para executar em segundo plano:

```bash
docker compose up -d
```

Para acompanhar os logs:

```bash
docker compose logs -f
```

Para acompanhar somente o agente:

```bash
docker compose logs -f mas
```

Para encerrar os containers:

```bash
docker compose down
```

---

## 14. Fluxos completos

### 14.1 Communication

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
request(Target, Operation, Data)
   │
   ▼
Agent Bob
```

Quando a requisição corresponde a:

```text
request(esp_01,setLed,on)
```

Bob produz:

```text
action(esp_01,setLed,on)
```

---

### 14.2 Reporting

```text
ESP32
   │
   ├── lightSensor
   │
   └── led
        │
        │ MQTT
        ▼
mqtt/jacamo/device/properties
        │
        ▼
Mosquitto
        │
        ▼
Node-RED
        │ HTTP
        ▼
MQTTArt.reporting()
        │
        ▼
report(Thing, Property, Value)
        │
        ▼
Agent Bob
        │
        ├── sensorState(...)
        │
        └── actuatorState(...)
```

---

### 14.3 Acting

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
   └── publica report(esp_01,led,on)
```

---

### 14.4 Monitoring

```text
ESP32
   │
   │ Event
   ▼
mqtt/jacamo/device/events
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
event(esp_01,Event,Data)
   │
   ▼
Agent Bob
   │
   ▼
eventState(...)
```

Os eventos gerados atualmente são:

```text
underLight
overLight
```

---

### 14.5 Ciclo completo de atuação e feedback

Uma das características importantes da baseline atual é o ciclo completo:

```text
Python
   │
   │ request
   ▼
Agent Bob
   │
   │ action
   ▼
MQTTArt
   │
   ▼
Node-RED
   │
   ▼
Mosquitto
   │
   ▼
ESP32
   │
   │ executa setLed
   ▼
LED
   │
   │ report
   ▼
Mosquitto
   │
   ▼
Node-RED
   │
   ▼
MQTTArt
   │
   ▼
Agent Bob
```

Esse fluxo demonstra que uma ação produzida pelo agente pode resultar em uma mudança de estado na Thing e que esse novo estado pode retornar ao agente como uma Property reportada.

---

## 15. Correspondência entre modelo e implementação

A implementação atual pode ser resumida da seguinte forma:

| Camada | Operação MQTTArt | Representação externa | Representação no agente |
|---|---|---|---|
| Communication | `communication()` | `request(Thing, Operation, Data)` | `request(...)` |
| Reporting | `reporting()` | `report(Thing, Property, Value)` | `sensorState(...)` / `actuatorState(...)` |
| Acting | `acting()` | `action(Thing, Action, Input)` | `action(...)` |
| Monitoring | `monitoring()` | `event(Thing, Event, Data)` | `eventState(...)` |

A distinção entre a camada semântica da Thing e a representação interna do agente é importante.

### Camada semântica da Thing

```text
WoT Thing
    │
    ├── Property
    │     ├── lightSensor
    │     └── led
    │
    ├── Action
    │     └── setLed
    │
    └── Event
          ├── underLight
          └── overLight
```

### Representação interna do agente

```text
Agent knowledge/state
    │
    ├── sensorState(Thing, Property, Value)
    ├── actuatorState(Thing, Property, Value)
    └── eventState(Thing, Event, Data)
```

Assim, `sensorState(...)`, `actuatorState(...)` e `eventState(...)` não representam diretamente categorias de interação da WoT. Eles representam estados e informações mantidos internamente pelo agente.

---

## 16. Baseline funcional

A versão atual estabelece uma baseline funcional em que uma Thing participa efetivamente dos diferentes tipos de interação.

A `esp_01`:

- publica leituras de `lightSensor`;
- publica o estado do atuador `led`;
- recebe a Action `setLed`;
- gera os Events `underLight` e `overLight`.

O agente Bob:

- recebe `request(...)`;
- produz `action(...)`;
- recebe `report(...)`;
- mantém estados internos de sensores e atuadores;
- classifica as leituras do sensor;
- recebe `event(...)`;
- mantém os eventos como estado interno.

A aplicação Python fornece a entrada externa para o fluxo de Communication.

Dessa forma, a baseline atual cobre:

```text
                 ┌─────────────────────┐
                 │     Communication   │
                 └──────────┬──────────┘
                            ▼
                            │
                         Agent
                            │
            ┌───────────────┼────────────────┐
            ▲               ▼                ▲
            │               │                │
         Reporting        Acting         Monitoring
            ▲               │                ▲
            │               ▼                │
            └──────────── Thing ─────────────┘
```

---

## 17. Estado atual da implementação

A versão atual constitui uma **baseline funcional de integração entre agente e Thing**, utilizando:

```text
Jason
  ↕
CArtAgO / MQTTArt
  ↕
HTTP/REST
  ↕
Node-RED
  ↕
Mosquitto MQTT
  ↕
ESP32 / Thing
```

Os quatro fluxos estão implementados:

```text
Communication
Reporting
Acting
Monitoring
```

A implementação estabelece uma separação entre:

1. **informações provenientes do ambiente**, representadas pelas mensagens `request`, `report` e `event`;
2. **ações produzidas pelo agente**, representadas por `action`;
3. **representações internas do agente**, como `sensorState`, `actuatorState` e `eventState`.

A infraestrutura atual permite demonstrar:

```text
Thing → Property → Agent
Thing → Event → Agent
Agent → Action → Thing
Application → Request → Agent
```

Essa infraestrutura será utilizada como base para a evolução do projeto de pesquisa.

---

## 18. Próximos passos

Com a baseline de integração funcional, as próximas etapas podem concentrar-se na camada de conhecimento e descoberta.

A infraestrutura atual pode ser mantida como camada de comunicação, percepção e atuação enquanto novos mecanismos são acrescentados sobre ela.

As etapas futuras poderão explorar:

```text
Discovery
    ↓
Selection
    ↓
Building
    ↓
Integration
    ↓
Utilization
```

Essas etapas representam a evolução prevista do projeto de pesquisa.

A integração com descrições semânticas, como **Thing Descriptions (TDs)**, poderá utilizar a infraestrutura atual para conectar informações descobertas sobre Things às representações e mecanismos de decisão do agente.

Portanto, a baseline atual deve ser considerada a infraestrutura experimental sobre a qual serão desenvolvidos os mecanismos de descoberta e construção e utilização de conhecimento.