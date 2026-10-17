# Histórico de alterações — AtomS3R Gateway

## 2026-10-17 — Estado atual

Registro das alterações realizadas nesta adaptação. Não foram atribuídos números de versão ou tags de release; o comando `version` continua usando a identificação calculada pelo firmware.

### Prioridade para gravacao OTA

- Atendimento da OTA movido para antes do portal, display e animacoes no ciclo principal; convite e inicio processados em chamadas consecutivas.
- Wi-Fi sem economia de energia enquanto a OTA esta disponivel.
- CPU em 240 MHz e prioridade da tarefa elevada para pelo menos 2 durante a transferencia, com restauracao no fim ou erro.
- Novas consultas de clima impedidas durante a OTA. Uma consulta que ja estava em andamento pode concluir normalmente.
- Progresso no serial limitado a passos de aproximadamente 10%, sem desenhos de progresso durante a gravacao.
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

- Adicionados dois olhos coloridos: ciano e magenta.
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
- IMU e magnetômetro não foram integrados. Não há armazenamento de logs em microSD.
