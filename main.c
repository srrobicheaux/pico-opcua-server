#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "FreeRTOS.h"
#include "task.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/netif.h"
#include "lwip/ip4_addr.h"
#include "lwip/sockets.h"
#include "lwip/apps/sntp.h"
#include "secrets.h"
#include "open62541/server.h"
#include "open62541/server_config_default.h"

/* Modular Headers */
#include "hardware.h"
#include "opc_handlers.h"

#define WIFI_TIMEOUT_MS 30000

#define STACK_OPC       5120
#define STACK_WIFI      2048
#define STACK_START     2048

static volatile int g_link = 0;

static void opc_task(void *pv)
{
    (void)pv;
    printf("opc_task:\t OPC Server Starting...");
    fflush(stdout);

    UA_Server *server = UA_Server_new();
    if (!server) {
        printf("UA_Server_new failed\n");
        vTaskDelete(NULL);
        return;
    }

    UA_ServerConfig *config = UA_Server_getConfig(server);
    UA_ServerConfig_setMinimal(config, 4840, NULL);

    config->maxSecureChannels = 10;
    config->maxSessions = 10;
    config->maxSessionTimeout = 10000;

    UA_StatusCode rc = UA_Server_run_startup(server);
    if (rc != UA_STATUSCODE_GOOD) {
        printf("run_startup failed: 0x%08x\n", (unsigned)rc);
        UA_Server_delete(server);
        vTaskDelete(NULL);
        return;
    }

    add_variables(server);
    printf(" online.\n");

    for (;;) {
        UA_Server_run_iterate(server, false);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void sntp_task(void *pvParameters)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    time_t now = 0;
    while (now < 1600000000) { 
        vTaskDelay(pdMS_TO_TICKS(1000));
        now = time(NULL);
    }

    printf("SNTP synced to (UTC): %s", ctime(&now));
    vTaskDelete(NULL);
}

static void wifi_task(void *pv)
{
    (void)pv;

    if (cyw43_arch_init()) {
        printf("cyw43_arch_init FAILED\n");
        vTaskDelete(NULL);
        return;
    }
    
    hardware_init();

    cyw43_arch_enable_sta_mode();
    netif_set_hostname(&cyw43_state.netif[CYW43_ITF_STA], "opcserver");

    int st = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID, WIFI_PASSWORD,
        CYW43_AUTH_WPA3_WPA2_AES_PSK,
        WIFI_TIMEOUT_MS);

    printf("\033[2J\033[H"); 
    printf("===\tPico Universal Access\t===\n\tWiFi\n\tSNTP\n\tOPC UA\n\n");
    printf("wifi_task:\t Connecting to '%s' ... ", WIFI_SSID);
    fflush(stdout);

    if (st != 0) {
        st = cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID, WIFI_PASSWORD,
            CYW43_AUTH_WPA2_MIXED_PSK,
            WIFI_TIMEOUT_MS);
    }

    g_link = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    const char *ip = ip4addr_ntoa(netif_ip4_addr(&cyw43_state.netif[CYW43_ITF_STA]));

    printf("(result=%d link=%d ip=%s) \n", st, g_link, ip);

    if (st != 0 && g_link != CYW43_LINK_UP) {
        printf("Wi-Fi FAILED — OPC not started\n");
    } else {
        printf("wifi_task:\t Wi-Fi OK. \nsntp_task:\t Setting time via SNTP ... \n");
        
        xTaskCreate(sntp_task, "SNTP_Task", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);

        time_t now = 0;
        while (now < 1600000000) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
            now = time(NULL);
        }

        xTaskCreate(opc_task, "opc", STACK_OPC, NULL, 2, NULL);
    }

    for (;;) {
        g_link = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(g_link == CYW43_LINK_UP ? 800 : 100));
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(g_link == CYW43_LINK_UP ? 800 : 100));
    }
}

static void startup_task(void *pv)
{
    (void)pv;
    xTaskCreate(wifi_task, "wifi", STACK_WIFI, NULL, 1, NULL);
    vTaskDelete(NULL);
}

int main(void)
{
    stdio_init_all();
    sleep_ms(50); 

    xTaskCreate(startup_task, "start", STACK_START, NULL, configMAX_PRIORITIES - 1, NULL);
    vTaskStartScheduler();
    
    for (;;); 
}