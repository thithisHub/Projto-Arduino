var mqtt = require('mqtt')
var client  = mqtt.connect('mqtt://test.mosquitto.org')
 
client.on('connect', function () {
    console.log("conectou");
  client.subscribe('enviar_msgs_mqtt_arduino_regador', function (err) {
    if (!err) {
      client.publish('receber_msgs_mqtt_arduino_regador', 'Hello mqtt')
    }
  })
})
 
client.on('message', function (topic, message) {
  // message is Buffer
  console.log(message.toString())
})