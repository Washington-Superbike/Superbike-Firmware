// Define pins
#define RD1 14
#define RD2 15
#define RD3 16
#define V_OUT 17
#define T 8  // transistor

float v_cell1;
float v_cell2;
float v_cell3;
float current;
float current_v;
float rd_adc_ratio_1;
float rd_adc_ratio_2;
float rd_adc_ratio_3;
float temp;

void setup() {
  pinMode(RD1, INPUT);
  pinMode(RD2, INPUT);
  pinMode(RD3, INPUT);
  pinMode(V_OUT, INPUT);
  pinMode(T, OUTPUT);

  digitalWrite(T, HIGH);
 
  rd_adc_ratio_1 = ((32150.0 +  9900.0) * 3.3) / (32150.0 * 1023.0);
  rd_adc_ratio_2 = ((28850.0 + 46600.0) * 3.3) / (28850.0 * 1023.0);
  rd_adc_ratio_3 = ((16160.0 + 46300.0) * 3.3) / (16160.0 * 1023.0);
}

void loop() {
  // Convert the resistor divider readings to get voltage.
  v_cell1 = analogRead(RD1) * rd_adc_ratio_1;
  v_cell2 = analogRead(RD2) * rd_adc_ratio_2;
  v_cell3 = analogRead(RD3) * rd_adc_ratio_3;
  
  // Convert the current sensor reading to get current (A).
  current_v = (analogRead(V_OUT) / 1023.0) * 3.3;
  current = (current_v - 1.665) / 0.110;  // +- 0.1

  // Print the voltage of each cell.
  Serial.print("Cell 1 Voltage: ");
  Serial.println(v_cell1);
  Serial.print("Cell 2 Voltage: ");
  Serial.println(v_cell2 - v_cell1);
  Serial.print("Cell 3 Voltage: ");
  Serial.println(v_cell3 - v_cell2);

  // Print the current sensor voltage and current readings.
  Serial.print("Current Sensor Voltage: ");
  Serial.println(current_v);
  Serial.print("Current Sensor Current: ");
  Serial.println(current);
  
  delay(2000);  // delay 2s
}
