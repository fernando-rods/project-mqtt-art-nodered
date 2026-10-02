// Agent: bob

/* Initial beliefs and rules */

/* Initial goals */
!start.

/* Plans */
+!start: true
	<- .print("Bob is running").

/* Communication: message(Source, Content) */
+ns1::message(Source, Message): true
	<- .print("Message from: ", Source, " = ", Message);
	
	/* Acting: action(Device, Actuator, Command) */
	if (Message == "turn_on") {
		ns1::acting(action(esp_01, setLed, on));
        	.print("Action: LED Turn on");
        }
        if (Message == "turn_off") {
        	ns1::acting(action(esp_01, setLed, off));
        	.print("Action: LED Turn off");
        }.

/* Status: status(Device, Actuator, Mode) */
+ns1::status(Device, Actuator, Mode): true
	<- -+actuator(Device, Actuator, Mode);
	.print("Actuator: ", Actuator, " = ", Mode).

/* Sensing: sensor(Device, Variable, Value) */
+ns1::read(Device, Variable, Value): true
	<- -+sensor(Device, Variable, Value);
	.print("Sensor: ", Variable, " = ", Value).

{ include("$jacamoJar/templates/common-cartago.asl") }
// { include("$jacamoJar/templates/common-moise.asl") }
// { include("$jacamoJar/templates/org-obedient.asl") }
