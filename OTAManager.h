#pragma once
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <esp32-hal-cpu.h>
#include "ClimaManager.h"
#include "DisplayUtil.h"

extern DisplayUtil display;

class OTAManager {
 inline static bool active_ = false;
 inline static uint32_t startedAt_ = 0, bytes_ = 0, savedCpu_ = 240;
 inline static int lastProgress_ = -10;
 inline static int lastScreenPercent_ = 0;
 inline static uint32_t lastScreenAt_ = 0;
 inline static UBaseType_t savedPriority_ = 1;

 static void finish() {
   if (!active_) return;
   active_ = false;
   ClimaManager::pauseForOta(false);
   vTaskPrioritySet(nullptr, savedPriority_);
   if (savedCpu_ != 240) setCpuFrequencyMhz(savedCpu_);
   // Keep sleep disabled: a failed upload can be retried without radio wake delays.
 }
public:
 static void begin(const char* hostname = "AtomS3R-Gateway") {
   ArduinoOTA.setHostname(hostname);
   WiFi.setSleep(false);
   ArduinoOTA.onStart([]() {
     active_ = true;
     startedAt_ = millis(); bytes_ = 0; lastProgress_ = -10;
     lastScreenPercent_ = 0; lastScreenAt_ = startedAt_;
     savedCpu_ = getCpuFrequencyMhz();
     if (savedCpu_ != 240) setCpuFrequencyMhz(240);
     savedPriority_ = uxTaskPriorityGet(nullptr);
     if (savedPriority_ < 2) vTaskPrioritySet(nullptr, 2);
     ClimaManager::pauseForOta(true);
     WiFi.setSleep(false);
     display.wake();
     display.showOtaProgress(0, "Gravando...");
     Serial.println("[OTA] Transferencia prioritaria iniciada");
   });
   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
     bytes_ = progress;
     const int percent = total ? uint64_t(progress) * 100 / total : 0;
     // Draw at most four frames/second and only after a meaningful advance.
     if ((percent >= lastScreenPercent_ + 5 && millis() - lastScreenAt_ >= 250) ||
         (percent == 100 && lastScreenPercent_ != 100)) {
       lastScreenPercent_ = percent;
       lastScreenAt_ = millis();
       display.showOtaProgress(percent, percent == 100 ? "Verificando..." : "Gravando...");
     }
     if (percent == 100 || percent >= lastProgress_ + 10) {
       lastProgress_ = percent;
       Serial.printf("[OTA] %d%%\n", percent);
     }
   });
   ArduinoOTA.onEnd([]() {
     display.showOtaProgress(100, "Concluido!");
     const uint32_t elapsed = max(uint32_t(1), uint32_t(millis() - startedAt_));
     Serial.printf("[OTA] Concluido: %lu bytes em %.2f s (%.1f KiB/s)\n",
                   (unsigned long)bytes_, elapsed / 1000.0,
                   bytes_ * 1000.0 / (1024.0 * elapsed));
     finish();
   });
   ArduinoOTA.onError([](ota_error_t error) {
     display.showOtaProgress(lastScreenPercent_, "Erro " + String(error));
     Serial.printf("[OTA] Erro %u; retomando servicos\n", error);
     finish();
   });
   ArduinoOTA.begin();
 }
 static void handle() {
   // The first call accepts a pending invitation; the second starts it immediately.
   ArduinoOTA.handle();
   ArduinoOTA.handle();
 }
};
