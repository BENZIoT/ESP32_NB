#include <Arduino.h>
// #include <AIS_NB_BC95.h>
#include <ModbusMaster.h>
#include <ArduinoJson.h>

//----- WiFi -----
#include <WiFi.h>
#include <WiFiUdp.h>

#define WIFI_SSID  "DOF_Hotspot"
#define WIFI_PASS  "DOF@2023"

WiFiUDP UDP;

ModbusMaster modbus;

// const char *serverIP = "34.142.165.246";
const char *serverIP = "103.146.161.211";
int serverPort = 8089;

#define DEVICE_NAME "SF999"
#define LOCATION "SNI999"

String udpData = "Hello DOF Aquaculture";

// AIS_NB_BC95 AISnb;

const long interval = (1000 * 60) * 15;
unsigned long previousMillis = 0;

long cnt = 0;

float extractFloatFromHex(uint32_t hexValue);
void readDO();
void readSal();
void readPH();
void sendData();

int data_buffer[10];
float tempDO = 0;
float DOValue = 0;

float SALValue = 0;

float PHValue = 0;
float ECValue = 0;

// int data[2];

void setup()
{
  Serial.begin(9600);
  modbus.begin(1, Serial);

  //---- Connect NB-IoT ----
  //AISnb.debug = true;
  //AISnb.setupDevice(serverPort);
  //String ip1 = AISnb.getDeviceIP();
  //delay(1000);
  //pingRESP pingR = AISnb.pingIP(serverIP);

  //---- Connect WiFi ----
  Serial.println("Connecting WiFi Name: " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  previousMillis = millis();
}
void loop()
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    readDO();
    delay(100);
    readSal();
    delay(100);
    readPH();

    // tempDO = random(0, 50) * 1.00;
    // DOValue = random(0, 50) * 1.00;
    // SALValue = random(0, 50) * 1.00;
    // PHValue = random(0, 50) * 1.00;
    // ECValue = random(0, 50) * 1.00;

    delay(500);
    sendData();
    previousMillis = currentMillis;
  }
  // UDPReceive resp = AISnb.waitResponse();
}

float extractFloatFromHex(uint32_t hexValue)
{
  float result;
  memcpy(&result, &hexValue, sizeof(result));
  return result;
}

void readDO()
{
  modbus.begin(1, Serial);
  delay(10);
  uint8_t result = modbus.readHoldingRegisters(0x2000, 10);
  if (result == modbus.ku8MBSuccess)
  {
    for (uint8_t i = 0; i < 10; i++)
    {
      data_buffer[i] = modbus.getResponseBuffer(i);
    }
    data_buffer[0] = ((data_buffer[0] << 8) & 0xFF00) | ((data_buffer[0] >> 8) & 0x00FF);
    data_buffer[1] = ((data_buffer[1] << 8) & 0xFF00) | ((data_buffer[1] >> 8) & 0x00FF);
    tempDO = extractFloatFromHex(data_buffer[1] << 16 | data_buffer[0]);

    data_buffer[4] = ((data_buffer[4] << 8) & 0xFF00) | ((data_buffer[4] >> 8) & 0x00FF);
    data_buffer[5] = ((data_buffer[5] << 8) & 0xFF00) | ((data_buffer[5] >> 8) & 0x00FF);
    DOValue = extractFloatFromHex(data_buffer[5] << 16 | data_buffer[4]);
  }
  // else
  // {
  //   UDPSend udp = AISnb.sendUDPmsgStr(serverIP, serverPort, "DO Sensor: 0xE2");
  // }
}

void readSal()
{
  modbus.begin(2, Serial);
  delay(10);
  uint8_t result = modbus.readHoldingRegisters(0x2000, 10);
  if (result == modbus.ku8MBSuccess)
  {
    for (uint8_t i = 0; i < 10; i++)
    {
      data_buffer[i] = modbus.getResponseBuffer(i);
    }
    data_buffer[2] = ((data_buffer[2] << 8) & 0xFF00) | ((data_buffer[2] >> 8) & 0x00FF);
    data_buffer[3] = ((data_buffer[3] << 8) & 0xFF00) | ((data_buffer[3] >> 8) & 0x00FF);
    SALValue = extractFloatFromHex(data_buffer[3] << 16 | data_buffer[2]);
  }
  // else
  // {
  //   UDPSend udp = AISnb.sendUDPmsgStr(serverIP, serverPort, "SAL Sensor: 0xE2");
  // }
}

void readPH()
{
  modbus.begin(3, Serial);
  delay(10);
  uint8_t result = modbus.readHoldingRegisters(0x2000, 10);
  if (result == modbus.ku8MBSuccess)
  {
    for (uint8_t i = 0; i < 10; i++)
    {
      data_buffer[i] = modbus.getResponseBuffer(i);
    }
    data_buffer[0] = ((data_buffer[0] << 8) & 0xFF00) | ((data_buffer[0] >> 8) & 0x00FF);
    data_buffer[1] = ((data_buffer[1] << 8) & 0xFF00) | ((data_buffer[1] >> 8) & 0x00FF);
    ECValue = extractFloatFromHex(data_buffer[1] << 16 | data_buffer[0]);

    data_buffer[2] = ((data_buffer[2] << 8) & 0xFF00) | ((data_buffer[2] >> 8) & 0x00FF);
    data_buffer[3] = ((data_buffer[3] << 8) & 0xFF00) | ((data_buffer[3] >> 8) & 0x00FF);
    PHValue = extractFloatFromHex(data_buffer[3] << 16 | data_buffer[2]);
  }
  // else
  // {
  //   UDPSend udp = AISnb.sendUDPmsgStr(serverIP, serverPort, "PH Sensor: 0xE2");
  // }
}

double round2(double value)
{
  return (int)(value * 100 + 0.50) / 100.00;
}

void sendData()
{
  // UDPSend udp = AISnb.sendUDPmsgStr(serverIP, serverPort, "{ \"water_temp\": " + String(tempDO) + ", \" DO_value\": " + String(DOValue) + ", \"Salinity\": " + String(SALValue) + ", \"Salinity\": " + String(ECValue) + ", \"PH_value\": " + String(PHValue) + ", \"Device_Name\": " + "\"" + DEVICE_NAME + "\"" + " }");
  DynamicJsonDocument jsonDoc(512);
  jsonDoc["measurement"] = "Water Quality";
  JsonObject tags = jsonDoc.createNestedObject("tags");
  tags["location"] = LOCATION;
  tags["deviceName"] = DEVICE_NAME;
  JsonObject fields = jsonDoc.createNestedObject("fields");
  fields["waterTemp"] = round2(tempDO);
  fields["DOvalue"] = round2(DOValue);
  fields["Salinity"] = round2(SALValue);
  fields["ECvalue"] = round2(ECValue);
  fields["PHvalue"] = round2(PHValue);
  jsonDoc["time"] = "2023-01-01T12:34:56Z";
  String payload = "";
  serializeJson(jsonDoc, payload);
  serializeJsonPretty(jsonDoc, Serial);
  
  //---- Send Data ----
  UDP.beginPacket(serverIP, serverPort);
  UDP.print(payload);
  UDP.endPacket();
}
