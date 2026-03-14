/*
Minimal code to test functionality of MH-Z19C CO2 Sensor via PWM

Hardware Connections:
PWM pin on MH-Z19C connects to IO35 on ESP32

Ouput:
Serial Print Values of CO2 ppm (115200 buad)
*/
#define CO2_PWM_PIN 35

volatile unsigned long previous_low_start = 0;  // us
volatile unsigned long previous_high_start = 0; //us
volatile unsigned long TL = 0; // us
volatile unsigned long TH = 0; // us

const short int CO2_log_rate = 1000; // ms

// Software interrupt on IO35 Change
void ARDUINO_ISR_ATTR read_PWM() {

  // Get level to determine if the ISR was triggered on rising on falling edge
  int level = digitalRead(CO2_PWM_PIN);

  // get the current microseconds since program start or last overflow
  unsigned long now = micros(); 

  // rising edge, the duration of the low time can be calculated by
  // subtracting the previous falling edge's time from the current time
  if (level == HIGH) {
    TL = now - previous_low_start;
    previous_high_start = now;
  } else { 
    // falling edge, the duration of the high time preceding this can be found by 
    // subtracting the previous rising edge's time from the current time
    TH = now - previous_high_start;
    previous_low_start = now;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(CO2_PWM_PIN, INPUT);
  attachInterrupt(CO2_PWM_PIN, read_PWM, CHANGE);
}

void loop() {
  static unsigned long previous_log_time = 0;
  unsigned long now = millis();

  if (now - previous_log_time >= CO2_log_rate) { // update every 1 second
    previous_log_time = now;

    // TH + TL should sum to above 4000 us to prevent a divide
    // by zero or negative ppm (can occur while sensor is heating)
    if (TH + TL > 4000) {

      // Using formula from datasheet: Cppm = 5000 * (TH - 2) / (TH + TL - 4),
      // converted to microseconds
      float ppm = 5000.0 * (TH - 2000) / (TH + TL - 4000);
      ppm = constrain(ppm, 400, 5000); // the sensor's range is 400-5000 ppm
      Serial.print("CO2: ");
      Serial.print(ppm);
      Serial.println(" ppm");
    } else {
      Serial.println("Invalid PWM Received");
    }
  }
}