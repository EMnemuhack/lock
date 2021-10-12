const int IN1 = 17;
const int CHANNEL_0 = 1;

const int LEDC_TIMER_BIT = 18;
const int LEDC_BASE_FREQ = 50;
const int VALUE_MAX = 31500;
const int VALUE_MIN = 6562;


char recv[0x1000];
uint16_t rpos = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(IN1, OUTPUT);
  Serial.begin(9600);
  ledcSetup(CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
  ledcAttachPin(IN1, CHANNEL_0);
}
int32_t indata = 0;
void loop() {
  // put your main code here, to run repeatedly:
  String cmd = "";

  while (Serial.available() > 0)
  {
    recv[rpos] = Serial.read();
    if (recv[rpos] == 0x0d)
    {
      recv[rpos] = 0x0;
    } else if (recv[rpos] == 0x0a)
    {
      recv[rpos] = 0x0;
      cmd.concat(&recv[0]);
      indata = cmd.toInt();

      Serial.print("setData= ");
      Serial.println(indata);
      rpos = 0;
    } else
    {
      Serial.print(recv[rpos]);
      rpos = (rpos + 1) & 0xfff;
    }

  }
  int val = map(indata, -360, 360, VALUE_MIN, VALUE_MAX);
  Serial.print("val=   ");
  Serial.println(val);
  ledcWrite(CHANNEL_0, val);
}