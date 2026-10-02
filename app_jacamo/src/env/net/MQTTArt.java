/* Artifact: mqtt */

package net;

import cartago.*;
import jason.asSyntax.*;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

public class MQTTArt extends jacamo.rest.util.DummyArt {

    public void init(String endpoint) {       
        defineObsProperty("message", "unknown", "unknown");     
        defineObsProperty("read", "unknown", "unknown", 0.0);
        defineObsProperty("status", "unknown", "unknown", "off");
        super.register(endpoint);
    }

    @OPERATION //Communication: Recebe uma mensagem destinada diretamente ao agente
    public void communication(String json) {       
	JsonObject obj = new JsonParser().parse(json).getAsJsonObject();
	String source = obj.get("source").getAsString();
	String content = obj.get("content").getAsString();
	getObsProperty("message").updateValues(source, content);
    }
    
    @OPERATION // Sensing: Recebe uma leitura do dispositivo e disponibiliza como percepção ao agente
    public void sensing(String json) {
	JsonObject obj = new JsonParser().parse(json).getAsJsonObject();
	String device = obj.get("device").getAsString();
	String variable = obj.get("variable").getAsString();
	double value = obj.get("value").getAsDouble();
	getObsProperty("read").updateValues(device, variable, value);
    }
    
    @OPERATION // Status: Recebe um status do atuador e disponibiliza como percepção ao agente
    public void monitoring(String json) {
	JsonObject obj = new JsonParser().parse(json).getAsJsonObject();
	String device = obj.get("device").getAsString();
	String actuator = obj.get("actuator").getAsString();
	String mode = obj.get("mode").getAsString();
	getObsProperty("status").updateValues(device, actuator, mode);
    }

    @OPERATION // Acting: Recebe uma ação do agente e delega sua execução ao mecanismo do DummyArt
    public void acting(String action) {
        try {
            OpFeedbackParam<Term> result = new OpFeedbackParam<Term>();
            act(action, result);
        } catch (Exception e) {
            e.printStackTrace();
            failed("[MQTTArt] Error operation acting: " + e.getMessage());
        }
    }
}
