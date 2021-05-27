
#include <Firmata.h>
#include <WiFiEsp.h>
#include <PubSubClient.h>
#include "SoftwareSerial.h"
 
SoftwareSerial Serial1(2, 3);//estabelecendo comunicaçao seria com o ESP

bool leituraSensor;
bool leituraAnterior;

//Wifi e mqtt configs
char* ssid = "Casa Leo"; //aqui vai o nome do wifi
char* pass = "07111996"; //aqui vai a senha do wifi
char* endereco_broker = "test.mosquitto.org"; //broker mqtt que usamos no projeto, tentamos com a do eclipse mas não funciona
int porta_broker = 1883; // porta publica do broker (não autenticada nem com criptografia)
WiFiEspClient espClient; //cria cliente de WiFi (compativel com o nosso modelo ESP8266), sera usada na lib do mqtt (para enviar os dados no wifi)
PubSubClient MQTT(espClient); //cria o objeto do mqtt




void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  
  //Sensor de umidade
  pinMode(8, INPUT);

  //Atuador relé
  pinMode(12, OUTPUT);

  WiFi.init(&Serial1); //define o serial criado como sendo o canal para o serial do wifi
  WiFi.config(IPAddress(192,168,15,116)); //colocar aqui um ip valido para o wifi
  WiFi.begin(ssid, pass); // conecta na rede wifi
  configuraMqtt();
  
  //LEDs
  pinMode(5, OUTPUT);  //vermelho
  pinMode(6, OUTPUT);  //amarelo
  pinMode(7, OUTPUT);  //verde

}

//metodo para iniciar o mqtt, conectar ao broker e setar os canais de leitura e publicação
void configuraMqtt() 
{
    MQTT.setServer(endereco_broker, porta_broker);   //informa qual broker e porta deve ser conectado
    delay(5000);//espera para testar conexão, as vezes o wifi demora um pouco e da falha na primeira, então achamos melhor esperar (parou de falhar)
    while (!MQTT.connected()) 
    {
      Serial.println((String)"conectado ao broker: " + (String)endereco_broker);
      if (MQTT.connect("thiagofernandesmqtt")) //cria uma sessão unica para uso do mqtt
      {
          Serial.println("conexao 2 com sucesso");
          MQTT.subscribe("receber_msgs_mqtt_arduino_regador"); //subscreve para ler esse canal (ouvir se chegar mensagem)
      } 
      else
      {
        Serial.println("Nao foi possivel conectar");//nao conectou/criou sessão vai tentar em 5 segundos novamente
        delay(5000);
      }
    } 
}


void publicaMensagemBroker(String mensagem)
{ 
  int tamanho_string = mensagem.length() + 1; //tamanho tem mais um para identificar o final da string
  char texto_char[tamanho_string];
  mensagem.toCharArray(texto_char, tamanho_string);
  MQTT.publish("enviar_msgs_mqtt_arduino_regador", texto_char);
  Serial.println("mensagem " + mensagem + " enviada");
  delay(500);//para dar um tempo para o broker receber
}
 


void loop() {

  MQTT.loop();//manda o sinal de keep-alive para o broker nao encerrar a conexão

  leituraSensor = digitalRead(8);// lendo sinal emitido pelo sensor de umidade

  if (leituraSensor == HIGH) {// se o sinal for high significa que a terra esta seca
     //No estado seco
     digitalWrite(5, HIGH);  //acende vermelho
     digitalWrite(7, LOW);   // apaga verde
     publicaMensagemBroker("Leitura: Terra Seca"); //informa ao broker que a terra foi lida como seca
  } else {// se o sinal for low significa que a terra esta umida
     //No estado úmido
     digitalWrite(5, LOW);   //apaga vermelho
     digitalWrite(7, HIGH);  // acende verde
     publicaMensagemBroker("Leitura: Terra molhada"); //informa ao broker que a terra foi lida como mollhada
  }

  //Ao entrar no estado seco  
  if (leituraSensor == HIGH && leituraAnterior == LOW) {// se o estado atual for seco e o anterior for molhado , entra no modo de irrigaçao
     delay(5000);
     digitalWrite(5, LOW);   //apaga vermelho
     digitalWrite(6, HIGH);  //acende amarelo

     while (digitalRead(8)) {// em quanto o sensor de umidade enviar o sinal de seco , irrigue
        digitalWrite(12, HIGH);//ativa a bomba de agua
        delay(500);// mantem ela ligada durante o periodo estipulado
        digitalWrite(12, LOW);//desliga a bomba
        publicaMensagemBroker("Ação: Irrigando");//informa ao broker que a ação de irrigar foi tomada
        delay(10000);// aguarda 10 sec para prox leitura de sensor          
     }
     digitalWrite(6, LOW);  //saindo do laço de irrigaçao , apaga o led amarelo
  }
  
}
