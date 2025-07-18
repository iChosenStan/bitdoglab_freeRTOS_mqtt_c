#include "headers/wifi_conn.h"         // Cabeçalho com a declaração da função de conexão Wi-Fi
#include "pico/cyw43_arch.h"           // Biblioteca para controle do chip Wi-Fi CYW43 no Raspberry Pi Pico W
#include <stdio.h>                     // Biblioteca padrão de entrada/saída (para usar printf)

/**
 * Função: connect_to_wifi
 * Objetivo: Inicializar o chip Wi-Fi da Pico W e conectar a uma rede usando SSID e senha fornecidos.
 */
// wifi_conn.c
bool connect_to_wifi(const char* ssid, const char* password) {
    printf("[WiFi] Inicializando modo STA...\n");
    if (cyw43_arch_init()) {
        printf("[WiFi] Falha ao inicializar o driver WiFi\n");
        return false;
    }

    cyw43_arch_enable_sta_mode();
    printf("[WiFi] Modo STA habilitado\n");

    int result = cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000);
    printf("[WiFi] Resultado da conexão: %d\n", result);

    if (result == 0) {
        printf("[WiFi] Conectado com sucesso\n");
        return true;
    } else {
        printf("[WiFi] Falha ao conectar\n");
        return false;
    }
}