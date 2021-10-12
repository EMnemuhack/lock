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

//チャタリング対策
#include <button.c>

//GPIO取得の時に追加
/*#define GPIO_INPUT_IO_0     16
#define GPIO_INPUT_IO_1     17
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1)) //unsignd long long
#define ESP_INTR_FLAG_DEFAULT 0*/

#define CONFIG_INPUT_PIN1 16
#define CONFIG_INPUT_PIN2 17

#define CONFIG_OUTPUT_PIN 23
#define ESP_INTR_FLAG_DEFAULT 0


#define BLINK_GPIO CONFIG_BLINK_GPIO

//プロトタイプ宣言
void task_servoSweep(void);
//void set_gpio(void);
//void button_task(void*);
//void IRAM_ATTR button_isr_handler(void *);

// ハンドラ型の変数？
TaskHandle_t ISR = NULL;

static char tag[] = "servo1";

void app_main()
{

	button_event_t ev;
	QueueHandle_t button_events = button_init(PIN_BIT(CONFIG_INPUT_PIN1) | PIN_BIT(CONFIG_INPUT_PIN2));

	//使用するGPIO選択
	//gpio_pad_select_gpio(CONFIG_INPUT_PIN1);
	//gpio_pad_select_gpio(CONFIG_OUTPUT_PIN);

	//GPIOの向きを選択
	//gpio_set_direction(CONFIG_INPUT_PIN1, GPIO_MODE_INPUT);
	//gpio_set_direction(CONFIG_OUTPUT_PIN, GPIO_MODE_OUTPUT);


	//GPIO割り込みタイプ選択
	//gpio_set_intr_type(CONFIG_INPUT_PIN, GPIO_INTR_NEGEDGE);	//立ち下がりで割り込み

	//GPIO割り込みハンドラー インストール　PIN毎の割り込みが出来るようになる
	//gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

	//対応するGPIOピンのISRハンドラーを追加	2番目の引数は　isr_handler：対応するGPIO番号のISRハンドラー関数。
	//gpio_isr_handler_add(CONFIG_INPUT_PIN, button_isr_handler, NULL);

	//統計タスクを作成して開始します button_taskへ飛ぶ
	//xTaskCreate(button_task, "button_task", 4096, NULL , 10,&ISR);

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
				
				xTaskCreate(&task_servoSweep, "task_servoSweep", 2048, NULL, 5,NULL);
				printf("servo sweep task  started\n");
			}
			if ((ev.pin == CONFIG_INPUT_PIN2) && (ev.event == BUTTON_DOWN)) {
				// ...
			}
		}
	}
}
/*/ボタンが押されたときに呼び出される割り込みサービスルーチン　IRAM_ATTRはメモリの領域指定(固定)してる　　必要な理由はISRで呼び出せる領域が決められているから
void IRAM_ATTR button_isr_handler(void* arg){
	xTaskResumeFromISR(ISR);
// portYIELD_FROM_ISR();
}*/

//サーボモータ制御
void task_servoSweep(void) {
	int bitSize         = 15;
	//int minValue        = 625;  // micro seconds (uS)
	//int maxValue        = 2500; // micro seconds (uS)

	int minValue        = 500;  // micro seconds (uS)
	int maxValue        = 2500; // micro seconds (uS)
	int sweepDuration   = 1000; // milliseconds (ms)	終了 時間
	int duty            = (1<<bitSize) * minValue / 20000;
	int direction       = 1; // 1 = up, -1 = down
	int valueChangeRate = 20; // msecs	変化率

	ESP_LOGD(tag, ">> task_servo1");


	//タイマーの設定
	ledc_timer_config_t timer_conf;
	timer_conf.bit_num    = LEDC_TIMER_15_BIT;		//タイマーのbit
	timer_conf.freq_hz    = 50;						//周波数
	timer_conf.speed_mode = LEDC_HIGH_SPEED_MODE;	//モード
	timer_conf.timer_num  = LEDC_TIMER_0;			//使用するタイマー？
	ledc_timer_config(&timer_conf);

	//PWMの設定
	ledc_channel_config_t ledc_conf;
	ledc_conf.channel    = LEDC_CHANNEL_0;
	ledc_conf.duty       = duty;
	ledc_conf.gpio_num   = CONFIG_OUTPUT_PIN;
	ledc_conf.intr_type  = LEDC_INTR_DISABLE;
	ledc_conf.speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_conf.timer_sel  = LEDC_TIMER_0;
	ledc_channel_config(&ledc_conf);

	int changesPerSweep = sweepDuration / valueChangeRate;	// 1000 / 20
	int changeDelta = (maxValue-minValue) / changesPerSweep;// 変化率(2500 - 625) / (1000 / 20)
	int i;
	ESP_LOGD(tag, "sweepDuration: %d seconds", sweepDuration);
	ESP_LOGD(tag, "changesPerSweep: %d", changesPerSweep);
	ESP_LOGD(tag, "changeDelta: %d", changeDelta);
	ESP_LOGD(tag, "valueChangeRate: %d", valueChangeRate);

	while(gpio_get_level(CONFIG_INPUT_PIN1) == 0){
		for (i=0; i<changesPerSweep; i++) {
			if (direction > 0) {
				duty += changeDelta;
				printf("プラス\n");
			} else {
				duty -= changeDelta;
				printf("マイナス\n");
			}
			printf("duty[%d]\n",duty);
			printf("現在のiは[%d]\n",i);
			ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
			ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
			vTaskDelay(valueChangeRate/portTICK_PERIOD_MS);
		}
		direction = -direction;	//＋側へ回転　-側へ回転の切り替え
		ESP_LOGD(tag, "Direction now %d", direction);
	} // End loop forever
	vTaskDelete(NULL);
}

/*
void set_gpio(void){
	//構造体初期化
    gpio_config_t io_conf = {};
    //1になったら割り込み
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //どこのpinを有効にするか　64bitで有効にしたい番号を１にする
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
}*/

