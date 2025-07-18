/*
Autor: Stanley de Oliveira Souza
Descrição: FreeRTOS (com fila, WiFi e MQTT)
*/
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "hardware/adc.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>
#include <string.h>
#include "headers/led.h"
#include "headers/wifi_conn.h"
#include "headers/mqtt_comm.h"

// Configurações do WiFi
#define WIFI_SSID "NOME_DA_SUA_REDE_WIFI"
#define WIFI_PASSWORD "SUA_SENHA_DA_REDE"

//Configurações do broker MQTT
#define NOME_DO_DISPOSITIVO "bitdog1ab"
#define IP_DO_BROKER "192.168.0.100"
#define USER_DO_BROKER "SEU_USUARIO_DO_BROKER_MQTT"
#define SENHA_DO_BROKER "SUA_SENHA_DO_BROKER_MQTT"
#define CANAL_DO_BROKER "bitdoglab/mcu/temperatura"

// Configuração do sensor interno
#define internal_sensor_adc 4

// Tamanho da fila
#define QUEUE_SIZE 10

// Variáveis globais
float g_tempC = 0.0f;
bool wifiConnected = false;

QueueHandle_t tempQueue;

static float read_internal_temperature_celsius(void) {
    adc_set_temp_sensor_enabled(true);
    adc_select_input(internal_sensor_adc);
    const float conversionFactor = 3.3f / (1 << 12);
    float adc = (float)adc_read() * conversionFactor;
    float temp_c = 27.0f - (adc - 0.706f) / 0.001721f;
    return temp_c;
}

// Task para medir temperatura e enfileirar
void vTemperatureTask(void *pvParameters) {
    (void) pvParameters;

    const uint8_t NUM_SAMPLES = 30;
    float acumulado = 0.0f;
    uint8_t amostras = 0;

    for (;;) {
        float temp = read_internal_temperature_celsius();
        printf("[Temp Task] Leitura: %.2f°C\n", temp);

        acumulado += temp;
        amostras++;

        if (amostras == NUM_SAMPLES) {
            float media = acumulado / NUM_SAMPLES;
            g_tempC = media;
            printf("[Temp Task] Média calculada: %.2f\n", media);

            if (xQueueSend(tempQueue, &media, 0) == pdPASS) {
                printf("[Temp Task] Média enfileirada com sucesso.\n");
            } else {
                printf("[Temp Task] Erro ao enfileirar média!\n");
            }

            acumulado = 0.0f;
            amostras = 0;
        }
        gpio_put(LED_G, 1);
        vTaskDelay(pdMS_TO_TICKS(50));
        gpio_put(LED_G, 0);
        vTaskDelay(pdMS_TO_TICKS(950)); // A cada 1s
    }
}

// Task de WiFi
void vWifiTask(void *pvParameters) {
    (void) pvParameters;

    while (true) {
        if (!wifiConnected) {
            printf("Conectando ao WiFi...\n");
            if (connect_to_wifi(WIFI_SSID, WIFI_PASSWORD)) {
                printf("Conexão WiFi bem-sucedida!\n");
                wifiConnected = true;
                mqtt_setup(NOME_DO_DISPOSITIVO, IP_DO_BROKER, USER_DO_BROKER, SENHA_DO_BROKER);
            } else {
                printf("Falha na conexão. Tentando novamente...\n");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10000)); // Verifica a cada 10s
    }
}

// Task de envio MQTT
void vMqttTask(void *pvParameters) {
    (void) pvParameters;

    float temp = 0.0f;
    char mensagem[32];

    for (;;) {
        if (wifiConnected) {
            printf("[MQTT Task] Verificando fila...\n");
            
            if (xQueueReceive(tempQueue, &temp, pdMS_TO_TICKS(1000))) {
                printf("[MQTT Task] Dado recebido da fila: %.2f\n", temp);
                snprintf(mensagem, sizeof(mensagem), "%.2f", temp);
                printf("Publicando via MQTT: %s\n", mensagem);

                bool sucesso = mqtt_comm_publish(CANAL_DO_BROKER, mensagem, strlen(mensagem));
                if (sucesso) {
                    printf("[MQTT Task] Publicado com sucesso: %.2f\n", temp);
                } else {
                    printf("[MQTT Task] Falha ao publicar. Reenfileirando...\n");
                    if (!xQueueSendToFront(tempQueue, &temp, 0)) {
                        printf("[MQTT Task] ERRO: Fila cheia! Dado %.2f perdido.\n", temp);
                    }
                }

            } // else: fila vazia, nada a fazer
            printf("[MQTT Task] Fila vazia, aguardando novos dados...\n");
        }
        vTaskDelay(pdMS_TO_TICKS(20000));
    }
}

void main() {
    stdio_init_all();
    adc_init();
    setup_led();
    // Criação da fila
    tempQueue = xQueueCreate(QUEUE_SIZE, sizeof(float));
    if (tempQueue == NULL) {
        printf("Erro ao criar a fila de temperaturas.\n");
        while (true);
    }
    printf("Criado a fila de temperaturas.\n");
    // Criação das tasks
    xTaskCreate(vWifiTask, "Wifi Task", 256, NULL, 2, NULL);
    xTaskCreate(vMqttTask, "MQTT Task", 256, NULL, 2, NULL);
    xTaskCreate(vTemperatureTask, "Temp Task", 256, NULL, 1, NULL);

    // Inicia o agendador
    vTaskStartScheduler();
}
