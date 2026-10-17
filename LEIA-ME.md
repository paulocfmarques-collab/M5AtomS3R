# AtomS3R Gateway

Firmware adaptado do ESP32C6 para o M5Stack AtomS3R, com tela IPS 128 x 128, flash de 8 MB e PSRAM OPI de 8 MB. O projeto ESP32C6 original permanece intacto.

## Arduino IDE

Instale ou atualize as bibliotecas M5Unified e M5GFX pelo Gerenciador de Bibliotecas. Nao ha dependencia de Adafruit NeoPixel.

Configuracao com o pacote esp32 by Espressif Systems 3.3.12 instalado:

- Placa: M5AtomS3 (o driver M5Unified identifica o AtomS3R).
- Flash Size: 8MB (64Mb).
- PSRAM: OPI PSRAM (corrigido em relacao a configuracao anterior do Lite).
- Partition Scheme: 8M with spiffs (3MB APP/1.5MB SPIFFS), com OTA.
- USB CDC On Boot: Enabled.
- USB Mode: Hardware CDC and JTAG.
- Arduino Runs On: Core 1.
- Events Run On: Core 0.

Abra M5AtomS3.ino nesta pasta e compile. O script Compilar.ps1 na pasta superior usa as mesmas opcoes. A compilacao e gravacao ficam com o usuario.

## Tela e botao

M5Unified/M5GFX inicializam o painel e o backlight controlado pelo LP5562. O boot informa no serial `[LCD] AtomS3R 128x128: OK`. Em caso de falha de deteccao, atualize ambas as bibliotecas e informe a mensagem do serial.

O display mostra a inicializacao e depois o relogio, IP e temperatura. Pressao curta no botao frontal GPIO41 avanca entre relogio, NTP, rede, sistema, redes salvas, SD indisponivel, monitor e previsao. O conteudo foi adaptado para 128 x 128, em vez do painel maior do ESP32C6. Comandos aparecem no console da tela por 10 segundos. Brilho e comandos de tela estao habilitados. Apos 2 minutos o brilho cai para no maximo 20%; botao ou comando restaura o valor configurado.

Segurar o botao por 3 segundos e soltar reinicia. Segurar por 10 segundos e soltar apaga os perfis Wi-Fi e reinicia.

## Dois nucleos e servicos

A tarefa application fica no core 1: portal, comandos, display, botao, NTP e OTA. O worker weather fica no core 0 e consulta o Open-Meteo. O cache do clima possui protecao entre nucleos. O loop cede processamento ao sistema e a criacao da tarefa principal e verificada.

Mantidos cinco perfis Wi-Fi em NVS, UDP 4210, serial 115200, monitor de rede, Prometheus na porta 9120 e portal na porta 80. AP: ATOMS3_CONFIG, portal http://192.168.4.1/wifi. Hostname: AtomS3R-Gateway. Clima: Porto Alegre.

O AtomS3R nao tem leitor microSD integrado: os comandos SD retornam indisponibilidade, sem acionar GPIOs de um leitor inexistente. Sensores IMU e magnetometro ainda nao foram integrados.

O AtomS3R possui RGB U5 controlado pelo LP5562 (I2C 0x30). Os comandos led_* controlam somente R/G/B; o canal W continua dedicado ao backlight. O LED verde D1 pertence ao controlador de reset/download.

## Validacao

Revisao estatica dos fontes; compilacao e teste na placa ficam com o usuario. Conferir a mensagem LCD no boot, tela, brilho, botao, cadastro de redes, reconexao, clima, portal e OTA.

Referencias: https://docs.m5stack.com/en/core/AtomS3R e https://docs.m5stack.com/en/arduino/m5atoms3r/display

O portal web nao possui pagina ou API de logs do SD. Links, indicacao de SD e rotas /logs e /api/logs foram removidos deste dispositivo.

Protetor de tela: apos 60 segundos sem botao ou comando, dois olhos coloridos olham em direcoes diferentes e piscam. Botao ou comando restaura a tela normal. Animacao composta no buffer, sem apagar o LCD entre quadros.

Tela: dois olhos ciano; texto geral em escala 1.5, com ajuste de identificadores longos sem quebra automatica. Relogio com hora ampliada, data centralizada e IP em linhas separadas.
