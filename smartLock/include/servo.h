#define BIT_SIZE 15         //bit幅
#define MIN_VALUE 625       //サーボMIN micro seconds (uS)
#define MAX_VALUE 2500      //サーボMAX micro seconds (uS)
#define SWEEP_DURATION 1000 //終了時間  milliseconds (ms)
#define VALUE_CHANGE_RATE 20  //変化率
#define FREQ_HZ 50          //周波数
#define DUTY    ((1<<BIT_SIZE) * MIN_VALUE / 20000)  //PWM　比率


//関数定義
void pwm_servo(bool);
