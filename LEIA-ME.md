# AtomS3R Gateway

Documentação atualizada em **18/10/2026**.

Firmware adaptado do ESP32C6 para o **M5Stack AtomS3R**, com tela IPS 128 × 128, flash de 8 MB e PSRAM OPI de 8 MB. O projeto ESP32C6 original permanece intacto.

## Configuração da Arduino IDE

Instale ou atualize **M5Unified** e **M5GFX**. Não há dependência de Adafruit NeoPixel. Configuração com o pacote **esp32 by Espressif Systems 3.3.12** instalado:

| Opção | Valor |
| --- | --- |
| Placa | M5AtomS3; o M5Unified identifica o AtomS3R |
| Flash Size | 8MB (64Mb) |
| PSRAM | OPI PSRAM |
| Partition Scheme | 8M with spiffs (3MB APP/1.5MB SPIFFS), com OTA |
| USB CDC On Boot | Enabled |
| USB Mode | Hardware CDC and JTAG |
| Arduino Runs On | Core 1 |
| Events Run On | Core 0 |

Abra `M5AtomS3.ino` nesta pasta. O script `Compilar.ps1` na pasta superior usa as mesmas opções. Compilação e gravação ficam com o usuário.

## Tela e botão

Use `rotacao:0`, `rotacao:90`, `rotacao:180` ou `rotacao:270` pelo serial ou UDP para escolher a orientação absoluta. Ela vale para todas as páginas, carinha e barra OTA e é mantida ao receber outros comandos. Não é salva em memória: ao reiniciar, volta ao padrão de 180°.

A orientação padrão da tela é girada em 180°, incluindo relógio, páginas, console, olhos e barra OTA. O movimento dos olhos considera essa rotação.

M5Unified/M5GFX inicializam o painel e o backlight controlado pelo LP5562. O serial informa `[LCD] AtomS3R 128x128: OK`. O relógio tem hora ampliada, data centralizada abaixo e IP em outra linha. As demais páginas usam fonte maior, com ajuste de linhas longas. O desenho é composto em memória e enviado pronto para evitar a piscada de limpeza do LCD.

**Cada toque curto no botão frontal GPIO41 avança uma tela ao soltar:**

Relógio → NTP → Rede → Sistema → Redes salvas → Monitor → Previsão → Relógio.

As páginas duplicadas de NTP/rede e o aviso de SD são pulados pelo botão e por `tela_next`. A seleção direta `tela:0..9` mantém a numeração descrita em [comandos.md](comandos.md).

**Após 20 segundos sem novo toque, a tela volta ao relógio.** Cada toque reinicia o prazo. Selecionar uma página por comando também concede 20 segundos. Esse retorno não reinicia o tempo do protetor de tela.

- Segurar por 3 segundos e soltar: reinicia a placa.
- Segurar por 10 segundos e soltar: apaga os perfis Wi-Fi e reinicia.
- Comandos aparecem no console da tela por cerca de 10 segundos; navegação seleciona a página imediatamente.
- Após dois minutos de inatividade, o brilho cai para no máximo 20%. Botão ou comando restaura o valor configurado. Com brilho zero, use `tela_on` ou `brilho:P`.

## Olhos e acelerômetro

O protetor agora inclui boca e sobrancelhas ciano: a carinha alterna entre feliz, curiosa, surpresa e sonolenta a cada 3,5 a 6,5 segundos. As pálpebras mudam suavemente e um movimento forte provoca uma reação de surpresa. A boca fica abaixo dos olhos, preservando a orientação de 180° e o desenho em buffer.

Após **60 segundos sem botão ou comando**, o protetor mostra dois olhos ciano. O acelerômetro BMI270 move o olhar conforme a inclinação e aceleração, com suavização, zona morta contra tremores e limites de deslocamento. A posição ao entrar no protetor é a referência central; as piscadas continuam automáticas.

Mover a placa não encerra o protetor; botão ou comando retorna às páginas. Se o sensor estiver indisponível, os olhos usam movimento automático. O boot informa `[IMU] Acelerometro pronto: olhos seguem o movimento` quando a inicialização funciona. Giroscópio e magnetômetro ainda não são utilizados.

## LED RGB

O RGB U5 é controlado pelo LP5562 no endereço I2C 0x30. Os comandos `led_on`, `led_off`, `led_breath`, `led_pisca:P:I`, `led_blink:I` e `led_rgb:R:G:B` atuam somente nos canais R/G/B, preservando o canal W do backlight. Exemplo: `led_rgb:0:255:255` para ciano.

Comandos manuais têm prioridade sobre a indicação automática de Wi-Fi. O LED verde D1 pertence ao circuito de reset/download. Consulte os limites e exemplos em [comandos.md](comandos.md).

## Dois núcleos e serviços

O Prometheus exporta `sensor_temperature_celsius` e `sensor_humidity_percent` com `source="open_meteo"` e `location="porto_alegre"`. O filtro de modelo usa `esp_info{model="M5Stack-AtomS3R"}`. Consulte as consultas completas e o comportamento do cache em [PROMETHEUS.md](PROMETHEUS.md).

O clima é consultado assim que houver Wi-Fi e, depois, a cada **15 minutos** desde a última tentativa, inclusive após falhas. `clima_sync` permite antecipar uma consulta. Sem rede ou durante OTA, a consulta aguarda a disponibilidade do serviço.

A tarefa `application` fica no núcleo 1 e atende portal, comandos, display, acelerômetro do protetor, botão, RGB, NTP e OTA. A tarefa `weather` fica no núcleo 0 e consulta o Open-Meteo para Porto Alegre. O cache de clima é protegido entre núcleos.

| Serviço | Acesso |
| --- | --- |
| Console USB | 115200 baud; envie `help` |
| Console UDP | Porta 4210; até 255 bytes por comando |
| AP de configuração | ATOMS3_CONFIG |
| Configuração Wi-Fi no AP | http://192.168.4.1/wifi |
| Portal conectado | http://IP_DA_PLACA/; porta 80 |
| Configuração Prometheus | /prometheus na porta 80 |
| Métricas | /metrics na porta 9120 por padrão; porta configurável |
| Hostname OTA | AtomS3R-Gateway |

São mantidos até cinco perfis Wi-Fi em NVS. O AtomS3R não tem leitor microSD integrado: comandos `sd_*` e `net_history` retornam indisponibilidade. Não há página de logs de SD nem rotas `/logs` e `/api/logs`.

## Gravação OTA

A OTA é atendida antes do portal e das animações. Durante a transferência, a tarefa principal fica dedicada à gravação; a tela mostra uma barra de progresso e a porcentagem, enquanto botão e portal ficam sem atualização. O Wi-Fi fica sem economia de energia, a CPU usa 240 MHz e a prioridade da tarefa aumenta temporariamente. Novas consultas de clima são impedidas; uma consulta em andamento pode concluir.

A barra avança após pelo menos 5 pontos percentuais e respeita um intervalo mínimo de 250 ms entre desenhos, exceto ao chegar a 100%. Exibe “Gravando...”, “Verificando...”, “Concluido!” ou o código de erro. O serial informa progresso a cada aproximadamente 10% e registra duração e taxa média em KiB/s. Frequência e prioridade são restauradas ao terminar ou falhar; o Wi-Fi continua sem economia de energia para novas tentativas. Não há ganho de velocidade medido na placa nesta revisão.

## Validação e documentação

O endpoint Prometheus também exporta `esp_udp_listener_up`, `esp_udp_packets_received_total`, `esp_udp_replies_sent_total` e `esp_udp_send_errors_total`. As definições e consultas estão em [PROMETHEUS.md](PROMETHEUS.md). Respostas completas são contadas separadamente dos blocos UDP.

Para investigar respostas UDP, envie `net_monitor` pelo serial ou consulte o relatório web: os contadores mostram RX, blocos TX, falhas TX e tentativas de recuperação. Falhas de envio também aparecem no serial com o prefixo `[UDP]`, destino e errno. O socket é reaberto após reconexões e recebe uma tentativa de recuperação quando o envio falha. A eficácia no problema de longa duração ainda precisa ser confirmada na placa.

Os fontes e esta documentação passaram por revisão estática. A compilação e os testes físicos das alterações recentes ficam com o usuário. Conferir botão, retorno em 20 segundos, relógio, olhos após 60 segundos, inclinação, RGB, redes, portal e OTA.

- [Comandos, limites e exemplos](comandos.md)
- [Histórico de alterações](changelog.md)
- [Documentação oficial do AtomS3R](https://docs.m5stack.com/en/core/AtomS3R)
- [Display com M5Unified](https://docs.m5stack.com/en/arduino/m5atoms3r/display)
