#include "DisplayUtil.h"
#include "ClimaManager.h"
#include <M5Unified.h>
#include <WiFi.h>

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
void DisplayUtil::begin() {
 auto cfg=M5.config(); cfg.fallback_board=m5::board_t::board_M5AtomS3R;
 cfg.internal_imu=false; cfg.internal_rtc=false; cfg.internal_mic=false; cfg.internal_spk=false;
 M5.begin(cfg); ready_=M5.Display.width()==128 && M5.Display.height()==128;
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
 M5.Display.setRotation(0); M5.Display.setTextWrap(true); setBacklight(brightness_);
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
void DisplayUtil::setRotation(uint8_t r) { if(ready_)M5.Display.setRotation(r&3); }
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
 static float lookX = 0, lookY = 0;
 static int targetX = 0, targetY = 0;
 static bool blinking = false;
 const uint32_t now = millis();
 if (now - lastFrame < 33) return;
 lastFrame = now;
 if (!nextLook || int32_t(now - nextLook) >= 0) {
   targetX = random(-6, 7); targetY = random(-5, 6);
   nextLook = now + random(900, 2400);
 }
 lookX += (targetX - lookX) * 0.12f;
 lookY += (targetY - lookY) * 0.12f;
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
   const int cx = x + 23 + int(lookX), cy = 65 + int(lookY);
   const uint16_t iris = 0x07FF;
   frame.fillRoundRect(x, 40, 46, 50, 16, 0xDFFF);
   frame.fillCircle(cx, cy, 16, iris);
   frame.drawCircle(cx, cy, 13, 0x041F);
   frame.fillCircle(cx, cy, 8, 0);
   frame.fillCircle(cx - 4, cy - 5, 3, 0xFFFF);
   frame.fillCircle(cx + 4, cy + 4, 1, 0xFFFF);
   // Eyelids are composed in the same buffer, keeping blinking free of screen flashes.
   if (lid) {
     frame.fillRect(x, 40, 46, lid, 0);
     frame.fillRect(x, 90 - lid, 46, lid, 0);
   }
 }
 frame.pushSprite(0, 0);
}
void DisplayUtil::dispararAnimacaoGravacaoSD() {}
void DisplayUtil::print(String text) { println(text); }
void DisplayUtil::println(String text) { text.replace("\r",""); while(text.length()){int nl=text.indexOf('\n');int n=min(13,nl<0?int(text.length()):nl);addLine(text.substring(0,n));text.remove(0,n+(nl==n?1:0));} redraw(); }
void DisplayUtil::addLine(String line) { lines.push_back(line); while(lines.size()>7)lines.erase(lines.begin()); }
void DisplayUtil::redraw() { String body;for(const auto& line:lines)body+=line+"\n";page("Console",body); }
