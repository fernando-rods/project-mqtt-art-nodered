// Agent: bob

/* Initial beliefs and rules */

/* Initial goals */
!start.

/* Plans */
+!start: true
	<- .print("Bob is running").

/* Communication: request(Target, Operation, Data) */
+ns1::request(Target, Operation, Data): Operation == setLed
	<- .print("Lighting request => target: ", Target, ", operation: ", Operation, " = ", Data);
	/* Acting: action(Thing, Action, Input) */
	ns1::acting(action(Target, Operation, Data));
        .print("Action: LED Turn ", Data).

+ns1::request(Target, Operation, Data)
	<- .print("Message: target - ", Target, ", operation: ", Operation, " = ", Data).

/* Reporting: report(Thing, Property, Value) */
+ns1::report(Thing, Property, Value): Property == led
	<- -+actuatorState(Thing, Property, Value);
	.print("Device: ", Thing," - actuator: ", Property, " = ", Value).

+ns1::report(Thing, Property, Value): Property == lightSensor
	<- -+sensorState(Thing, Property, Value);
	!classifyLight(Thing, Value).

+ns1::report(Thing, Property, Value)
	<- .print("Device: ", Thing," - property: ", Property, " = ", Value).

+!classifyLight(Thing, Value): Value >= 0 & Value < 30
	<- .print("Device: ", Thing, " - sensor: lightSensor = ", Value, " (Measurement LOW)").

+!classifyLight(Thing, Value): Value >= 30 & Value < 70
	<- .print("Device: ", Thing, " - sensor: lightSensor = ", Value, " (Measurement NORMAL)").

+!classifyLight(Thing, Value): Value >= 70 & Value <= 100
	<- .print("Device: ", Thing, " - sensor: lightSensor = ", Value, " (Measurement HIGH)").

+!classifyLight(Thing, Value): Value < 0 | Value > 100
	<- .print("Device: ", Thing, " - sensor: lightSensor = ", Value, " (Measurement OUTSIDE RANGE)").

/* Monitoring: event(Thing, Event, Data) */
+ns1::event(Thing, Event, Data): true
	<- -+eventState(Thing, Event, Data);
	.print("Event alert => device: ", Thing," - event: ", Event, " = ", Data).

{ include("$jacamoJar/templates/common-cartago.asl") }
// { include("$jacamoJar/templates/common-moise.asl") }
// { include("$jacamoJar/templates/org-obedient.asl") }
