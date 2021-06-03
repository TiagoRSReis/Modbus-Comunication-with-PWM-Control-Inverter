#include <ModbusMaster.h>               //Library for using ModbusMaster
#include <SoftwareSerial.h>             //Library for using additional serial port

#define MAX485_DE      5                // DE pin in MAX485
#define MAX485_RE_NEG  16               // RE pin in MAX485
#define Adress_to_read_speed  23        //Adress for values of Freq,Speed,current,torque (Hz,RPM,A,Nm)
#define Adress_to_read_erros  54        //Adress for values of Erros
#define Quantity_of_bits  4             //Quantity of values to read
#define Quantity_of_bits_for_error  4   //Quantity of erros to read

//Config for internet connection and Mqtt publisher
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
WiFiClient TCP_Client;                                         // Object type tipo TCP
PubSubClient client(TCP_Client);                               // Object type client MQTT
String Network_name ="put here your SSID";
String Network_pass ="put here your network password";
IPAddress Ip_Broker_Mqtt(192, 168, 1, 101);                    //IP of broker MQTT
int Port_Broker = 1883;                                        //MQTT Port 

SoftwareSerial mySerial(4, 0);                                 //software serial 4->Rx 0->Tx 

ModbusMaster node;                                             //object node for class ModbusMaster

int Delay_on_acquisition = 2000;                               //Time between read all over again 

//--------------------------------END OF CONFIGS,INCLUDES,BLABLABLA-----------------------------------------
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
{ //Iniciate serial ports
  Serial.begin(9600);             //Baud Rate of Serial as 115200 
  mySerial.begin(9600);           //Baud Rate of SoftwareSerial as 115200 
  
   // WiFi network
  Serial.println(); Serial.print("Connecting to ");Serial.println(Network_name);
  WiFi.mode(WIFI_STA);
  WiFi.begin(Network_name, Network_pass);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500); Serial.print(".");}
  Serial.println();
  Serial.print("The ESP is configurated with this ip: ");
  Serial.println(WiFi.localIP());

 // Config the GPIOS to ModBus 
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 1);



  // Config for ModBusMaster
  node.begin(1, mySerial);                       //Slave ID as 1
  node.preTransmission(preTransmission);         //Callback for configuring RS-485 Transreceiver correctly
  node.postTransmission(postTransmission);       //Callback for configuring RS-485 Receive correctly

 Serial.println("modbus configured");

    // MQTT
 client.setServer(Ip_Broker_Mqtt, Port_Broker);       
 client.setCallback(callback);
 client.connect("ESP8266");                           // O ESP need to register in broker. For any new equipment new name
}

void loop()
{
  static uint32_t i;
   uint8_t j, result;
  String data[Quantity_of_bits],Data_Errors[Quantity_of_bits_for_error];
  char data_mqtt[16];

  i++;
// set word 0 of TX buffer to least-significant word of counter (bits 15..0)
node.setTransmitBuffer(0, lowWord(i));

// set word 1 of TX buffer to most-significant word of counter (bits 31..16)
node.setTransmitBuffer(1, highWord(i));

 result = node.readHoldingRegisters(Adress_to_read_speed, Quantity_of_bits);
// do something with data if read is successful
if (result == node.ku8MBSuccess)
{
    for (j = 0; j < Quantity_of_bits; j++)
  {
    data[j] = node.getResponseBuffer(j);
    if(j==0){
    Serial.print("Freq Output = ");
    Serial.println(data[j]);
//    data[j].toCharArray(data_mqtt,16);
//    client.publish("Freq", data_mqtt);
    }
    if(j==1){
    Serial.print("Speed = ");
    Serial.println(data[j]);
//    data[j].toCharArray(data_mqtt,16);
//    client.publish("Speed", data_mqtt);
    }
    if(j==2){
    Serial.print("Current = ");
    Serial.println(data[j]);
//    data[j].toCharArray(data_mqtt,16);
//    client.publish("Current", data_mqtt);
    }
    if(j==3){
    Serial.print("Torque = ");
    Serial.println(data[j]);
//    data[j].toCharArray(data_mqtt,16);
//    client.publish("Torque", data_mqtt);
    }
}

delay(500);

result = node.readHoldingRegisters(Adress_to_read_erros, Quantity_of_bits_for_error);
// do something with data if read is successful
if (result == node.ku8MBSuccess)
{
    for (j = 0; j < Quantity_of_bits_for_error; j++)
  {
    data[j] = node.getResponseBuffer(j);
    if(j==0){
    Serial.print("Last Fault = ");
    Serial.println(data[j]);
    data[j].toCharArray(data_mqtt,16);
    client.publish("last_fault", data_mqtt);
    }
    if(j==1){
    Serial.print("1. Fault = ");
    Serial.println(data[j]);
    data[j].toCharArray(data_mqtt,16);
    client.publish("1.fault", data_mqtt);
    }
    if(j==2){
    Serial.print("2. Fault = ");
    Serial.println(data[j]);
    data[j].toCharArray(data_mqtt,16);
    client.publish("2.fault", data_mqtt);
    }
    if(j==3){
    Serial.print("3. Fault = ");
    Serial.println(data[j]);
    data[j].toCharArray(data_mqtt,16);
    client.publish("3.fault", data_mqtt);
    }
}
}
Serial.println("-------------------------------------------------------------------------------------------");
delay(Delay_on_acquisition);
}else{
  Serial.print("nothing done!");
}
}

