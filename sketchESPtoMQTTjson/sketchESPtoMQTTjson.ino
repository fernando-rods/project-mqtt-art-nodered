/*
 * PROJETO: comunicação MQTT entre ESP32 e Broker Mosquitto
*/

#include <WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

/* Definições de variáveis*/
#define LED 2 // LED integrado ao módulo ESP32
#define INTERVAL 10000 // Intervalo de tempo para publicação de mensagens no Broker

/* Tópicos MQTT */
const char* mqttClientID = "esp_01"; // Identificação da sessão entre o Cliente MQTT e o Broker
const char* mqttTopicPubSensor = "mqtt/jacamo/device/properties";  // Tópico de publicação do sensor
const char* mqttTopicPubLed = "mqtt/jacamo/device/properties";  // Tópico de publicação do led
const char* mqttTopicSubLed = "mqtt/jacamo/device/esp_01"; // Tópico de assinatura do ESP
const char* mqttTopicPubEvent = "mqtt/jacamo/device/events"; // Tópico de publicação de eventos

// Informações da Rede WiFi
const char* ssid = "YOUR_WIFI_SSID";          // Nome da rede WI-FI que deseja se conectar
const char* password = "YOUR_WIFI_PASSWORD"; // Senha da rede WI-FI que deseja se conectar

// Informações da Broker MQTT
const char* mqttServer = "YOUR_MQTT_BROKER";       //server
const char* mqttUser = "YOUR_MQTT_USER";          //user
const char* mqttPassword = "YOUR_MQTT_PASSWORD"; //password
const int mqttPort = 1883;                      //port

// Objetos globais
WiFiClient espClient;             // Cria o objeto com nome: "espClient"
PubSubClient client(espClient);  // Instancia o Cliente MQTT com nome: "client", passando o objeto "espClient"

// Variáveis globais
unsigned long lastPubMQTT = 0; // tempo do último envio de mensagem MQTT
double sensorValue = 0;    // variável de leitura do sensor

/* Prototypes */
void connectWiFi();
void connectBroker();
void callback(char* topic, byte* payload, unsigned int length);
void checkConnections();
void checkSensors();
void pubMQTT();

/* Implementações das funções*/
// Função ConnectWiFi: connecta ou reconecta à rede WiFi
void connectWiFi(){
    // Se já está conectado à rede WiFi, nada é feito. Caso contrário, são efetuadas tentativas de conexão.
    if (WiFi.status() == WL_CONNECTED){
      return;
    } else {
      Serial.print("Conectando a Rede WiFi");
      WiFi.begin(ssid, password); // Conecta na rede WiFi
      while (WiFi.status() != WL_CONNECTED){
        delay(100);
        Serial.print(".");
      }
      Serial.println();
      Serial.println("- WIFI CONECTADO");
      Serial.print("Rede: ");
      Serial.println(ssid);
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    }
}

// Função ConnectBroker: conecta ou reconecta ao Broker MQTT e assina os tópicos MQTT
void connectBroker(){
  //Conexao ao broker MQTT
  client.setServer(mqttServer, mqttPort); // Informa o servidor e porta para conexão ao Broker
  client.setCallback(callback); // Atribui a função de callback

  // Se já está conectado ao Broker, nada é feito. Caso contrário, são efetuadas tentativas de conexão e em caso de falha, informa o estado da mesma.
  if (client.connected()) {
    return;
  } else {
    while (!client.connected()){
      Serial.println("Conectando ao Broker MQTT...");
      if (client.connect(mqttClientID, mqttUser, mqttPassword )){
        Serial.println("- BROKER CONECTADO");
        client.subscribe(mqttTopicSubLed, 1); //nivel de qualidade: QoS 1
      } else {
        Serial.print("Falha na conexão com o Broker - Estado: ");
        Serial.println(client.state());
        delay(2000);
      }
    }
  }
}

// Função CheckConnections: verifica o estado das conexões WiFi e Broker MQTT. Em caso de desconexão (qualquer uma das duas), a conexão é refeita.
void checkConnections(){
  connectWiFi(); //se não há conexão com o WiFI, a conexão é refeita

  if (!client.connected()) {
    connectBroker(); //se não há conexão com o Broker, a conexão é refeita
  }

}

// Função Callback: função de callback é chamada toda vez que uma informação de um dos tópicos assinados chega)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("SUBSCRIBER - tópico: ");
  Serial.println(topic);
  Serial.print("Mensagem: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("--------------------------------------------------");

  // Verifica se a mensagem veio do tópico de atuação
  if (strcmp(topic, mqttTopicSubLed) == 0) {
    // Cria documento JSON
    JsonDocument doc;

    // Faz o parsing diretamente do payload MQTT
    DeserializationError error = deserializeJson(doc, payload, length);

    // Verifica erro no JSON
    if (error) {
      Serial.print("Erro ao interpretar JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Extrai os campos
    const char* thing = doc["thing"];
    const char* action = doc["action"];
    const char* input = doc["input"];

    // Verifica se os campos existem
    if (thing == nullptr || action == nullptr || input == nullptr) {
      Serial.println("JSON invalido: campos obrigatorios ausentes.");
      return;
    }

    // Verifica se a ação é destinada ao LED
    if (strcmp(thing, mqttClientID) == 0 && strcmp(action, "setLed") == 0) {
      String statusMode = "";

      if (strcmp(input, "on") == 0) { // Liga o LED
        digitalWrite(LED, HIGH);
        statusMode = "on";
      } else if (strcmp(input, "off") == 0) { // Desliga o LED
        digitalWrite(LED, LOW);
        statusMode = "off";
      } else { // Comando desconhecido
        Serial.print("Modo de atuação desconhecido: ");
        Serial.println(input);
        return;
      }

      // Leitura do estado atual do LED
      int ledValue = digitalRead(LED);

      // Cria JSON de status
      JsonDocument statusDoc;
      statusDoc["thing"] = mqttClientID;
      statusDoc["property"] = "led";
      statusDoc["value"] = statusMode;

      // Serializa JSON
      String msgJSON;
      serializeJson(statusDoc, msgJSON);

      // Publica status
      if (!client.publish(mqttTopicPubLed, msgJSON.c_str())) {
        Serial.println("Falha ao publicar mensagem do led.");
      }

      // Debug
      Serial.print("LED value: ");
      Serial.println(ledValue);
      Serial.print("PUBLISHER - tópico: ");
      Serial.println(mqttTopicPubLed);
      Serial.print("Payload enviado: ");
      Serial.println(msgJSON);
      Serial.println("--------------------------------------------------");
    } else {
      Serial.println("Ação ignorada: thing ou action não corresponde.");
    }
  }
}

void checkSensors(){
  // Simulação da variável de leitura de um sensor (variação de 0 a 100)
  sensorValue = random(0, 101);

  if (isnan(sensorValue)){
    Serial.println("Falha na leitura do sensor...");
  } else {
    Serial.print("Leitura do sensor: ");
    Serial.print(sensorValue);
    Serial.println(" %");

    if(sensorValue < 15 || sensorValue > 85){
      // Cria JSON de status
      JsonDocument statusDoc;
      if (sensorValue < 15){
        statusDoc["thing"] = mqttClientID;
        statusDoc["event"] = "underLight";
        statusDoc["data"] = sensorValue;
      }
      if (sensorValue > 85){
        statusDoc["thing"] = mqttClientID;
        statusDoc["event"] = "overLight";
        statusDoc["data"] = sensorValue;
      }
      // Serializa JSON
      String msgJSON;
      serializeJson(statusDoc, msgJSON);

      // Publica status
      if (!client.publish(mqttTopicPubEvent, msgJSON.c_str())) {
        Serial.println("Falha ao publicar mensagem do alerta.");
      }

      // Debug
      Serial.print("Alert - value: ");
      Serial.println(sensorValue);
      Serial.print("PUBLISHER - tópico: ");
      Serial.println(mqttTopicPubEvent);
      Serial.print("Payload enviado: ");
      Serial.println(msgJSON);
      Serial.println("--------------------------------------------------");
    } else {
      Serial.println("Nenhum alerta de luminosidade.");
    }
  }
}

void pubMQTT(){
  JsonDocument doc;  // documento JSON

  // Monta a estrutura do JSON
  doc["thing"] = mqttClientID;
  doc["property"] = "lightSensor";
  doc["value"] = sensorValue;

  // Cria uma string para armazenar o JSON serializado
  String msgJSON;
  serializeJson(doc, msgJSON);

  // Publica a string contendo o JSON no broker
  if (!client.publish(mqttTopicPubSensor, msgJSON.c_str())) {
    Serial.println("Falha ao publicar mensagem do sensor.");
  }

  Serial.print("PUBLISHER - tópico: ");
  Serial.println(mqttTopicPubSensor);
  Serial.print("Payload enviado: ");
  Serial.println(msgJSON);
  Serial.println("--------------------------------------------------");
}

/* Função de setup */
void setup(){
    Serial.begin(115200);
    pinMode(LED, OUTPUT);
    digitalWrite(LED,LOW);

    randomSeed(esp_random());

    // Inicializa a conexão WiFi
    connectWiFi();

    // Inicializa a conexão com o Broker MQTT
    connectBroker();
}

/* Loop principal */
void loop(){
    // Verifica as conexões WiFi e Broker MQTT
    checkConnections();

    // Faz a leitura dos sensores e publica no Broker MQTT a cada intervalo de tempo definido no código
    if ((millis() - lastPubMQTT) > INTERVAL) {
      checkSensors();
      pubMQTT();
      lastPubMQTT = millis();
    }

    // keep-alive da comunicação com o Broker MQTT
    client.loop();
}