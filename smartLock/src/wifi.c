#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define WIFI_SSID "iPhone"
#define WIFI_PASS "mwxn2izjehdzi"


// Event group
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;


// Wifi event handler
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
		
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    
	case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    
	case SYSTEM_EVENT_STA_DISCONNECTED:
		xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    
	default:
        break;
    }
   
	return ESP_OK;
}


// Main task
void main_task(void *pvParameter)
{
	// wait for connection
	printf("Main task: waiting for connection to the wifi network... ");
	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
	printf("connected!\n");
	
	// print the local IP address
	tcpip_adapter_ip_info_t ip_info;
	ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
	printf("IP Address:  %s\n", ip4addr_ntoa(&ip_info.ip));
	printf("Subnet mask: %s\n", ip4addr_ntoa(&ip_info.netmask));
	printf("Gateway:     %s\n", ip4addr_ntoa(&ip_info.gw));
	
	while(1) {
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}


// Main application
void wifi_set()
{
	// disable the default wifi logging
	esp_log_level_set("wifi", ESP_LOG_NONE);
	
	// initialize NVS
	ESP_ERROR_CHECK(nvs_flash_init());
	
	// create the event group to handle wifi events
	wifi_event_group = xEventGroupCreate();
		
	// initialize the tcp stack
	tcpip_adapter_init();

	// initialize the wifi event handler
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	
	// initialize the wifi stack in STAtion mode with config in RAM
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	// configure the wifi connection and start the interface
	wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
	printf("Connecting to %s\n", WIFI_SSID);
	
	// start the main task
    xTaskCreate(&main_task, "main_task", 2048, NULL, 5, NULL);
}

void wifi_Disconect(){
	esp_wifi_deinit();
}

/*include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "esp32_ifttt_maker.h"

#define ESP_INTR_FLAG_DEFAULT 0
#define WIFI_SSID "iPhone"
#define WIFI_PASS "mwxn2izjehdzi"
#define CONFIG_BUTTON_PIN 22


// Event group for inter-task communication
static EventGroupHandle_t event_group;
const int WIFI_CONNECTED_BIT = BIT0;
const int BUTTON_PRESSED_BIT = BIT1;



// button ISR
void IRAM_ATTR button_isr_handler(void* arg) {
	
	xEventGroupSetBitsFromISR(event_group, BUTTON_PRESSED_BIT, NULL);
}

// Wifi event handler
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
		
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    
	case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(event_group, WIFI_CONNECTED_BIT);
        break;
    
	case SYSTEM_EVENT_STA_DISCONNECTED:
		xEventGroupClearBits(event_group, WIFI_CONNECTED_BIT);
        break;
    
	default:
        break;
    }  
	return ESP_OK;
}


// Main task
void main_task(void *pvParameter)
{
	// wait for connection
	printf("Waiting for connection to the wifi network...\n ");
	xEventGroupWaitBits(event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
	printf("Connected\n\n");
	
	// loop waiting for button press
	for(;;) {
		
		int ret;
		
		xEventGroupWaitBits(event_group, BUTTON_PRESSED_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
		printf("Button pressed, sending events...\n");
		
		printf("Sending HELLO event without values... ");
		fflush(stdout);
		if((ret = ifttt_maker_trigger("hello")) == IFTTT_MAKER_OK) printf("success :)\n");
		else printf("error (%d) :(\n", ret);
		
		printf("Sending ALARM event with a value... ");
		fflush(stdout);
		char* value[] = {"door is open"};		
		if((ret = ifttt_maker_trigger_values("alarm", value, 1)) == IFTTT_MAKER_OK) printf("success :)\n");
		else printf("error (%d) :(\n", ret);
	}
}


// initialize the button
void button_setup() {
	
	gpio_pad_select_gpio(CONFIG_BUTTON_PIN);
	gpio_set_direction(CONFIG_BUTTON_PIN, GPIO_MODE_INPUT);
	gpio_set_intr_type(CONFIG_BUTTON_PIN, GPIO_INTR_NEGEDGE);
	
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(CONFIG_BUTTON_PIN, button_isr_handler, NULL);	
}

// setup and start the wifi connection
void wifi_setup() {
	
	event_group = xEventGroupCreate();
		
	tcpip_adapter_init();

	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}


// Main application
void wifi_main()
{
	// disable the default wifi logging
	esp_log_level_set("wifi", ESP_LOG_NONE);

	nvs_flash_init();
	wifi_setup();
	button_setup();
	ifttt_maker_init("key");
    xTaskCreate(&main_task, "main_task", 20000, NULL, 5, NULL);
}

void wifi_Disconect(){




}*/