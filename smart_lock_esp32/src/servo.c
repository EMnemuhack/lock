#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "esp_log.h"
#include "driver/mcpwm.h"


#include "driver/ledc.h"

#include "env.h"
#include "servo.h"


static char tag[] = "servo1";

/* Servo Motor control example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
static const char *TAG = "example";

// You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH_US (625) // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US (2500) // Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE        (90)   // Maximum angle in degree upto which servo can rotate

#define SERVO_PULSE_GPIO        (23)   // GPIO connects to the PWM signal line

static inline uint32_t example_convert_servo_angle_to_duty_us(int angle)
{
    return (angle + SERVO_MAX_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (2 * SERVO_MAX_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

void pwm_servo(void)
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, SERVO_PULSE_GPIO); // To drive a RC servo, one MCPWM generator is enough

    static int direction = -1;

    mcpwm_config_t pwm_config = {
        .frequency = 50, // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        .cmpr_a = 0,     // duty cycle of PWMxA = 0
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

    do{
        for (int angle = (SERVO_MAX_DEGREE * direction); angle < SERVO_MAX_DEGREE * (-(direction)); angle++) {
            ESP_LOGI(TAG, "Angle of rotation: %d", angle);
            ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, example_convert_servo_angle_to_duty_us(angle)));
            vTaskDelay(pdMS_TO_TICKS(11)); //Add delay, since it takes time for servo to rotate, generally 100ms/60degree rotation under 5V power supply
        }
        direction = direction * -1;
    }while(0);
}

//サーボモータ制御
void task_servoSweep(void) {
	
	static int direction       = 1; // 1 = up, -1 = down
    static int duty = DUTY;

    //debug用
	int valueChangeRate = VALUE_CHANGE_RATE; // msecs	変化率
    int sweepDuration =  SWEEP_DURATION;
	printf("servo起動\n");
	ESP_LOGD(tag, ">> task_servo1");

	//タイマーの設定
	ledc_timer_config_t timer_conf;
	timer_conf.bit_num    = LEDC_TIMER_15_BIT;		//タイマーのbit
	timer_conf.freq_hz    = FREQ_HZ;				//周波数
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
	int changeDelta = (MAX_VALUE-MIN_VALUE) / changesPerSweep;// 変化率(2500 - 625) / (1000 / 20)
	int i;
	ESP_LOGD(tag, "sweepDuration: %d seconds", sweepDuration);
	ESP_LOGD(tag, "changesPerSweep: %d", changesPerSweep);
	ESP_LOGD(tag, "changeDelta: %d", changeDelta);
	ESP_LOGD(tag, "valueChangeRate: %d", valueChangeRate);
	
	//180度動かす
	do{
		for (i=0; i<changesPerSweep; i++) {
			if (direction > 0) {
				duty += changeDelta;
			} else {
				duty -= changeDelta;
			}
			ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
			ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
			vTaskDelay(valueChangeRate/portTICK_PERIOD_MS);
		}
		direction = -direction;	//＋側へ回転　-側へ回転の切り替え
		ESP_LOGD(tag, "Direction now %d", direction);
	}while(0); // End loop forever
	vTaskDelete(NULL);
}

/*サーボモータ制御
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

	printf("元のDuty%d\n",duty);
	printf("シフト%d * minvalue%d\n",(1<<bitSize),minValue);
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
	printf("Duty%d",duty);
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
}*/