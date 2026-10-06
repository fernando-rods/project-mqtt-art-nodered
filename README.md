# JaCaMo + Node-RED + MQTT

Este projeto apresenta uma integração entre **JaCaMo**, **Node-RED**, **Mosquitto MQTT** e **ESP32**, utilizando o artefato `MQTTArt` como elemento intermediário entre o agente e os dispositivos/serviços conectados por MQTT.

A versão atual foi revisada e validada com quatro operações:

-   **Communication** --- recebimento de requisições destinadas ao agente;
-   **Reporting** --- recebimento de valores de propriedades reportados por uma Thing;
-   **Acting** --- encaminhamento de ações produzidas pelo agente para uma Thing;
-   **Monitoring** --- recebimento de eventos gerados por uma Thing.

A arquitetura utiliza as representações:

``` text
request(Target, Operation, Data)
report(Thing, Property, Value)
action(Thing, Action, Input)
event(Thing, Event, Data)
```

O dispositivo IoT utilizado no experimento é o `esp_01`.

------------------------------------------------------------------------

## 1. Arquitetura

A arquitetura atual é composta pelos seguintes elementos:

``` text
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
                         │  lightSensor + LED   │
                         └──────────────────────┘
```

O **Node-RED** realiza a integração entre as interfaces HTTP/REST do JaCaMo e o broker MQTT.

Os fluxos são bidirecionais:

``` text
Thing → Mosquitto → Node-RED → Artifact → Agent
```

e:

``` text
Agent → Artifact → Node-RED → Mosquitto → Thing
```

------------------------------------------------------------------------

## 2. Componentes

O projeto utiliza:

-   **JaCaMo 1.1** --- plataforma para execução do sistema multiagente;
-   **Jason** --- linguagem utilizada para implementação do agente Bob;
-   **CArtAgO** --- framework utilizado para criação do artefato `MQTTArt`;
-   **Node-RED 1.0.4** --- integração entre HTTP/REST e MQTT;
-   **Eclipse Mosquitto 2** --- broker MQTT;
-   **ESP32** --- Thing utilizada no experimento;
-   **Python + Paho MQTT** --- cliente auxiliar para testes e publicação de mensagens MQTT;
-   **Docker Compose** --- orquestração dos serviços JaCaMo, Node-RED e Mosquitto.

------------------------------------------------------------------------

## 3. Estrutura do projeto

``` text
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
│   └── sketchESP32toMQTTjson.ino
│
├── AppPubSubMQTT.py
├── architecture
├── docker-compose.yml
└── readme.md
```

O diretório também contém arquivos auxiliares gerados durante a execução do JaCaMo/Node-RED. Os arquivos de configuração de credenciais devem ser atualizados com as informações do usuário.

------------------------------------------------------------------------

## 4. Aplicação JaCaMo

A aplicação JaCaMo está definida em:

``` text
app_jacamo/main.jcm
```

O agente Bob utiliza o workspace `wmqtt` e o artefato:

``` text
mqtt: net.MQTTArt("http://nodered:1880/act")
```

O endpoint:

``` text
http://nodered:1880/act
```

é utilizado pelo `MQTTArt` para encaminhar ao Node-RED as ações produzidas pelo agente.

A API REST do JaCaMo é disponibilizada na porta:

``` text
8080
```

e o servidor Jason na porta:

``` text
3272
```

------------------------------------------------------------------------

## 5. Modelo conceitual de interação

A arquitetura atual distingue a comunicação com o agente das interações semânticas com uma Thing:

``` text
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
                              report     action     event
```

As representações utilizadas são:

### Communication

``` text
request(Target, Operation, Data)
```

Exemplo:

``` text
request(esp_01,setLed,on)
```

### Reporting

``` text
report(Thing, Property, Value)
```

Exemplos:

``` text
report(esp_01,lightSensor,45)
report(esp_01,led,on)
```

### Acting

``` text
action(Thing, Action, Input)
```

Exemplo:

``` text
action(esp_01,setLed,on)
```

### Monitoring

``` text
event(Thing, Event, Data)
```

Exemplo:

``` text
event(esp_01,overLight,75)
```

Na camada conceitual da Thing, são considerados:

``` text
Thing
│
├── Property
│   ├── lightSensor
│   └── led
│
├── Action
│   └── setLed
│
└── Event
    └── overLight
```

> **Nota:** `lightSensor` e `led` são tratados como **Properties** na camada semântica da Thing. `setLed` é uma **Action** e `overLight` é um **Event**. Os termos `sensorState(...)`, `actuatorState(...)` e `eventState(...)` são representações internas do conhecimento/estado operacional do agente e não categorias da WoT.

------------------------------------------------------------------------

## 6. Agente Bob

O agente está implementado em:

``` text
app_jacamo/bob.asl
```

O objetivo inicial é:

``` text
!start.
```

produzindo:

``` text
Bob is running
```

O agente possui planos para as quatro formas de interação.

### 6.1 Communication

O artefato disponibiliza ao agente:

``` text
request(Target, Operation, Data)
```

Quando a operação é:

``` text
setLed
```

Bob produz uma ação:

``` text
action(Target,setLed,Data)
```

Por exemplo:

``` text
request(esp_01,setLed,on)
```

gera:

``` text
action(esp_01,setLed,on)
```

Para outras requisições, o agente possui um plano genérico de tratamento e impressão da mensagem.

------------------------------------------------------------------------

### 6.2 Reporting

O agente percebe:

``` text
report(Thing, Property, Value)
```

Para a propriedade:

``` text
lightSensor
```

o valor é armazenado como conhecimento operacional:

``` text
sensorState(Thing, Property, Value)
```

e submetido à classificação:

``` text
0 ≤ Value < 30       → LOW
30 ≤ Value < 70      → NORMAL
70 ≤ Value ≤ 100     → HIGH
Value < 0 ou > 100   → OUTSIDE RANGE
```

Exemplo:

``` text
report(esp_01,lightSensor,36)
```

resulta em:

``` text
sensorState(esp_01,lightSensor,36)
```

e:

``` text
Device: esp_01 - sensor: lightSensor = 36 (Measurement NORMAL)
```

Para a propriedade:

``` text
led
```

o agente mantém:

``` text
actuatorState(Thing, Property, Value)
```

Exemplo:

``` text
report(esp_01,led,on)
```

resulta em:

``` text
actuatorState(esp_01,led,on)
```

Essa separação evita que uma atualização de estado do sensor substitua uma atualização de estado do LED dentro da mesma representação interna.

------------------------------------------------------------------------

### 6.3 Acting

As ações são representadas por:

``` text
action(Thing, Action, Input)
```

Exemplos:

``` text
action(esp_01,setLed,on)
action(esp_01,setLed,off)
```

Quando Bob recebe uma requisição:

``` text
request(esp_01,setLed,on)
```

o plano correspondente produz:

``` text
action(esp_01,setLed,on)
```

A ação é enviada ao artefato pela operação:

``` text
acting(...)
```

------------------------------------------------------------------------

### 6.4 Monitoring

O agente percebe:

``` text
event(Thing, Event, Data)
```

e mantém uma representação interna:

``` text
eventState(Thing, Event, Data)
```

Exemplo:

``` text
event(app_01,overLight,75)
```

produz:

``` text
eventState(app_01,overLight,75)
```

e:

``` text
Event alert => device: app_01 - event: overLight = 75
```

A versão atual registra o evento e o apresenta ao agente. A utilização do evento como gatilho para uma nova decisão/ação pode ser incorporada em uma etapa posterior.

------------------------------------------------------------------------

## 7. Artefato CArtAgO `MQTTArt`

O artefato está implementado em:

``` text
app_jacamo/src/env/net/MQTTArt.java
```

O `MQTTArt` possui quatro operações:

``` text
communication()
reporting()
acting()
monitoring()
```

e três propriedades observáveis utilizadas pelo agente:

``` text
request
report
event
```

O artefato também realiza a conversão dos valores JSON para termos Jason, preservando:

-   números como números Jason;
-   booleanos como átomos `true`/`false`;
-   strings como átomos;
-   valores JSON estruturados como strings Jason.

Essa conversão é importante para que valores numéricos de sensores possam ser utilizados diretamente nas regras de comparação do agente.

------------------------------------------------------------------------

### 7.1 `communication()`

Recebe:

``` text
request(Target, Operation, Data)
```

a partir de um JSON com a estrutura:

``` json
{
  "target": "esp_01",
  "operation": "setLed",
  "data": "on"
}
```

A informação é disponibilizada ao agente como:

``` text
request(esp_01,setLed,on)
```

------------------------------------------------------------------------

### 7.2 `reporting()`

Recebe:

``` text
report(Thing, Property, Value)
```

a partir de um JSON:

``` json
{
  "thing": "esp_01",
  "property": "lightSensor",
  "value": 43
}
```

ou:

``` json
{
  "thing": "esp_01",
  "property": "led",
  "value": "on"
}
```

A operação disponibiliza essas informações ao agente por meio da propriedade observável:

``` text
report(device,property,value)
```

------------------------------------------------------------------------

### 7.3 `acting()`

Recebe:

``` text
action(Thing, Action, Input)
```

por exemplo:

``` text
action(esp_01,setLed,on)
```

e utiliza o mecanismo fornecido pelo `DummyArt` para encaminhar a ação ao endpoint do Node-RED:

``` text
http://nodered:1880/act
```

------------------------------------------------------------------------

### 7.4 `monitoring()`

Recebe:

``` text
event(Thing, Event, Data)
```

a partir de um JSON:

``` json
{
  "thing": "app_01",
  "event": "overLight",
  "data": 75
}
```

A informação é disponibilizada ao agente por meio da propriedade observável:

``` text
event(device,event,data)
```

------------------------------------------------------------------------

## 8. Node-RED

Os fluxos Node-RED estão armazenados em:

``` text
node-red/flows/flows.json
```

A implementação possui quatro fluxos principais:

``` text
Communication
Reporting
Monitoring
Acting
```

------------------------------------------------------------------------

### 8.1 Communication

O Node-RED recebe mensagens no tópico:

``` text
mqtt/jacamo/agent/bob
```

e encaminha os dados para:

``` text
http://mas:8080/workspaces/wmqtt/artifacts/mqtt/operations/communication/execute
```

Fluxo:

``` text
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
   │
    ▼
request(Target, Operation, Data)
   │
    ▼
Agent Bob
```

------------------------------------------------------------------------

### 8.2 Reporting

O Node-RED recebe valores de propriedades no tópico:

``` text
mqtt/jacamo/device/properties
```

e encaminha para:

``` text
http://mas:8080/workspaces/wmqtt/artifacts/mqtt/operations/reporting/execute
```

Fluxo:

``` text
Thing
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
```

Payload de uma leitura:

``` json
{
  "thing": "esp_01",
  "property": "lightSensor",
  "value": 43
}
```

Payload do estado do LED:

``` json
{
  "thing": "esp_01",
  "property": "led",
  "value": "on"
}
```

------------------------------------------------------------------------

### 8.3 Monitoring

O Node-RED recebe eventos no tópico:

``` text
mqtt/jacamo/device/events
```

e encaminha para:

``` text
http://mas:8080/workspaces/wmqtt/artifacts/mqtt/operations/monitoring/execute
```

Fluxo:

``` text
Thing / Test Client
   │
   │ MQTT
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
event(Thing, Event, Data)
   │
    ▼
Agent Bob
```

Exemplo:

``` json
{
  "thing": "app_01",
  "event": "overLight",
  "data": 75
}
```

------------------------------------------------------------------------

### 8.4 Acting

Quando Bob produz:

``` text
action(esp_01,setLed,on)
```

o `MQTTArt` encaminha a ação para:

``` text
http://nodered:1880/act
```

O Node-RED interpreta o termo Jason e extrai:

``` text
Thing = esp_01
Action = setLed
Input = on
```

Em seguida, gera:

``` json
{
  "thing": "esp_01",
  "action": "setLed",
  "input": "on"
}
```

e publica no tópico:

``` text
mqtt/jacamo/device/esp_01
```

Fluxo:

``` text
Agent Bob
   │
   │ action(esp_01,setLed,on)
    ▼
MQTTArt.acting()
   │
   │ HTTP POST
    ▼
Node-RED /act
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

------------------------------------------------------------------------

## 9. Broker Mosquitto

O broker MQTT utiliza:

``` text
eclipse-mosquitto:2
```

e é configurado em:

``` text
mosquitto/config/mosquitto.conf
```

A porta utilizada é:

``` text
1883
```

A configuração atual exige autenticação:

``` text
listener 1883
allow_anonymous false
password_file /mosquitto/config/passwd
```

O broker participa da rede Docker:

``` text
jacamo-nodered-net
```

Os dados e logs do broker são persistidos nos diretórios:

``` text
mosquitto/data
mosquitto/log
```

------------------------------------------------------------------------

## 10. ESP32

O código da Thing está em:

``` text
sketchESPtoMQTTjson/sketchESP32toMQTTjson.ino
```

O dispositivo utiliza:

-   Wi-Fi;
-   `PubSubClient`;
-   `ArduinoJson`;
-   LED integrado no GPIO 2.

O identificador da Thing é:

``` text
esp_01
```

O sensor de luz é atualmente simulado por valores aleatórios entre `0` e `100`.

O intervalo de publicação utilizado no código é de:

``` text
10 segundos
```

------------------------------------------------------------------------

### 10.1 Tópicos MQTT

O ESP32 publica propriedades em:

``` text
mqtt/jacamo/device/properties
```

recebe ações em:

``` text
mqtt/jacamo/device/esp_01
```

e possui um tópico definido para eventos:

``` text
mqtt/jacamo/device/events
```

A publicação atual de sensor e estado do LED utiliza o tópico comum:

``` text
mqtt/jacamo/device/properties
```

------------------------------------------------------------------------

### 10.2 Reporting do sensor

A leitura simulada é publicada como:

``` json
{
  "thing": "esp_01",
  "property": "lightSensor",
  "value": 43
}
```

O valor é numérico no JSON, permitindo que o agente Jason realize comparações como:

``` text
Value >= 30
Value < 70
```

------------------------------------------------------------------------

### 10.3 Acting e estado do LED

O ESP32 está inscrito em:

``` text
mqtt/jacamo/device/esp_01
```

e espera uma ação no formato:

``` json
{
  "thing": "esp_01",
  "action": "setLed",
  "input": "on"
}
```

ou:

``` json
{
  "thing": "esp_01",
  "action": "setLed",
  "input": "off"
}
```

O dispositivo verifica:

``` text
thing == esp_01
```

e:

``` text
action == setLed
```

antes de executar a ação.

Depois da atuação, publica o estado do LED como uma propriedade:

``` json
{
  "thing": "esp_01",
  "property": "led",
  "value": "on"
}
```

ou:

``` json
{
  "thing": "esp_01",
  "property": "led",
  "value": "off"
}
```

Essa informação retorna ao fluxo de `reporting()` e é percebida pelo agente como:

``` text
report(esp_01,led,on)
```

ou:

``` text
report(esp_01,led,off)
```

------------------------------------------------------------------------

## 11. Cliente Python

O arquivo:

``` text
AppPubSubMQTT.py
```

implementa um cliente MQTT utilizando:

``` text
paho.mqtt.client
```

Na configuração atual, o cliente:

1.  conecta ao broker MQTT;
2.  assina o tópico do dispositivo;
3.  recebe mensagens publicadas pelo ESP32;
4.  permite informar pelo terminal uma requisição;
5.  publica a requisição no tópico do agente Bob.

O tópico de publicação é:

``` text
mqtt/jacamo/agent/bob
```

O tópico de assinatura é:

``` text
mqtt/jacamo/device/esp_01
```

A entrada esperada no terminal é:

``` text
target operation data
```

Por exemplo:

``` text
esp_01 setLed on
```

gera:

``` json
{
  "target": "esp_01",
  "operation": "setLed",
  "data": "on"
}
```

que é publicado em:

``` text
mqtt/jacamo/agent/bob
```

Para desligar:

``` text
esp_01 setLed off
```

------------------------------------------------------------------------

## 12. Docker Compose

Os serviços são definidos em:

``` text
docker-compose.yml
```

### JaCaMo

``` text
image: jomifred/jacamo:1.1
```

Portas:

``` text
8080
3272
```

### Node-RED

``` text
image: nodered/node-red:1.0.4
```

Porta:

``` text
1880
```

### Mosquitto

``` text
image: eclipse-mosquitto:2
```

Porta:

``` text
1883
```

Os três serviços utilizam a rede:

``` text
jacamo-nodered-net
```

------------------------------------------------------------------------

## 13. Execução

Na raiz do projeto, execute:

``` bash
docker compose up
```

Os principais serviços estarão disponíveis em:

``` text
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

``` bash
docker compose up -d
```

Para acompanhar os logs:

``` bash
docker compose logs -f
```

Para acompanhar somente o agente:

``` bash
docker compose logs -f mas
```

Para encerrar os containers:

``` bash
docker compose down
```

------------------------------------------------------------------------

## 14. Fluxos completos

### 14.1 Communication

``` text
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

------------------------------------------------------------------------

### 14.2 Reporting

``` text
ESP32
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

------------------------------------------------------------------------

### 14.3 Acting

``` text
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

------------------------------------------------------------------------

### 14.4 Monitoring

``` text
Thing / Test Client
   │
   │ MQTT
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
event(Thing, Event, Data)
   │
    ▼
Agent Bob
   │
    ▼
eventState(...)
```

------------------------------------------------------------------------

## 15. Correspondência entre modelo e implementação

A implementação atual pode ser resumida da seguinte forma:

  -------------------------------------------------------------------------------------------------
  Camada            Operação MQTTArt    Representação externa               Representação no agente
  ----------------- ------------------- ----------------------------------- -----------------------
  Communication     `communication()`   `request(Thing, Operation, Data)`   `request(...)`

  Reporting         `reporting()`       `report(Thing, Property, Value)`    `sensorState(...)` /
                                                                            `actuatorState(...)`

  Acting            `acting()`          `action(Thing, Action, Input)`      `action(...)`

  Monitoring        `monitoring()`      `event(Thing, Event, Data)`         `eventState(...)`
  -------------------------------------------------------------------------------------------------

A distinção é importante:

``` text
WoT Thing Description
    │
    ├── Property
    │     ├── lightSensor
    │     └── led
    │
    ├── Action
    │     └── setLed
    │
    └── Event
          └── overLight
```

enquanto o agente mantém representações internas:

``` text
Agent knowledge/state
    │
    ├── sensorState(Thing, Property, Value)
    ├── actuatorState(Thing, Property, Value)
    └── eventState(Thing, Event, Data)
```

Assim, a camada semântica da Thing e a camada de representação interna do agente permanecem conceitualmente separadas.

------------------------------------------------------------------------

## 16. Teste funcional da versão atual

A versão atual foi validada com: comunication, reporting, acting e monitoring.

Uma execução apresentou, entre outras, as seguintes saídas:

``` text
[bob] Bob is running

[bob] Message: target - target, operation: operation = data
[bob] Device: device - property: property = value
[bob] Event alert => device: device - event: event = data

[bob] Device: esp_01 - sensor: lightSensor = 5 (Measurement LOW)
[bob] Device: esp_01 - sensor: lightSensor = 12 (Measurement LOW)

[bob] Lighting request => target: esp_01, operation: setLed = on
[bob] Action: LED Turn on
[bob] Device: esp_01 - actuator: led = on

[bob] Device: esp_01 - sensor: lightSensor = 36 (Measurement NORMAL)
[bob] Device: esp_01 - sensor: lightSensor = 43 (Measurement NORMAL)
[bob] Device: esp_01 - sensor: lightSensor = 73 (Measurement HIGH)

[bob] Event alert => device: app_01 - event: overLight = 75

[bob] Lighting request => target: esp_01, operation: setLed = off
[bob] Action: LED Turn off
[bob] Device: esp_01 - actuator: led = off

[bob] Device: esp_01 - sensor: lightSensor = 45 (Measurement NORMAL)
```

Essa execução confirma o funcionamento integrado dos principais fluxos:

``` text
Reporting → percepção do sensor → classificação
```

``` text
Communication → deliberação do agente → Acting → ESP32
```

``` text
Acting → mudança do LED → Reporting do estado do LED
```

``` text
Monitoring → percepção de evento → registro pelo agente
```

------------------------------------------------------------------------

## 17. Estado atual da implementação

A versão atual constitui uma **baseline funcional de integração** entre agente e dispositivo, usando artefato, Node-RED e protocolo MQTT.

Os quatro fluxos estão definidos:

``` text
Communication
Reporting
Acting
Monitoring
```

e são implementados por:

``` text
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
ESP32 / outras fontes MQTT
```

A implementação também estabelece uma separação entre:

1.  **informações provenientes do ambiente**, representadas pelas mensagens: `request`, `report` e `event`;
2.  **ações produzidas pelo agente**, representadas por `action`;
3.  **representações internas do agente**, como `sensorState`, `actuatorState` e `eventState`.

Essa separação fornece uma base para etapas posteriores de descoberta de recursos, descrição semântica, seleção de informações, construção e integração de conhecimento.

------------------------------------------------------------------------

## 18. Próximos passos

A versão atual deve ser considerada a baseline de integração do projeto para posteriormente integrar descrições semânticas de Things, como **Thing Descriptions (TDs)**, mecanismos de descoberta e os mecanismos de construção e utilização do conhecimento do agente.

A intenção é manter a infraestrutura de comunicação e atuação estável enquanto os mecanismos de descoberta e conhecimento são acrescentados sobre a arquitetura existente.
