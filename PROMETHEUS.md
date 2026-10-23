# Métricas do AtomS3R no Prometheus

Atualizado em 19/10/2026.

## Monitoramento UDP

As quatro séries abaixo são exportadas mesmo antes do primeiro comando, com o rótulo `device` do exportador. `instance` e `job` são acrescentados pelo Prometheus.

| Métrica | Tipo | Significado |
| --- | --- | --- |
| `esp_udp_listener_up` | gauge | 1 quando a última abertura local da porta UDP 4210 teve sucesso e o socket não foi fechado pelo firmware; 0 quando está fechado ou a abertura falhou. Não comprova conectividade nem recebimento no cliente. |
| `esp_udp_packets_received_total` | counter | Datagramas detectados pelo receptor, inclusive entradas recusadas por exceder 255 bytes. |
| `esp_udp_replies_sent_total` | counter | Respostas completas aceitas pelo envio local. Uma resposta dividida em vários blocos conta apenas uma vez, após sucesso de todos os blocos. |
| `esp_udp_send_errors_total` | counter | Tentativas locais de envio que falharam na abertura do pacote, escrita ou envio. Uma resposta pode acumular duas falhas se a repetição também falhar. |

Os contadores usam 64 bits, ficam em RAM e zeram ao reiniciar a placa. Reabrir o socket ou reconectar o Wi-Fi não zera os valores. Respostas vazias, sem destino ou interrompidas por falha não incrementam o contador de respostas completas. Envio local bem-sucedido não garante entrega, pois UDP não tem confirmação do destinatário.

Exemplos de consultas:

```promql
esp_udp_listener_up
rate(esp_udp_packets_received_total[5m])
rate(esp_udp_replies_sent_total[5m])
increase(esp_udp_send_errors_total[5m])
```

Para conferir, colete as séries, envie `alive` por UDP e colete novamente: RX e respostas completas devem aumentar em uma unidade, salvo outro tráfego concorrente. Erros de envio só aumentam diante de falha real do envio local. Testes na placa e no servidor ficam com o usuário.

## Clima de Porto Alegre

O endpoint `/metrics` exporta os dados do cache do Open-Meteo, atualizado a cada 15 minutos. O scrape não consulta a API de clima. Use `clima_sync` para solicitar uma atualização antecipada.

## Séries

- `sensor_temperature_celsius{source="open_meteo",location="porto_alegre"}`: temperatura do ar em °C.
- `sensor_humidity_percent{source="open_meteo",location="porto_alegre"}`: umidade relativa do ar em %, de 0 a 100; não é probabilidade de chuva.
- `esp_info{model="M5Stack-AtomS3R"}`: identificação usada pelo filtro do dashboard.

Todas também recebem o rótulo `device` já usado pelo exportador. `instance` e `job` são acrescentados pelo Prometheus ao coletar o endpoint, permitindo a associação entre séries da mesma placa. O seletor de umidade pode omitir `source`, como nas consultas abaixo.

## Consultas do dashboard

Temperatura:

```promql
sensor_temperature_celsius{source="open_meteo",location="porto_alegre"}
and on(instance, job) esp_info{model="$device"}
```

Umidade:

```promql
sensor_humidity_percent{location="porto_alegre"}
and on(instance, job) esp_info{model="$device"}
```

A variável `$device` deve selecionar o valor do rótulo **model**, que nesta placa é `M5Stack-AtomS3R`, e não o MAC do rótulo `device`. Uma variável de dashboard pode usar `label_values(esp_info, model)`.

## Disponibilidade e validação

- A série de temperatura aparece após a primeira leitura válida de temperatura e condição do tempo.
- A série de umidade aparece após a primeira leitura válida de `relative_humidity_2m`. Valores ausentes, não numéricos ou fora de 0 a 100 não substituem o cache válido anterior.
- Antes da primeira leitura válida, a série correspondente é omitida, evitando publicar zero fictício.
- Após falha da consulta, os últimos valores válidos permanecem no cache. `esp_weather_data_age_seconds` indica a idade da última atualização válida de temperatura/condição; não é um relógio independente da umidade.
- `esp_cpu_temperature_celsius` continua sendo a temperatura interna do chip, separada do clima.
- Após instalar o firmware, solicite `clima_sync`, aguarde a consulta e verifique as duas séries em `http://IP_DA_PLACA:9120/metrics` (ou na porta configurada). Depois aguarde o próximo scrape no Prometheus.
- Integração conferida nos fontes; compilação, coleta real e consultas no dashboard ainda precisam ser verificadas pelo usuário.

Referência da API: [Open-Meteo](https://open-meteo.com/en/docs), campos `temperature_2m` e `relative_humidity_2m` de `current`.
