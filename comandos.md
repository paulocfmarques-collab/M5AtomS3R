# Comandos do AtomS3R Gateway

Documentação do firmware atual, revisada em 17/10/2026.

## Como enviar

- **USB serial:** 115200 baud. Envie um comando por linha, terminando com Enter (LF, CR ou CR+LF).
- **UDP:** envie o comando ao IP da placa, porta **4210**. A resposta volta ao endereço e à porta do remetente.
- Limite de entrada: **255 bytes** por comando. No serial, linhas maiores são descartadas.
- O nome do comando é convertido para minúsculas. Os parâmetros preservam maiúsculas, minúsculas e espaços.
- Use dois-pontos para separar parâmetros. Não acrescente aspas aos exemplos.

Exemplos para o Monitor Serial:

```text
help
info
wifi_list
brilho:80
tela:0
clima_sync
```

## Sistema e diagnóstico

| Comando | Função |
| --- | --- |
| `help` | Lista os comandos do firmware. A lista herdada ainda inclui comandos SD, indisponíveis nesta placa. |
| `alive` | Responde `alive`. |
| `info` | Resumo de firmware, rede, memória, data/hora, uptime e reset. |
| `status` | Estado de Wi-Fi, NTP e heap livre. |
| `version` | Versão calculada pelo firmware. |
| `build` | Data e hora da compilação. |
| `uptime` | Tempo desde a inicialização em dias, horas, minutos e segundos. |
| `reason` | Motivo do último reset e uptime atual. |
| `init` | Consulta o motivo do reset; não reinicializa o dispositivo. |
| `crash_info` | Consulta o diagnóstico de crash, se core dump estiver habilitado na compilação. Sem SD, não arquiva o relatório e preserva o registro interno. |
| `cpu` | Modelo, revisão, número de núcleos e frequência da CPU. |
| `chip_info` | Informações do chip e versão do SDK. |
| `temp` | Temperatura interna do chip; não é a temperatura ambiente. |
| `ram` | Estatísticas de heap e uso de RAM. |
| `heap` | Heap livre em bytes e KB. |
| `heap_min` | Menor heap livre registrado desde o boot. |
| `flash` | Informações de flash e espaço do firmware. |
| `flash_info` | Tamanho da flash, firmware e espaço para OTA. |
| `health` | Resumo de Wi-Fi, NTP, RAM e atualização do clima. |
| `selftest` | Verificações de Wi-Fi, NTP, RAM, clima e frequência da CPU. |
| `reboot` | Reinicia a placa após aproximadamente 500 ms. |

`health` e `selftest` consideram RAM suficiente a partir de 40 KB de heap livre e clima recente até 30 minutos. Não são testes físicos completos dos periféricos.

## Wi-Fi e rede

| Comando | Função |
| --- | --- |
| `wifi_add:SSID:senha` | Salva ou atualiza uma rede, com confirmação de leitura em NVS. Aplicada na próxima reconexão. |
| `wifi_list` | Lista os SSIDs salvos, sem as senhas. |
| `wifi_status` | Estado da conexão, SSID, IP e RSSI. |
| `net_info` | SSID, IP, gateway, máscara, DNS e RSSI. |
| `net_monitor` | Estatísticas do monitor de rede e ping ao gateway. |
| `mac` | Endereço MAC da interface Wi-Fi. |
| `ip` | IP da conexão Wi-Fi. |
| `ssid` | Nome da rede conectada. |
| `rssi` | Intensidade do sinal em dBm. |
| `channel` | Canal Wi-Fi atual. |
| `reset_wifi` | Apaga os perfis Wi-Fi salvos e reinicia. |

Há até cinco perfis de rede, com atualização circular. Para rede aberta, mantenha o último dois-pontos:

```text
wifi_add:MinhaRede:MinhaSenha
wifi_add:RedeAberta:
```

O primeiro dois-pontos após `wifi_add:` separa SSID e senha: esse formato não permite dois-pontos no SSID. A senha pode conter dois-pontos. Espaços fazem parte das credenciais.

Ping ao gateway mede a resposta do roteador; não comprova acesso à Internet.

## Tela e brilho

| Comando | Função |
| --- | --- |
| `tela:N` | Seleciona uma página, de `0` a `9`. |
| `tela_next` | Avança à próxima página. |
| `brilho:P` | Define o brilho de `0` a `100` por cento. |
| `tela_on` | Liga a iluminação com brilho de 80%. |
| `tela_off` | Define brilho zero; os serviços de rede continuam ativos. |

| Página | Conteúdo |
| --- | --- |
| 0 | Relógio, IP e temperatura do clima |
| 1 e 2 | NTP, data/hora, fuso e DST |
| 3 e 4 | Informações de rede |
| 5 | Sistema, heap, PSRAM e uptime |
| 6 | Cinco redes salvas |
| 7 | Aviso de ausência de leitor microSD |
| 8 | Monitor de rede |
| 9 | Previsão de Porto Alegre |

As páginas 1/2 e 3/4 usam o mesmo conteúdo nesta adaptação. O console ocupa a tela por cerca de 10 segundos após um comando; comandos de navegação selecionam a página imediatamente.

Após **60 segundos sem interação**, aparecem dois olhos ciano com movimento suave e piscadas. Botão ou comando sai do protetor. Após dois minutos, o brilho cai para no máximo 20%; uma interação restaura o valor configurado. Se o brilho configurado for zero, use `tela_on` ou `brilho:P` para voltar a iluminar.

## Data e hora

| Comando | Função |
| --- | --- |
| `time` | Consulta a hora local. |
| `date` | Consulta a data local. |
| `ntp_status` | Estado de sincronização e data/hora quando disponíveis. |
| `set_fuso:X` | Define o fuso UTC inteiro entre `-12` e `14`. Exemplo: `set_fuso:-3`. |
| `dst_on` | Ativa o ajuste de horário de verão. |
| `dst_off` | Desativa o ajuste de horário de verão. |
| `set_time:AAAA-MM-DD HH:MM:SS` | Ajusta manualmente a data e hora local. |

Exemplo: `set_time:2026-10-17 14:30:00`. O fuso e o ajuste de horário de verão ficam salvos em NVS. O NTP pode atualizar posteriormente a hora ajustada manualmente.

## LED RGB

O RGB U5 (NH-B2020RGBA-HF) e controlado pelo LP5562 U4 no endereco I2C 0x30, conforme o esquema oficial AtomS3R v0.4.1. O driver acessa apenas os canais R/G/B, preservando o canal W da iluminacao da tela. O LED verde D1 pertence ao circuito de reset/download e nao e comandado pelo firmware.

| Comando | Funcao |
| --- | --- |
| `led_on` | Acende o RGB em branco e encerra animacoes. |
| `led_off` | Apaga o RGB e encerra animacoes. |
| `led_rgb:R:G:B` | Define a cor; cada componente deve ser inteiro de 0 a 255. |
| `led_breath` | Pulsacao suave em ciano, sem bloquear o programa. |
| `led_pisca:P:I` | Pisca P vezes (1 a 100), com intervalo de 1 a 5000 ms entre alternancias. |
| `led_blink:I` | Pisca continuamente, intervalo de 50 a 60000 ms entre alternancias. |

Exemplos: `led_rgb:255:0:0` (vermelho), `led_rgb:0:255:255` (ciano), `led_pisca:5:300`. Sem parametros, `led_pisca` usa 10 pulsos/250 ms e `led_blink` usa 250 ms. `led_on`, `led_off` e `led_rgb` interrompem os efeitos. Comandos manuais tem prioridade sobre a sinalizacao automatica de Wi-Fi. Segurar o botao pode sinalizar a acao de reinicio/apagar redes no RGB.

O boot informa `[RGB] LP5562 pronto (RGB independente do backlight)`. Se houver erro de I2C, os comandos informam indisponibilidade. A resposta do controlador nao substitui a verificacao visual do LED na placa.

## Clima

| Comando | Função |
| --- | --- |
| `clima` | Temperatura, condição e código WMO do cache atual. |
| `clima_age` | Tempo desde a última atualização válida. |
| `previsao` | Data, mínima, máxima, chance de chuva e idade da previsão diária. |
| `clima_sync` | Solicita uma consulta em segundo plano, se houver Wi-Fi. |

Os dados são do Open-Meteo para **Porto Alegre**, herdados do projeto de origem. A atualização normal ocorre a cada 15 minutos; após falha, a próxima tentativa ocorre em cerca de um minuto. `clima_sync` confirma a solicitação, não a conclusão. Consulte `clima_age` e `previsao` para verificar a atualização; `clima` isoladamente não comprova que o cache já foi sincronizado.

## SD indisponível

O AtomS3R não tem leitor microSD integrado. Todos os comandos `sd_*` retornam indisponibilidade, inclusive os ainda listados no `help`. `net_history` também retorna SD indisponível. Não há gravação de logs no cartão nem página web de logs.

## Botão e endereços

- Pressão curta no botão frontal: próxima página e saída do protetor.
- Segurar por 3 segundos e soltar: reinicia.
- Segurar por 10 segundos e soltar: apaga os perfis Wi-Fi e reinicia.
- AP de configuração: **ATOMS3_CONFIG**; página Wi-Fi: `http://192.168.4.1/wifi`.
- Portal conectado: `http://IP_DA_PLACA/`, com `/info`, `/wifi` e `/prometheus` na porta 80.
- Métricas: `http://IP_DA_PLACA:9120/metrics` por padrão; a porta pode ser alterada no portal.
- Hostname OTA: **AtomS3R-Gateway**.

Referência de configuração e bibliotecas: [LEIA-ME.md](LEIA-ME.md).

## Gravacao OTA

Selecione a porta de rede AtomS3R-Gateway na Arduino IDE para atualizar o firmware. A primeira instalacao desta otimizacao ainda usa o firmware anterior; as otimizacoes passam a valer no boot seguinte.

A OTA recebe atendimento antes do portal e das animacoes. Durante a transferencia, a biblioteca atende exclusivamente a gravacao na tarefa principal: tela, botao e portal ficam temporariamente sem atualizacao. Novas consultas de clima sao impedidas; uma consulta em andamento pode terminar. O Wi-Fi fica sem economia de energia, a CPU usa 240 MHz e a prioridade da tarefa aumenta temporariamente. Frequencia e prioridade retornam aos valores anteriores em caso de erro; o Wi-Fi continua sem economia de energia para permitir nova tentativa rapida.

O serial informa progresso a cada aproximadamente 10% e termina com bytes, segundos e KiB/s. Compare esse registro entre gravacoes para medir a taxa real. O tempo depende do sinal Wi-Fi, roteador, computador e tamanho do firmware; nao foi medida uma aceleracao na placa nesta revisao.
