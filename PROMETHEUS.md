# Clima de Porto Alegre no Prometheus

Atualizado em 19/10/2026.

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
