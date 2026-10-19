#include "DisplayUtil.h"
#include "ClimaManager.h"
#include <M5Unified.h>
#include <WiFi.h>
#include <math.h>

namespace {
M5Canvas& pageBuffer() {
 static M5Canvas buffer(&M5.Display);
 return buffer;
}
}

DisplayUtil::DisplayUtil() : textColor(0xFFFF), backgroundColor(0), textSize(1) {}
void DisplayUtil::page(const String& title, const String& body) {
 if (!ready_) return;
 auto& frame = pageBuffer();
 frame.fillScreen(backgroundColor);
 frame.setTextWrap(false);
 // Every logical line stays on one row. Long identifiers use a smaller font.
 auto row = [&](String value, int y, uint16_t color, float size) {
   frame.setTextSize(size);
   if (frame.textWidth(value) > 122) frame.setTextSize(1.25f);
   if (frame.textWidth(value) > 122) {
     while (value.length() && frame.textWidth(value + "..") > 122) value.remove(value.length()-1);
     value += "..";
   }
   frame.setTextColor(color, backgroundColor);
   frame.setCursor(3, y); frame.print(value);
 };
 row(title, 3, 0x07FF, 1.5f);
 frame.drawFastHLine(0, 20, 128, 0x07FF);
 int start = 0;
 for (int y = 27; y <= 111 && start < int(body.length()); y += 14) {
   int end = body.indexOf('\n', start);
   if (end < 0) end = body.length();
   row(body.substring(start, end), y, textColor, 1.5f);
   start = end + 1;
 }
 frame.pushSprite(0, 0);
}
void DisplayUtil::showOtaProgress(uint8_t percent, const String& status) {
 if (!ready_) return;
 percent = min(uint8_t(100), percent);
 auto& frame = pageBuffer();
 frame.fillScreen(0); frame.setTextWrap(false);
 frame.setTextColor(0x07FF, 0); frame.setTextSize(1.25f);
 const String title = "Atualizacao OTA";
 frame.setCursor((128 - int(frame.textWidth(title))) / 2, 7); frame.print(title);
 frame.drawFastHLine(5, 25, 118, 0x07FF);
 frame.setTextColor(0xFFFF, 0); frame.setTextSize(3);
 const String value = String(percent) + "%";
 frame.setCursor((128 - int(frame.textWidth(value))) / 2, 38); frame.print(value);
 frame.drawRoundRect(8, 73, 112, 18, 4, 0x07FF);
 const int width = uint32_t(percent) * 106 / 100;
 if (width) frame.fillRect(11, 76, width, 12, 0x07FF);
 frame.setTextSize(1.25f);
 frame.setCursor(max(0, (128 - int(frame.textWidth(status))) / 2), 104);
 frame.print(status);
 frame.pushSprite(0, 0);
}

void DisplayUtil::begin() {
 auto cfg=M5.config(); cfg.fallback_board=m5::board_t::board_M5AtomS3R;
 cfg.internal_imu=true; cfg.internal_rtc=false; cfg.internal_mic=false; cfg.internal_spk=false;
 M5.begin(cfg); ready_=M5.Display.width()==128 && M5.Display.height()==128;
 Serial.println(M5.Imu.isEnabled() ? "[IMU] Acelerometro pronto: olhos seguem o movimento" : "[IMU] Indisponivel: olhos usam movimento automatico");
 if (ready_) {
   auto& frame = pageBuffer();
   frame.setColorDepth(16);
   if (!frame.createSprite(128,128)) {
     Serial.println("[LCD] Sem memoria para o buffer da tela");
     ready_ = false;
   }
   frame.setTextWrap(false);
 }
 Serial.printf("[LCD] AtomS3R %dx%d: %s\n",M5.Display.width(),M5.Display.height(),ready_?"OK":"falha: atualize M5Unified e M5GFX");
 M5.Display.setRotation(2); M5.Display.setTextWrap(true); setBacklight(brightness_);
 page("AtomS3R", "Inicializando...\n\nAplicacao: core 1\nClima: core 0");
}
void DisplayUtil::setBacklight(uint8_t p) { brightness_=min(uint8_t(100),p); wake(); }
void DisplayUtil::wake() {
 lastActivity_=millis();
 updatePower();
}
void DisplayUtil::updatePower() {
 uint8_t level = uint32_t(millis()-lastActivity_>=120000?min(uint8_t(20),brightness_):brightness_)*255/100;
 if (ready_ && level != appliedBrightness_) {
   M5.Display.setBrightness(level);
   appliedBrightness_ = level;
 }
}
void DisplayUtil::clear() { lines.clear(); if(ready_){M5.Display.fillScreen(backgroundColor); M5.Display.setCursor(0,0);} }
void DisplayUtil::setTextColor(uint16_t c) { textColor=c; if(ready_)M5.Display.setTextColor(c,backgroundColor); }
void DisplayUtil::setBackgroundColor(uint16_t c) { backgroundColor=c; }
void DisplayUtil::setTextSize(uint8_t s) { textSize=s; if(ready_)M5.Display.setTextSize(s); }
void DisplayUtil::setRotation(uint8_t r) { if(ready_)M5.Display.setRotation(r & 3); }
void DisplayUtil::showClock(String date) {
 if (!ready_) return;
 auto& frame = pageBuffer(); auto c = ClimaManager::snapshot();
 frame.fillScreen(backgroundColor); frame.setTextWrap(false);
 auto centered = [&](const String& value, int y, float size, uint16_t color) {
   frame.setTextSize(size); frame.setTextColor(color, backgroundColor);
   frame.setCursor(max(0, (128 - int(frame.textWidth(value))) / 2), y); frame.print(value);
 };
 // NTP format: DD/MM/YYYY HH:MM:SS; never wrap a date or a time across rows.
 centered("AtomS3R", 3, 1.5f, 0x07FF);
 frame.drawFastHLine(0, 21, 128, 0x07FF);
 centered(date.length() >= 19 ? date.substring(11,19) : String("--:--:--"), 31, 2.25f, textColor);
 centered(date.length() >= 19 ? date.substring(0,10) : String("Sem hora"), 57, 1.75f, textColor);
 centered(WiFi.status()==WL_CONNECTED ? WiFi.localIP().toString() : String("AP: 192.168.4.1"), 83, 1.25f, 0xBDF7);
 centered(c.sincronizado ? String(c.temperatura,1)+" C" : String("Clima aguardando"), 105, 1.25f, 0x07FF);
 frame.pushSprite(0,0);
}
void DisplayUtil::showStatusPage(bool,bool sync,const String& date,int32_t zone,bool dst,uint8_t) {
 page("NTP", String(sync?"Sincronizado":"Aguardando")+"\n"+
 (date.length()>=19?date.substring(0,10)+"\n"+date.substring(11,19):String("Sem data/hora"))+
 "\nFuso: "+String(zone)+"\nDST: "+String(dst?"ON":"OFF"));
}
void DisplayUtil::showNetworkPage(uint8_t) { page("Rede",WiFi.status()==WL_CONNECTED?WiFi.SSID()+"\n"+WiFi.localIP().toString()+"\nRSSI: "+String(WiFi.RSSI())+" dBm\nGateway:\n"+WiFi.gatewayIP().toString():"AP: ATOMS3_CONFIG\n192.168.4.1\nBuscando:\n"+connecting_); }
void DisplayUtil::showSystemPage() { page("Sistema",String(ESP.getChipModel())+"\nCores: "+String(ESP.getChipCores())+"\nHeap: "+String(ESP.getFreeHeap()/1024)+" KB\nPSRAM: "+String(ESP.getPsramSize()/1024/1024)+" MB\nUptime: "+String(millis()/1000)+" s"); }
void DisplayUtil::showSavedWifiPage(const String ssids[5],int slot,uint8_t) { String body; for(int i=0;i<5;++i)body+=String(i==slot?"> ":"  ")+String(i+1)+":"+ssids[i].substring(0,12)+"\n"; page("Redes salvas",body); }
void DisplayUtil::showHoldMessage(const String& a,const String& b,uint16_t c) { setTextColor(c); page(a,b); setTextColor(0xFFFF); Serial.println(a+" "+b); }
void DisplayUtil::showSdPage(bool,const String&,uint64_t,uint64_t) { page("Armazenamento","Sem leitor microSD\nintegrado"); }
void DisplayUtil::showSdLoading(uint8_t) { showSdPage(false,"",0,0); }
void DisplayUtil::showMonitorPage() { if(!monitor_){page("Monitor","Aguardando...");return;} const auto& m=*monitor_; page("Monitor de rede",String(m.connected?"WiFi conectado":"WiFi desconectado")+"\nPing: "+(!m.hasResult?String("aguardando"):m.replied?String(m.rtt)+" ms":String("timeout"))+"\nQuedas: "+String(m.drops)+"\nFalhas: "+String(m.failures)); }
void DisplayUtil::showForecastPage() { auto c=ClimaManager::snapshot(); page("Porto Alegre",c.previsaoValida?String(c.data)+"\nMin: "+String(c.minima,1)+" C\nMax: "+String(c.maxima,1)+" C\nChuva: "+String(c.chuva)+"%\nIdade: "+String((millis()-c.previsaoAtualizada)/60000)+" min":String("Aguardando clima")); }
void DisplayUtil::desenharOlhosScreensaver() {
 if (!ready_) return;
 static uint32_t lastFrame = 0, nextLook = 0, nextBlink = 0, blinkAt = 0;
 static uint32_t lastAccelAt = 0;
 static float lookX = 0, lookY = 0;
 static float targetX = 0, targetY = 0;
 static float neutralX = 0, neutralY = 0, filteredX = 0, filteredY = 0;
 static bool referenceReady = false;
 static bool blinking = false;
 enum Expression : uint8_t { HAPPY, CURIOUS, SURPRISED, SLEEPY };
 static uint8_t expression = HAPPY;
 static uint32_t nextExpressionAt = 0, lastSurpriseAt = 0;
 static float topLids[2] = {}, bottomLids[2] = {};
 const uint32_t now = millis();
 if (now - lastFrame < 33) return;
 // A new screensaver session uses the current position as its neutral pose.
 if (now - lastFrame > 250) referenceReady = false;
 lastFrame = now;
 float ax = 0, ay = 0, az = 0;
 bool suddenMotion = false;
 if (M5.Imu.isEnabled() && M5.Imu.getAccel(&ax, &ay, &az) &&
     isfinite(ax) && isfinite(ay) && isfinite(az)) {
   const float acceleration = sqrtf(ax * ax + ay * ay + az * az);
   suddenMotion = acceleration > 1.65f || acceleration < 0.35f;
   // M5Unified corrects sensor mounting axes; rotate them into LCD coordinates.
   float x = ax, y = -ay;
   switch (M5.Display.getRotation() & 3) {
     case 1: x = ay; y = ax; break;
     case 2: x = -ax; y = ay; break;
     case 3: x = -ay; y = -ax; break;
   }
   if (!referenceReady) {
     neutralX = filteredX = x; neutralY = filteredY = y;
     referenceReady = true;
   }
   filteredX += (x - filteredX) * 0.18f;
   filteredY += (y - filteredY) * 0.18f;
   const float dx = filteredX - neutralX, dy = filteredY - neutralY;
   // Ignore small vibrations, and keep both irises inside their eye outlines.
   targetX = fabsf(dx) < 0.03f ? 0 : constrain(dx * 12.0f, -6.0f, 6.0f);
   targetY = fabsf(dy) < 0.03f ? 0 : constrain(dy * 10.0f, -5.0f, 5.0f);
   lastAccelAt = now;
 }
 // Retain automatic motion only when no valid accelerometer data is available.
 if ((!referenceReady || now - lastAccelAt > 1000) &&
     (!nextLook || int32_t(now - nextLook) >= 0)) {
   targetX = random(-6, 7); targetY = random(-5, 6);
   nextLook = now + random(900, 2400);
 }
 lookX += (targetX - lookX) * 0.12f;
 lookY += (targetY - lookY) * 0.12f;
 if (suddenMotion && now - lastSurpriseAt >= 4000) {
   expression = SURPRISED; lastSurpriseAt = now; nextExpressionAt = now + 2000;
 } else if (!nextExpressionAt || int32_t(now - nextExpressionAt) >= 0) {
   // Pick a different expression without blocking the network or the animation.
   if (nextExpressionAt) expression = (expression + 1 + random(3)) % 4;
   nextExpressionAt = now + random(3500, 6500);
 }
 if (!nextBlink) nextBlink = now + random(2000, 5000);
 if (!blinking && int32_t(now - nextBlink) >= 0) { blinking = true; blinkAt = now; }
 int lid = 0;
 if (blinking) {
   const uint32_t elapsed = now - blinkAt;
   if (elapsed >= 200) { blinking = false; nextBlink = now + random(2500, 6000); }
   else lid = (elapsed < 100 ? elapsed : 200 - elapsed) * 25 / 100;
 }
 auto& frame = pageBuffer();
 frame.fillScreen(0);
 for (int eye = 0; eye < 2; ++eye) {
   const int x = eye ? 70 : 12;
   const int cx = x + 23 + int(lookX), cy = 53 + int(lookY);
   const uint16_t iris = 0x07FF;
   frame.fillRoundRect(x, 28, 46, 50, 16, 0xDFFF);
   frame.fillCircle(cx, cy, 16, iris);
   frame.drawCircle(cx, cy, 13, 0x041F);
   frame.fillCircle(cx, cy, expression == SURPRISED ? 6 : 8, 0);
   frame.fillCircle(cx - 4, cy - 5, 3, 0xFFFF);
   frame.fillCircle(cx + 4, cy + 4, 1, 0xFFFF);
   const float targetTop = expression == SLEEPY ? 19 : expression == CURIOUS ? (eye ? 12 : 2) : 0;
   const float targetBottom = expression == HAPPY ? 6 : expression == SLEEPY ? 10 : 0;
   topLids[eye] += (targetTop - topLids[eye]) * 0.18f;
   bottomLids[eye] += (targetBottom - bottomLids[eye]) * 0.18f;
   const int top = max(lid, int(topLids[eye])), bottom = max(lid, int(bottomLids[eye]));
   if (top) frame.fillRect(x, 28, 46, top, 0);
   if (bottom) frame.fillRect(x, 78 - bottom, 46, bottom, 0);
   // Eyebrows reinforce the expression without touching the moving eyes.
   const int browY = expression == SURPRISED ? 13 : expression == SLEEPY ? 23 : 19;
   const int tilt = expression == CURIOUS ? (eye ? 5 : -5) : 0;
   for (int stroke = 0; stroke < 2; ++stroke)
     frame.drawLine(x + 8, browY + stroke, x + 37, browY + tilt + stroke, iris);
 }
 // Mouth sits below the eyes, entirely inside the 128x128 canvas.
 const int mouthX = 64 + int(lookX * 0.4f);
 if (expression == SURPRISED) {
   frame.fillRoundRect(mouthX - 7, 94, 14, 21, 7, 0x07FF);
   frame.fillRoundRect(mouthX - 4, 97, 8, 15, 4, 0);
 } else if (expression == SLEEPY) {
   frame.fillRoundRect(mouthX - 11, 103, 22, 4, 2, 0x07FF);
 } else if (expression == CURIOUS) {
   for (int stroke = 0; stroke < 3; ++stroke)
     frame.drawLine(mouthX - 10, 108 + stroke, mouthX + 10, 103 + stroke, 0x07FF);
 } else {
   // A curved smile, with a tiny breathing motion.
   const int bounce = int(sinf(now * 0.002f) * 1.5f);
   for (int dx = -17; dx <= 17; ++dx) {
     const int y = 102 + bounce + (289 - dx * dx) * 10 / 289;
     frame.fillCircle(mouthX + dx, y, 1, 0x07FF);
   }
 }
 frame.pushSprite(0, 0);
}
void DisplayUtil::dispararAnimacaoGravacaoSD() {}
void DisplayUtil::print(String text) { println(text); }
void DisplayUtil::println(String text) { text.replace("\r",""); while(text.length()){int nl=text.indexOf('\n');int n=min(13,nl<0?int(text.length()):nl);addLine(text.substring(0,n));text.remove(0,n+(nl==n?1:0));} redraw(); }
void DisplayUtil::addLine(String line) { lines.push_back(line); while(lines.size()>7)lines.erase(lines.begin()); }
void DisplayUtil::redraw() { String body;for(const auto& line:lines)body+=line+"\n";page("Console",body); }
