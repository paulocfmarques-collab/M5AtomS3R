#include <Arduino.h>
#if !defined(CONFIG_IDF_TARGET_ESP32S3) || CONFIG_FREERTOS_UNICORE
#error "Selecione ESP32-S3 com dois nucleos habilitados"
#endif
#include "RGBLed.h"
#include "DisplayUtil.h"
#include "ESP32Gateway.h"
#include "NTPUtil.h"
#include "SDUtil.h"
#include "SharedSpi.h"
#include "CommandProcessor.h"
#include "OTAManager.h"
#include "ClimaManager.h"
#include <WiFi.h>
#include "SerialConsole.h"

constexpr uint8_t USER_BUTTON_PIN = 41; // Botao frontal AtomS3R

DisplayUtil display;
RGBLed rgbLed;
ESP32Gateway gateway;
NTPUtil ntp;
SDUtil sd;
CommandProcessor commandProcessor(display, gateway, ntp, rgbLed, sd);
SerialConsole serialConsole;

bool otaInicializadoCompleto = false;
bool sdDisponivel = false;
enum { PAGE_CLOCK = 0, PAGE_NTP_1, PAGE_NTP_2, PAGE_NET_1, PAGE_NET_2, PAGE_SYS, PAGE_WIFI, PAGE_SD, PAGE_MONITOR, PAGE_FORECAST, PAGE_COUNT };
uint32_t tempoUltimoComando = 0; // Monitor de ociosidade

uint8_t nextDisplayPage(uint8_t current)
{
  // Skip duplicate panels and the SD placeholder on this board.
  do { current = (current + 1) % PAGE_COUNT; }
  while (current == PAGE_NTP_2 || current == PAGE_NET_2 || current == PAGE_SD);
  return current;
}

void applicationTask(void*);
void applicationLoop();

void setup() 
{
  Serial.begin(115200);
  delay(100); 
  pinMode(USER_BUTTON_PIN, INPUT_PULLUP);

  SharedSpi::begin();
  // Inicializa o display AtomS3R e o adaptador sem SD.
  sdDisponivel = commandProcessor.begin();
  display.begin();
  display.clear();
  display.println("Sistema Inicializando...");
  delay(200); 

  delay(100);

  rgbLed.begin();
  gateway.begin();
  ntp.initNTP();
  tempoUltimoComando = millis();
  if (xTaskCreatePinnedToCore(applicationTask, "application", 16384, nullptr, 1, nullptr, 1) != pdPASS) {
    Serial.println("[FATAL] Falha ao criar tarefa no core 1");
    abort();
  }
  Serial.println("[Serial] Console pronto: 115200 baud, termine com Enter. Digite help.");
}


void applicationLoop() 
{
  static uint8_t currentPage = 0;
  static uint32_t lastFrame = 0, consoleUntil = 0;
  static bool consoleVisible = false;
  static bool screensaverActive = false;
  static int lastButtonReading = HIGH;
  static int buttonState = HIGH;
  static uint32_t lastDebounceTime = 0;
  static uint32_t buttonPressedAt = 0;
  static uint32_t lastNavigationAt = 0;
  static bool returnToClockArmed = false;
  static uint8_t holdStage = 0;
  static int8_t lastLedState = -1;
  String comando;

  // Handle the OTA invitation and transfer before HTTP rendering and peripherals.
  if (WiFi.status() == WL_CONNECTED) {
    if (!otaInicializadoCompleto) {
      OTAManager::begin("AtomS3R-Gateway");
      otaInicializadoCompleto = true;
    }
    OTAManager::handle();
  }

  display.updatePower();
  gateway.handleClient();
  commandProcessor.update();  

  int buttonReading = digitalRead(USER_BUTTON_PIN);
  if (buttonReading != lastButtonReading)
  {
    lastDebounceTime = millis();
  }
  if (millis() - lastDebounceTime >= 50 && buttonReading != buttonState)
  {
    buttonState = buttonReading;
    if (buttonState == LOW)
    {
      display.wake();
      buttonPressedAt = millis();
      lastNavigationAt = buttonPressedAt;
      holdStage = 0;
    }
    else if (holdStage == 0)
    {
      Serial.printf("[STATUS] Core=%d WiFi=%s Heap=%u\n", xPortGetCoreID(), WiFi.status() == WL_CONNECTED ? "OK" : "offline", ESP.getFreeHeap());
      currentPage = nextDisplayPage(currentPage);
      lastNavigationAt = millis();
      returnToClockArmed = currentPage != PAGE_CLOCK;
      consoleVisible = false;
      lastFrame = millis() - 1000;
      tempoUltimoComando = millis();
    }
    else if (holdStage == 1)
    {
      display.showHoldMessage("REINICIANDO", "", 0x07FF);
      delay(500);
      ESP.restart();
    }
    else
    {
      display.showHoldMessage("WI-FI APAGADO", "Reiniciando...", 0xF800);
      delay(500);
      gateway.clearConfig();
    }
  }
  lastButtonReading = buttonReading;

  if (buttonState == LOW)
  {
    lastNavigationAt = millis();
    uint32_t held = millis() - buttonPressedAt;
    if (held >= 10000 && holdStage < 2)
    {
      holdStage = 2;
      rgbLed.red();
      display.showHoldMessage("APAGAR WI-FI", "Solte p/ confirmar", 0xF800);
    }
    else if (held >= 3000 && holdStage < 1)
    {
      holdStage = 1;
      rgbLed.blue();
      display.showHoldMessage("REINICIAR", "Solte (ou segure 10s)", 0x07FF);
    }
    tempoUltimoComando = millis();
  }
  else if (commandProcessor.ledBusy())
  {
    lastLedState = -1;
  }
  else
  {
    int8_t ledState = WiFi.getMode() == WIFI_AP ? 1 : (WiFi.status() == WL_CONNECTED ? 0 : 2);
    if (ledState != lastLedState)
    {
      lastLedState = ledState;
      if (ledState == 0) rgbLed.green();
      else if (ledState == 1) rgbLed.yellow();
      else rgbLed.red();
    }
  }

  if (otaInicializadoCompleto && WiFi.status() == WL_CONNECTED)
  {
    ClimaManager::atualizar();
  }

  // Comandos UDP e serial usam o mesmo processamento.
  const bool serialCommand = serialConsole.poll(Serial, comando);
  if (serialCommand || gateway.receiveCommand(comando))
  {
    display.wake();
    commandProcessor.executeCommand(comando, serialCommand);
    sdDisponivel = commandProcessor.sdReady();
    consoleVisible = true; consoleUntil = millis() + 10000;
    tempoUltimoComando = millis();
  }

  int request = display.takePageRequest();
  if (request != -1) {
    currentPage = request == -2 ? nextDisplayPage(currentPage) : request % PAGE_COUNT;
    lastNavigationAt = millis();
    returnToClockArmed = currentPage != PAGE_CLOCK;
    consoleVisible = false; lastFrame = millis() - 1000;
  }
  if (returnToClockArmed && buttonState != LOW && millis() - lastNavigationAt >= 20000UL) {
    currentPage = PAGE_CLOCK;
    returnToClockArmed = false;
    consoleVisible = false;
    lastFrame = millis() - 1000;
  }
  if (consoleVisible && int32_t(millis() - consoleUntil) >= 0) consoleVisible = false;
  const bool idle = buttonState != LOW && !consoleVisible && display.idleFor(60000);
  if (idle) {
    screensaverActive = true;
    display.desenharOlhosScreensaver();
  } else if (screensaverActive) {
    screensaverActive = false;
    lastFrame = millis() - 1000;
    if (buttonState == LOW) display.showClock(ntp.getDateTime(10));
  }
  if (!idle && buttonState != LOW && !consoleVisible && millis() - lastFrame >= 1000) {
    lastFrame = millis();
    switch (currentPage) {
      case PAGE_CLOCK: display.showClock(ntp.getDateTime(10)); break;
      case PAGE_NTP_1: case PAGE_NTP_2:
        display.showStatusPage(false, ntp.isSincronizado(), ntp.getDateTime(10), ntp.getFusoHora(), ntp.isDstAtivo()); break;
      case PAGE_NET_1: case PAGE_NET_2: display.showNetworkPage(); break;
      case PAGE_SYS: display.showSystemPage(); break;
      case PAGE_WIFI: {
        String ssids[5]; int slot = -1;
        for (int i=0;i<5;++i) { ssids[i]=gateway.savedSsid(i); if(WiFi.status()==WL_CONNECTED && ssids[i]==WiFi.SSID())slot=i; }
        display.showSavedWifiPage(ssids,slot,gateway.nextSlot()); break;
      }
      case PAGE_SD: display.showSdPage(false,"",0,0); break;
      case PAGE_MONITOR: display.showMonitorPage(); break;
      case PAGE_FORECAST: display.showForecastPage(); break;
    }
  }
  vTaskDelay(pdMS_TO_TICKS(5));
}

void applicationTask(void*) {
  Serial.printf("[CPU] Aplicacao no core %d; clima fixado no core 0\n", xPortGetCoreID());
  for (;;) { applicationLoop(); }
}
void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
