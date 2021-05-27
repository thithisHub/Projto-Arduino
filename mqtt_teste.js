var mqtt = require('mqtt')// modulo npm para uso do protocolo mqqt
var client  = mqtt.connect('mqtt://test.mosquitto.org')// estabelece comunicaçao com broker especificado
 
client.on('connect', function () {//
    console.log("conectou");
  client.subscribe('enviar_msgs_mqtt_arduino_regador', function (err) {// ao se conectar subscreve o canal de comunicaçao especificado   
  })
})
 
client.on('message', function (topic, message) {
  console.log(message.toString())// recebe e printa informaçoes enviadas pelo arduino
})