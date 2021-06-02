#include <ModbusMaster.h>               //Library for using ModbusMaster
#include <PubSubClient.h>               //Library for MQTT 


int Adress_to_read=23;
int Quant_to_read=5;
int Adress_to_read_errors=54;
int Quant_to_read_errors=5;
int Slave_inicial=10;
int Slave_final=0;
int slave=Slave_inicial;
String marca="Siemens"; //Mitsubishi  , Siemens
const char* topico_mqtt="slave";


# define delay_measures 400
#define MAX485_DE      19
#define MAX485_RE_NEG  18

ModbusMaster node;                    //object node for class ModbusMaster

// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char* ssid     = "Riablades";
const char* password = "W1relessP@ssw0rd";

// Set your Static IP address
IPAddress local_IP(172, 31, 4, 218);
// Set your Gateway IP address
IPAddress gateway(172, 31, 0, 254);
IPAddress subnet(255, 255, 0, 0);

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.8144";
const char* mqtt_server = "172.31.4.12";
WiFiClient espClient; 
PubSubClient client(espClient);


void preTransmission()            //Function for setting stste of Pins DE & RE of RS-485
{
  digitalWrite(MAX485_RE_NEG, 1);             
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}






void setup()
{
  
  
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  
  
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);

  Serial.begin(9600,SERIAL_8N2);             //Baud Rate as 115200
  Serial2.begin(9600,SERIAL_8N2);
  node.begin(slave, Serial2);            //Slave ID as 1
  node.preTransmission(preTransmission);         //Callback for configuring RS-485 Transreceiver correctly
  node.postTransmission(postTransmission);

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to mqtt server
  client.setServer(mqtt_server, 1883);
  client.connect("ESP32");
  if (client.connect("ESP32")) {
      Serial.println("connected");

}
}




void loop()
{
if (!client.connect("ESP32")) {
  client.connect("ESP32");
  delay(500);
}

  
  uint8_t ResultadoLectura;
  static uint32_t i;
  String Data[Quant_to_read],Data_Errors[Quant_to_read_errors],topic;
  char data_mqtt[16],topic_mqttchr[18];
  i++;
  topic="Slave " + String(slave) + " Leituras";

node.begin(slave, Serial2);            //Slave ID as 1

  
// set word 0 of TX buffer to least-significant word of counter (bits 15..0)
node.setTransmitBuffer(0, lowWord(i));

// set word 1 of TX buffer to most-significant word of counter (bits 31..16)
node.setTransmitBuffer(1, highWord(i));
  
  ResultadoLectura = node.readHoldingRegisters(Adress_to_read,Quant_to_read);        //Writes value to 0x40000 holding register
  Serial.println("Leituras:------------>");
  //if (ResultadoLectura == node.ku8MBSuccess || ResultadoLectura!= 226 )
  if (ResultadoLectura == node.ku8MBSuccess  )
  {
    for (int j = 0; j < Quant_to_read; j++)
    {
      Data[j] = node.getResponseBuffer(j);
      Serial.println(node.getResponseBuffer(j));
      Data[j].toCharArray(data_mqtt,16);
      //client.publish(topico_mqtt+slave, data_mqtt);
      topic.toCharArray(topic_mqttchr,18);
      client.publish(topic_mqttchr, data_mqtt);
      //Serial.println("Publish on: "+String(topico_mqtt+slave));
      Serial.println("Publish on: "+ String(topic_mqttchr));

    }
  }
  delay(500);
  Serial.println("Errors:-------------->");
  topic="Slave " + String(slave) + " Erros";
   ResultadoLectura = node.readHoldingRegisters(Adress_to_read_errors,Quant_to_read_errors);        //Writes value to 0x40000 holding register
  //Serial.println(ResultadoLectura);
  //if (ResultadoLectura == node.ku8MBSuccess || ResultadoLectura!= 226 )
  if (ResultadoLectura == node.ku8MBSuccess  )
  {
    for (int j = 0; j < Quant_to_read_errors; j++)
    {
      Data_Errors[j] = node.getResponseBuffer(j);
      Serial.println(node.getResponseBuffer(j));
      Data_Errors[j].toCharArray(data_mqtt,16);
      //client.publish(topico_mqtt+slave, data_mqtt);
       topic.toCharArray(topic_mqttchr,16);
      client.publish(topic_mqttchr, data_mqtt);
      //Serial.println("Publish on: "+String(topico_mqtt+slave));
      Serial.println("Publish on: "+ String(topic_mqttchr));

    }
  }
  Serial.print("Slave nº ");
  Serial.println(slave);
  Serial.println("------------------------------------------------------------------");

// Serial2.flush();
// delay(20);


slave = slave-1;
if (slave==Slave_final){
  slave=Slave_inicial;
}
 
if(slave==10 || slave == 9 || slave == 8 || slave == 7 || slave == 6 || slave == 5 || slave == 4 || slave == 3)
{
  Adress_to_read=23;
  Quant_to_read=4;
  Adress_to_read_errors=54;
  Quant_to_read_errors=5;
  marca="Siemens";
}else{
  Adress_to_read=201;
  Quant_to_read=3;
  Adress_to_read_errors=501;
  Quant_to_read_errors=5;
  marca="Mitsubishi";
}

 delay(delay_measures);
}
