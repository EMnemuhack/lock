//INPUT_GPIO_PIN
#define CONFIG_INPUT_PIN1 16
#define CONFIG_INPUT_PIN2 17


//OUTPUT_GPIO_PIN
#define CONFIG_OUTPUT_PIN 23


#define ESP_INTR_FLAG_DEFAULT 0





//いらないコード置き場
/*/ボタンが押されたときに呼び出される割り込みサービスルーチン　IRAM_ATTRはメモリの領域指定(固定)してる　　必要な理由はISRで呼び出せる領域が決められているから
void IRAM_ATTR button_isr_handler(void* arg){
	xTaskResumeFromISR(ISR);
// portYIELD_FROM_ISR();
}*/



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