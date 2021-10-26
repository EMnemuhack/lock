/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

//LED_API時に追加
#include "driver/ledc.h"
#include "freertos/queue.h"
#include "esp_log.h"


//定義
#include "env.h"

//チャタリング対策
#include "button.c"

//サーボモータ動作モジュール
#include "servo.c"

#include "example_ble_sec_gatts_demo.c"

//GPIO取得の時に追加
/*#define GPIO_INPUT_IO_0     16
#define GPIO_INPUT_IO_1     17
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1)) //unsignd long long
#define ESP_INTR_FLAG_DEFAULT 0*/

//void set_gpio(void);
//void button_task(void*);
//void IRAM_ATTR button_isr_handler(void *);

// ハンドラ型の変数？
TaskHandle_t ISR = NULL;

void app_main()
{

	button_event_t ev;
	QueueHandle_t button_events = button_init(PIN_BIT(CONFIG_INPUT_PIN1) | PIN_BIT(CONFIG_INPUT_PIN2));
	ble_init();

	int cnt = 0;
	//bool led_status = false;

	while (true) {
		if (xQueueReceive(button_events, &ev, 1000/portTICK_PERIOD_MS)) {

			if ((ev.pin == CONFIG_INPUT_PIN1) && (ev.event == BUTTON_DOWN)) {
				//LED点灯
				//gpio_set_level(CONFIG_OUTPUT_PIN, led_status);

				//led_status = !(led_status);
				cnt++;
				printf("ボタンが%d回押されました\n",cnt);
				pwm_servo();
				//xTaskCreate(&task_servoSweep, "task_servoSweep", 2048, NULL, 9,NULL);
				printf("servo sweep task  started\n");
			}
			if ((ev.pin == CONFIG_INPUT_PIN2) && (ev.event == BUTTON_DOWN)) {
				// ...
			}
		}
	}
}