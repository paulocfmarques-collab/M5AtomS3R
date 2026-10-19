# Histórico de alterações — AtomS3R Gateway

## 2026-10-19 — Temperatura e umidade de Porto Alegre no Prometheus

- Adicionada consulta de `relative_humidity_2m` no Open-Meteo e armazenamento no cache protegido entre núcleos, com validação de 0 a 100%.
- Exportadas `sensor_temperature_celsius` e `sensor_humidity_percent` com `source="open_meteo"` e `location="porto_alegre"`; séries omitidas antes de uma leitura válida.
- Preservado `esp_info{model="M5Stack-AtomS3R"}` para o filtro `and on(instance, job)` do dashboard.
- Intervalo de clima mantido em 15 minutos, sem chamadas de API durante o scrape.
- Criado PROMETHEUS.md com as consultas solicitadas e os limites do cache. Compilação e verificação no servidor ficam com o usuário.

## 2026-10-19 — Recuperação das respostas UDP

- Endereço e porta do remetente capturados na recepção, preservados durante o processamento e uma eventual recuperação do socket.
- Verificados os resultados de abertura do pacote, escrita e envio; falhas geram diagnóstico serial com destino e errno.
- Em falha de envio, socket fechado e reaberto na porta 4210, com uma única repetição do bloco; o comando não é executado novamente.
- Socket reaberto ao reconectar a STA/fechar o AP e ao voltar ao AP após perda de conexão.
- Respostas longas divididas explicitamente em datagramas de até 1200 bytes, evitando o envio automático sem verificação da biblioteca.
- `net_monitor` e relatório web incluem contadores de recepção, blocos enviados, falhas e tentativas de recuperação.
- Causa do problema após longo tempo ligado ainda depende de confirmação física. Compilação e teste prolongado ficam com o usuário.

## 2026-10-31 — Reset dos contadores NVS

- Adicionado `reset_counters` ao menu e ao processamento UDP/Serial; zera
  apenas os contadores de boot/watchdog e preserva as configurações Prometheus.
- Guia local e catálogo consolidado atualizados.

## 2026-10-18 — Comando de rotação do display

- Adicionado `rotacao:0|90|180|270` ao serial, UDP e help, com validação estrita dos quatro ângulos.
- Orientação absoluta aplicada a páginas, console, carinha e barra OTA; o acelerômetro acompanha a rotação efetiva.
- Removidos os resets de orientação na inicialização do sketch e antes dos comandos. O driver mantém 180° como padrão de boot; a escolha não é persistida após reiniciar.
- Documentação atualizada; compilação e teste físico ficam com o usuário.

## 2026-10-18 — Boca e expressões no protetor

- Adicionadas boca e sobrancelhas ciano, com expressões feliz, curiosa, surpresa e sonolenta.
- Expressões alternam a cada 3,5 a 6,5 segundos, sem repetir a anterior; pálpebras transitam suavemente.
- Movimento forte detectado pelo acelerômetro provoca surpresa por cerca de dois segundos, com intervalo mínimo de quatro segundos entre reações.
- Mantidos olhos ciano, olhar pelo acelerômetro, piscadas, orientação de 180° e desenho em buffer sem limpeza direta do LCD.
- Elementos distribuídos em 128 × 128 pixels, com boca abaixo dos olhos.
- Compilação e teste visual na placa ficam com o usuário.

## 2026-10-18 — Tela girada em 180°

- Aplicada a orientação de 180° desde a inicialização, preservada ao receber comandos e durante a OTA.
- Relógio, páginas, console e olhos seguem a mesma orientação. O mapeamento do acelerômetro já considera a rotação efetiva do display.
- Compilação e teste na placa ficam com o usuário.

## 2026-10-18 — Barra de progresso OTA

- Adicionada barra ciano com porcentagem na tela durante a gravação OTA.
- Atualização pelo buffer, sem limpar diretamente o LCD, em avanços de pelo menos 5 pontos percentuais e intervalo mínimo de 250 ms; 100% é exibido imediatamente.
- Indicados os estados de gravação, verificação, conclusão e código de erro.
- Mantidas as otimizações de prioridade e os logs de tempo/taxa no serial.
- Documentação atualizada; compilação e teste na placa ficam com o usuário.

## Revisão documental consolidada — 2026-10-18

- Documentado `name`, que retorna o nome configurado no dispositivo.
- Catálogo geral corrigido: RGB pelo LP5562 está implementado; somente SD
  e histórico de rede em cartão permanecem indisponíveis.
- Guia local detalha rejeição de entradas acima de 255 bytes, efeitos de
  `reset_wifi`, intervalo de alternância do LED e limites de OTA/core dump.
- Alterações de navegação e clima abaixo preservadas e identificadas pelo
  commit `5c56bcf` no [histórico ESP32](../../ESP32_Documents/changelog.md).
- Nenhuma alteração de firmware ou validação física nesta revisão documental.

## 2026-10-18 — Navegação e retorno ao relógio

- Clima configurado com intervalo automático fixo de 15 minutos, inclusive após falhas; primeira consulta ao conectar e atualização manual por `clima_sync` preservadas.
- Atualizados e consolidados LEIA-ME.md, comandos.md e changelog.md, incluindo navegação, temporizadores, olhos com acelerômetro, RGB e OTA.

- Cada toque curto avança uma página, com debounce e uma única troca ao soltar.
- Sequência do botão: relógio, NTP, rede, sistema, redes salvas, monitor e previsão. Páginas duplicadas e o aviso de SD são pulados.
- Após 20 segundos sem novo toque, retorna ao relógio. Segurar o botão não dispara esse retorno.
- Navegação por `tela:N` ou `tela_next` também inicia o prazo de 20 segundos; a numeração dos comandos foi preservada.
- Retorno automático não reinicia o contador do protetor de olhos, que continua com 60 segundos de inatividade.
- Compilação e teste na placa ficam com o usuário.

## 2026-10-17 — Adaptação inicial e melhorias

Registro das alterações realizadas nesta adaptação. Não foram atribuídos números de versão ou tags de release; o comando `version` continua usando a identificação calculada pelo firmware.

### Olhos controlados pelo acelerometro

- Habilitada a IMU interna pelo M5Unified, usando o acelerometro BMI270 para mover as iris ciano.
- Posicao neutra definida ao entrar no protetor; movimentos relativos a essa posicao controlam o olhar.
- Eixos adaptados a rotacao do display, com filtragem suave, zona morta contra pequenas vibracoes e limites para manter as iris nos olhos.
- Leitura limitada aos quadros do protetor (aproximadamente 30 Hz), sem tarefa adicional ou espera bloqueante.
- Piscadas preservadas; movimento automatico usado se a IMU estiver indisponivel ou sem leituras validas por mais de um segundo.
- Movimento nao encerra o protetor; botao ou comando continua retornando as paginas normais.
- API conferida na biblioteca instalada; compilacao e teste fisico ficam com o usuario.

### Prioridade para gravacao OTA

- Atendimento da OTA movido para antes do portal, display e animacoes no ciclo principal; convite e inicio processados em chamadas consecutivas.
- Wi-Fi sem economia de energia enquanto a OTA esta disponivel.
- CPU em 240 MHz e prioridade da tarefa elevada para pelo menos 2 durante a transferencia, com restauracao no fim ou erro.
- Novas consultas de clima impedidas durante a OTA. Uma consulta que ja estava em andamento pode concluir normalmente.
- Progresso no serial limitado a passos de aproximadamente 10%, inicialmente sem desenhos durante a gravacao; barra adicionada na revisao de 18/10/2026.
- Log final com bytes transferidos, duracao e taxa media em KiB/s. Integridade e troca de particao continuam a cargo de ArduinoOTA/Update.
- Ganho de velocidade ainda nao medido na placa; compilacao e gravacao ficam com o usuario.

### RGB e layout revisados

- Corrigida a interpretacao anterior do esquema: U5 NH-B2020RGBA-HF esta ligado as redes LED_R, LED_G e LED_B do LP5562 U4. O LED verde D1 pertence ao controlador de reset/download.
- Substituido o stub sem operacao por controle direto do LP5562 no barramento interno gerenciado pelo M5Unified, sem alterar o backlight W.
- Reativados led_on, led_off, led_breath, led_pisca e led_blink; adicionado led_rgb:R:G:B com validacao de 0 a 255.
- Comandos manuais preservam prioridade sobre o indicador automatico de Wi-Fi; iniciar blink encerra breathing.
- Ambos os olhos do protetor agora sao ciano.
- Fonte geral ampliada para 1.5; linhas extensas usam ajuste de tamanho e abreviacao em vez de quebrar palavras e numeros entre linhas.
- Relogio redesenhado com hora em destaque, data centralizada abaixo e IP separado. Pagina NTP tambem separa data e hora.
- Documentacao atualizada. Compilacao e teste visual na placa continuam com o usuario.

### Documentação

- Criado `comandos.md`, com sintaxe, parâmetros, limites, exemplos, páginas, botão e endereços dos serviços.
- Criado este histórico de alterações.
- Documentados os comandos herdados de SD que permanecem indisponíveis neste hardware.

### Protetor de tela

- Adicionados inicialmente olhos ciano e magenta; ambos foram alterados para ciano na revisão de layout.
- Movimento suave do olhar, com novos destinos e piscadas em intervalos variáveis.
- Ativação após 60 segundos sem interação e saída por botão ou comando.
- Animação no mesmo buffer da interface, sem limpar o LCD entre quadros.
- Serviços de rede e tarefas continuam sendo atendidos durante a animação.

### Legibilidade e atualização do display

- Aumentado o corpo do texto em 25% e os títulos em 20%.
- Ajustadas as quebras de linha do console e a apresentação das redes salvas para 128 × 128 pixels.
- Substituído o redesenho direto, que apagava a tela a cada atualização, por composição em um buffer RGB565 de 128 × 128 (32 KiB) e envio do quadro pronto.
- Escritas do brilho limitadas às mudanças efetivas de valor.
- Verificada a alocação do buffer, com mensagem no serial em caso de falta de memória.

### Portal sem logs de SD

- Removidos o menu e a página “Logs do SD”.
- Removidas as rotas `/logs` e `/api/logs` e o arquivo `LogPortal.h`.
- Retiradas a linha de SD da página de informações e as referências a SD no rodapé e no relatório do monitor.
- Preservados os serviços de informações, configuração Wi-Fi e Prometheus.

### Correção do modelo para AtomS3R

- Corrigida a adaptação inicial, que seguia o PDF do AtomS3 Lite, após confirmação de que a placa física é um AtomS3R.
- Reativado o display com M5Unified/M5GFX e controle do backlight pelo LP5562.
- Adicionadas páginas compactas de relógio, NTP, rede, sistema, redes salvas, monitor e previsão.
- Reativados comandos de navegação e brilho; pressão curta no botão volta a trocar páginas.
- Adaptado o RGB para o driver LP5562, removendo o uso de WS2812 no GPIO35 e a dependência de Adafruit NeoPixel. Posteriormente substituido por acesso direto aos canais RGB do LP5562 (ver acima).
- Corrigida a configuração para PSRAM OPI de 8 MB; flash de 8 MB e partições com OTA.
- Atualizada a identificação do dispositivo para AtomS3R, com hostname `AtomS3R-Gateway`.
- Mantido o SD indisponível, pois o AtomS3R não tem leitor integrado.

### Migração do ESP32C6 e dois núcleos

- Copiados e adaptados os serviços do ESP32C6 sem modificar o projeto de origem.
- Tarefa principal fixada no núcleo 1, com portal, comandos, display, botão, NTP e OTA.
- Consulta de clima em tarefa fixada no núcleo 0, com cache protegido entre núcleos.
- Adicionadas verificação de criação da tarefa principal e rejeição de configuração single-core.
- Botão frontal configurado no GPIO41.
- Preservados cinco perfis Wi-Fi em NVS, console USB a 115200 baud, UDP 4210, monitor de rede, clima e Prometheus.
- Adicionado `Compilar.ps1` com a configuração de compilação da placa.
- Corrigida a inclusão explícita de `SharedSpi.h` no arquivo principal após erro de compilação relatado pelo usuário.

### Validação e limites

- Documentação conferida com o processamento de comandos e os fontes atuais.
- Compilação e gravação ficam com o usuário, conforme solicitado; não há confirmação de compilação completa desta revisão nem de teste físico das alterações mais recentes.
- O usuário forneceu log de inicialização que confirma a tarefa principal no núcleo 1; as revisões posteriores de tela e animação ainda dependem de verificação na placa.
- Acelerometro integrado ao protetor; giroscopio e magnetometro ainda nao sao utilizados. Não há armazenamento de logs em microSD.
