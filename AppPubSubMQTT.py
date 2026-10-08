'''
    App que publica via terminal e assina para receber mensagens do Broker MQTT
'''

import paho.mqtt.client as mqtt
import time
import json

# Configurações do broker MQTT
BROKER = "YOUR_MQTT_BROKER"  # Endereço IP
PORT = 1883
user = "YOUR_MQTT_UER"      # Username
pwd = "YOUR_MQTT_PASSWORD" # Passwaord
TOPIC_PUBLISH = "mqtt/jacamo/agent/bob" #"mqtt/jacamo/device/properties" #"mqtt/jacamo/device/events"
TOPIC_SUBSCRIBE = "mqtt/jacamo/device/esp_01"

# Callback: conexão estabelecida com o broker
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado ao broker MQTT!")
        client.subscribe(TOPIC_SUBSCRIBE)
        print(f"Assinado no tópico: {TOPIC_SUBSCRIBE}")
    else:
        print(f"Falha na conexão...")

# Callback: mensagem recebida
def on_message(client, userdata, msg):
    print(f"Mensagem recebida: tópico = {msg.topic}, payload = {msg.payload.decode()}")

# Criação do cliente MQTT
client = mqtt.Client()

# Usuário e senha
client.username_pw_set(user, pwd)

# Associação dos callbacks
client.on_connect = on_connect
client.on_message = on_message

# Conexão com o broker
client.connect(BROKER, PORT, keepalive=60)

# Loop em thread separada para receber mensagens
client.loop_start()

try:
    while (True):
        time.sleep(1)
        # Communication: Message JSON to Bob = esp_01 setLed on
        target, operation, data = input("Digite: source content: ").split() #request(Target, Operation, Data)
        message = {"target":target,"operation":operation, "data":data}
        json_message = json.dumps(message)
        info = client.publish(TOPIC_PUBLISH, json_message)
        info.wait_for_publish(timeout=1)
        if info.rc == mqtt.MQTT_ERR_SUCCESS:
            print(f"Publicado: {json_message} em {TOPIC_PUBLISH}")
        else:
            print(f"Falha na publicação MQTT (rc={info.rc})")
except KeyboardInterrupt:
    print("\nEncerrando cliente MQTT...")
    client.loop_stop()
    client.disconnect()
