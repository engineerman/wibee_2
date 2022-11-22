#include <Arduino.h>
#include "ESPAsyncWebServer.h"
#include <AsyncElegantOTA.h>

#include "SD_MMC.h"
#include "Version.h"
#include "settings.h"

#include "html.h"
#include "sdkconfig.h"
#include <WiFiMulti.h>

#include "ESPmDNS.h"

#include "ESP32Time.h"
#include "ctime"

ESP32Time rtc;

#define DEBUG(x) Serial.println(x);
#define DEFAULT_WIFIAP_PASSWORD "wibee444"
// OTA Update

unsigned long mTimer;
#define TIMEOUT 4000

WiFiMulti wifiMulti;
bool mIsWifiOnline = false;
bool mIsFirmwareUpdating = false;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
// flag to use from web update to reboot the ESP
bool shouldReboot = false;

char c;
char rx1[1025 + 128];
int rxInd = 0;

char rx2[1025 + 128];
int rx2Ind = 0;

unsigned long lastRxTime;
unsigned long lastRx2Time;

bool enableLog = false;
bool enableTimeStamp = false;

systemConfiguration *sysConfig = NULL;
File logFile;
char *mLogMemory = NULL;
uint32_t mLogMemoryIndex = 0;
uint32_t mSDWriteChunkSize = SD_CARD_WRITE_CHUNK_SIZE;

#define DEBUG(x) Serial.println(x);

// #define DATE_TIME_FORMAT "%Y.%m.%d-%H:%M:%S"
#define DATE_TIME_FORMAT "%Y-%m-%dT%H:%M:%S"

static String datetime_now(String format = DATE_TIME_FORMAT)
{
  // TICK();
  String ss = rtc.getTime(format);
  // DEBUG(TOCK());
  return ss;
}

static void set_datetime(String dateTimeStr, String format = DATE_TIME_FORMAT)
{
  struct tm tm_datetime;

  strptime(dateTimeStr.c_str(), format.c_str(), &tm_datetime);

  rtc.setTimeStruct(tm_datetime);

  DEBUG(datetime_now());
}

void print_mem()
{
  char cc[128];
  sprintf(cc, "Total heap: %d", ESP.getHeapSize());

  DEBUG(String(cc));

  sprintf(cc, "Free heap: %d", ESP.getFreeHeap());
  DEBUG(String(cc));

  sprintf(cc, "Free Heap2: %d", esp_get_free_heap_size());
  DEBUG(String(cc));

  sprintf(cc, "Min Free Heap: %d", esp_get_minimum_free_heap_size());
  DEBUG(String(cc));

  // sprintf(cc, "Largest Free block: %d", heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
  // DEBUG(String(cc));

  // sprintf(cc, "Total PSRAM: %d", ESP.getPsramSize());
  // DEBUG(String(cc));

  // sprintf(cc, "Free PSRAM: %d", ESP.getFreePsram());
  // DEBUG(String(cc));
}

bool AdvertiseServices(const char *serviceName = "wibee")
{
  MDNS.end();

  if (!MDNS.begin(serviceName))
  {
    DEBUG(F("Error setting up MDNS responder"));
    delay(1000);
    return false;
  }
  else
  {
    MDNS.disableArduino();
    DEBUG(F("mDNS responder started"));
    DEBUG(serviceName);

    // Add service to MDNS-SD
    MDNS.addService("wibee", "tcp", 80);

    return true;
  }
}

void onUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (index == 0)
  {
    Serial.printf("WiBee FIRMWARE DOWNLOADING: %s\n", filename.c_str());

    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    { // start with max available size
      DEBUG("Update Error");
      Update.printError(Serial);
    }
    else
    {
      mIsFirmwareUpdating = true;
    }
  }

  /* flashing firmware to ESP*/
  if (Update.write(data, len) != len)
  {
    Update.printError(Serial);
    mIsFirmwareUpdating = false;
  }

  if (final)
  {
    if (Update.end(true))
    { // true to set the size to the current progress
      Serial.printf("UPDATE SUCESS: %u\n", index + len);
      request->send(200, "text/html", "Update done");
      vTaskDelay(1000);
      ESP.restart();
    }
    else
    {
      Update.printError(Serial);
    }

    mIsFirmwareUpdating = false;
  }
}

void process_websocket_messages(const uint8_t *buffer, size_t size, int cid)
{
  char *cc = (char *)malloc(sizeof(char) * (size + 1));
  memcpy(cc, buffer, size);

  cc[size] = 0;

  String cmd = String(cc);

  if (cmd.startsWith("ping"))
  {
    ws.binary(cid, "pong");
  }
  else if (cmd.startsWith("status"))
  {
    String sts = "Version " + String(VERSION) + "\nClients " + String(ws.count());

    if (enableLog)
    {
      sts += "\nLog enabled";
    }
    else
    {
      sts += "\nLog disabled";
    }

    if (enableTimeStamp)
    {
      sts += "\nTimestamp enabled\n";
    }
    else
    {
      sts += "\nTimestamp disabled\n";
    }

    sts += datetime_now();

    sts += "\n";

    ws.binary(cid, sts);
  }
  else if (cmd.startsWith("netstat"))
  {
    String sts = " clients " + String(ws.count()) + "\n";

    DEBUG("Sending Stat");

    ws.binary(cid, sts);
  }
  else if (cmd.startsWith("tx"))
  {
    String tx = cmd.substring(3, cmd.length() - 2);
    Serial.println(tx);
  }
  else if (cmd.startsWith("serial"))
  {
    String args = cmd.substring(7, cmd.length() - 2);

    DEBUG(args + " " + String(cmd.length()) + "  " + args.length());

    int ind = args.indexOf(",");
    int targetPort = args.substring(0, ind).toInt();
    args = args.substring(ind + 1);
    int baudrate = args.toInt();

    if (targetPort >= 0 && targetPort < 2 && baudrate >= 9600)
    {
      if (targetPort == 0)
      {
        Serial.end();
        Serial.begin(baudrate);
        DEBUG("Setting Serial1 " + String(baudrate));
      }
      else if (targetPort == 1)
      {
        Serial2.end();
        Serial2.begin(baudrate);
        DEBUG("Setting Serial2 " + String(baudrate));
      }
      delay(1000);
    }
    else
    {
      DEBUG("Invalid parameters " + String(targetPort) + "-" + String(baudrate));
    }
  }
  else if (cmd.startsWith("mem"))
  {
    DEBUG(String(mLogMemoryIndex));
  }
  else if (cmd.startsWith("enlog"))
  {
    int en = cmd.substring(6, cmd.length() - 1).toInt();
    enableLog = en > 0;

    if (enableLog)
    {
      DEBUG("Logging enabled");
    }
    else
    {
      DEBUG("Logging disabled");
    }
  }
  else if (cmd.startsWith("enTS"))
  {
    int en = cmd.substring(5, cmd.length() - 1).toInt();
    enableTimeStamp = en > 0;

    String rsp;
    if (enableTimeStamp)
    {
      DEBUG("TimeStamping enabled");
      rsp = "TimeStamping enabled\n";
    }
    else
    {
      DEBUG("TimeStamping disabled");
      rsp = "TimeStamping disabled\n";
    }

    ws.binary(cid, rsp);
  }
  else if (cmd.startsWith("dellog"))
  {
    SD_MMC.remove(LOG_FILE_PATH);
    DEBUG("Log file deleted");
  }
  else if (cmd.startsWith("chunk_size"))
  {
    int chunkSize = cmd.substring(11, cmd.length() - 1).toInt();

    if (chunkSize > SD_CARD_WRITE_CHUNK_SIZE && chunkSize < 65536 * 16)
    {
      mSDWriteChunkSize = chunkSize;
      DEBUG("Chunk size set to " + String(chunkSize));
    }
    else
    {
      DEBUG("Invalid Chunk size " + String(chunkSize));
    }
  }
  else if (cmd.startsWith("clrw"))
  {
    int ind = cmd.substring(5, cmd.length() - 1).toInt();

    if (ind < 0 && ind < 10)
    {
      sysConfig->clearWifiCredentials(ind);

      sysConfig->addWifiCredentials(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);

      sysConfig->saveToFile();
    }
  }
  else if (cmd.startsWith("addw"))
  {
    String ssid;
    String pass;

    String args = cmd.substring(5, cmd.length() - 1);

    int ind = args.indexOf(",");

    if (ind > 0)
    {
      ssid = args.substring(0, ind);
      pass = args.substring(ind + 1);
      wifiMulti.addAP(ssid.c_str(), pass.c_str());

      sysConfig->addWifiCredentials(ssid, pass);

      DEBUG("Adding Wifi " + ssid);

      sysConfig->saveToFile();
    }
  }
  else if (cmd.startsWith("set_clock"))
  {
    String args = cmd.substring(10, cmd.length() - 1);

    set_datetime(args);

    String rsp = "Time set to " + args + "\n";

    ws.binary(cid, rsp);
  }
  else
  {
    DEBUG("Unknown command " + cmd);
  }

  free(cc);
}

void on_ws_event(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  if (type == WS_EVT_CONNECT)
  {
    // client connected
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    // client->printf("Hello Client %u :)", client->id());
    client->ping();
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    // client disconnected
    Serial.printf("ws[%s][%d] disconnected\n", server->url(), client->id());
  }
  else if (type == WS_EVT_ERROR)
  {
    // error was received from the other end
    Serial.printf("ws[%s][%u] error(%d): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
  }
  else if (type == WS_EVT_PONG)
  {
    // pong message was received (in response to a ping request maybe)
    // Serial.printf("ws[%s][%u] pong[%d]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
  }
  else if (type == WS_EVT_DATA)
  {
    // Serial.printf("ws[%s][%u] data[%d]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");

    process_websocket_messages(data, len, client->id());
  }
}

bool check_timeout(unsigned long timer, int timeout = TIMEOUT)
{
  bool res = millis() - timer > timeout;

  if (res)
  {
    timer = millis();
  }
  return res;
}

bool check_req_timeout(int timeout = TIMEOUT)
{
  bool res = millis() - mTimer > timeout;

  if (res)
  {
    mTimer = millis();
  }
  return res;
}

bool addKnownWifis()
{
  // Get available Wifi clearWifiCredentials()
  for (size_t i = 0; i < MAX_NUM_WIFI_CREDENTIALS; i++)
  {
    String ssid = sysConfig->mWifiCredentials[i][0];
    String pass = sysConfig->mWifiCredentials[i][1];

    if (!ssid.isEmpty() && !pass.isEmpty())
    {
      DEBUG("Adding " + ssid + ":  ***");
      wifiMulti.addAP(ssid.c_str(), pass.c_str());
    }
  }

  return false;
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(460800);
  Serial2.begin(460800);

  Serial.setRxBufferSize(1024);  // Default 256
  Serial2.setRxBufferSize(1024); // Default 256

  Serial.println("\r\nWibee " + String(VERSION) + " Starting....");
  Serial.println("Compiled on: " + String(__DATE__) + " " + String(__TIME__));

  lastRxTime = millis();

  set_datetime("2022-1-1T12:00:00");

  print_mem();

  if (SD_MMC.begin())
  {
    // Load the settings
    DEBUG("SD Card Present");

    sysConfig = new systemConfiguration();

    if (sysConfig->readFromFile(CONFIGURATION_DATA_FILE))
    {
      DEBUG("System configuration read");
      DEBUG(sysConfig->toJson(false));
    }
    else
    {
      DEBUG("Invalid system configuration file. Saving default configuration");
      sysConfig->checkDefaultWifiCredentials();
      sysConfig->saveToFile();
    }

    addKnownWifis();

    mLogMemory = (char *)malloc(LOG_MEM_SIZE);

    if (!mLogMemory)
    {
      DEBUG("Not Enough Memory");
    }

    mLogMemoryIndex = 0;
  }
  else
  {
    Serial.println("Failed to initialize SD card");
  }

  IPAddress localIP(37, 155, 1, 1);
  IPAddress gateway(37, 155, 1, 0);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.softAPConfig(localIP, gateway, subnet);
  String softAPName = "wibee";
  WiFi.softAP(softAPName.c_str(), DEFAULT_WIFIAP_PASSWORD, 1, 0, 1);

  WiFi.setHostname(softAPName.c_str());

  if (wifiMulti.run() == WL_CONNECTED)
  {
    DEBUG("");
    DEBUG("WiFi connected to " + WiFi.SSID());
    DEBUG("IP address: ");
    DEBUG(WiFi.localIP());
  }
  else
  {
    DEBUG("WiFi NOT connected");
  }

  WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {

  },
               SYSTEM_EVENT_AP_STADISCONNECTED);

  DEBUG("");

  // attach AsyncWebSocket
  ws.onEvent(on_ws_event);
  server.addHandler(&ws);

  server.on(
      "/update", HTTP_POST, [](AsyncWebServerRequest *request)
      { request->send(200); },
      onUpdate);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });

  server.serveStatic("/log", SD_MMC, LOG_FILE_PATH);

  AsyncElegantOTA.begin(&server); // Start ElegantOTA

  server.begin();

  mTimer = millis();

  AdvertiseServices();

  strcpy(rx1, "RX1,");
  strcpy(rx2, "RX2,");
}

bool checkWiFi()
{
  if (wifiMulti.run() != WL_CONNECTED)
  {
    mIsWifiOnline = false;

    return false;
  }
  else
  {
    if (!mIsWifiOnline)
    {
      DEBUG("");
      DEBUG("WiFi connected to " + WiFi.SSID());
      DEBUG("IP address: ");
      DEBUG(WiFi.localIP());

      AdvertiseServices();
    }

    mIsWifiOnline = true;

    return true;
  }
}

int charCnt = 0;

void loop()
{

  // Check serial port and push them through websocket

  if (check_req_timeout())
  {
    // Check Wifi Connection
    checkWiFi();
  }

  charCnt = 0;
  while (Serial.available())
  {
    rx1[rxInd++] = Serial.read();

    lastRxTime = millis();

    if (charCnt++ == 128)
    {
      break;
    }
  }

  if (rxInd > 1024 || (rxInd > 4 && check_timeout(lastRxTime, 100)))
  {
    rx1[rxInd++] = 0;

    if (ws.count() > 0)
    {
      ws.binaryAll(rx1);
    }

    if (mLogMemory)
    {
      // Write to the buffer
      strcpy(&mLogMemory[mLogMemoryIndex], &rx1[4]);
      mLogMemoryIndex += (rxInd - 5);
    }

    rxInd = 4;

    strcpy(rx1, "RX1,");
  }

  charCnt = 0;
  while (Serial2.available())
  {
    rx2[rx2Ind++] = Serial2.read();

    lastRx2Time = millis();

    if (charCnt++ == 128)
    {
      break;
    }
  }

  if (rx2Ind > 1024 || (rx2Ind > 4 && check_timeout(lastRx2Time, 100)))
  {
    rx2[rx2Ind++] = 0;
    if (ws.count() > 0)
    {
      ws.binaryAll(rx2);
    }

    rx2Ind = 4;

    strcpy(rx2, "RX2,");
  }

  if (mLogMemoryIndex > mSDWriteChunkSize)
  {
    if (enableLog)
    {
      // Write the chunk to the SD card.
      DEBUG("Writing to SD Card");

      fs::FS &fs = SD_MMC;
      logFile = fs.open(LOG_FILE_PATH, FILE_APPEND);
      logFile.write((uint8_t *)mLogMemory, mLogMemoryIndex);

      if (enableTimeStamp)
      {
        String nnow = "\n<" + datetime_now() + ">\n";
        logFile.write((uint8_t *)nnow.c_str(), nnow.length());
      }

      logFile.close();
    }

    mLogMemoryIndex = 0;
  }
}