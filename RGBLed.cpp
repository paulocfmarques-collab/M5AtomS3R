#include "RGBLed.h"
#include <M5Unified.h>

namespace { constexpr uint8_t ADDRESS=0x30; constexpr uint32_t FREQUENCY=400000; }
void RGBLed::begin() {
 // M5.begin() already enabled the shared driver and configured its W channel.
 uint8_t mapping=0;
 ready_=M5.In_I2C.readRegister(ADDRESS,0x70,&mapping,1,FREQUENCY);
 if(ready_) ready_=M5.In_I2C.writeRegister8(ADDRESS,0x70,mapping & 0xC0,FREQUENCY);
 // Set only RGB current registers to 5 mA/channel. Preserve white current/PWM.
 for(uint8_t reg=0x05;ready_ && reg<=0x07;++reg)
   ready_=M5.In_I2C.writeRegister8(ADDRESS,reg,50,FREQUENCY);
 apply();
 Serial.println(ready_?"[RGB] LP5562 pronto (RGB independente do backlight)":"[RGB] Falha de comunicacao com LP5562");
}
void RGBLed::apply() {
 if(!ready_)return;
 uint8_t pwm[3]={uint8_t(uint16_t(b_)*brightness_/255),uint8_t(uint16_t(g_)*brightness_/255),uint8_t(uint16_t(r_)*brightness_/255)};
 // Registers 0x02..0x04: B, G, R PWM. No writes to white PWM (0x0E).
 if(!M5.In_I2C.writeRegister(ADDRESS,0x02,pwm,3,FREQUENCY)) {
   ready_=false; Serial.println("[RGB] Falha ao atualizar LP5562");
 }
}
void RGBLed::setColor(uint8_t r,uint8_t g,uint8_t b){r_=r;g_=g;b_=b;apply();}
void RGBLed::setBrightness(uint8_t value){brightness_=value;apply();}
void RGBLed::breathingTask(uint8_t r,uint8_t g,uint8_t b){
 const uint32_t now=millis();if(now-breathAt_<20)return;breathAt_=now;
 const uint32_t phase=now%2400;const uint16_t level=(phase<1200?phase:2400-phase)*255/1200;
 setColor(uint16_t(r)*level/255,uint16_t(g)*level/255,uint16_t(b)*level/255);
}
void RGBLed::blink(uint8_t color,uint16_t,uint16_t){
 switch(color){case RED:red();break;case GREEN:green();break;case BLUE:blue();break;case YELLOW:yellow();break;case CYAN:cyan();break;case MAGENTA:magenta();break;default:white();}
}
