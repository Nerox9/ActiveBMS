//**---------------**//
//** PIN VARIABLES **//
//**---------------**//

//** Misc. **//
byte tempSensor = A0;
byte ampSensor = A1;

//** Cell Feedback **//
byte fb1 = A2;
byte fb2 = A3;
byte fb3 = A4;
//byte fb4 = A5;

//**-------------------**//
//** VOLTAGE VARIABLES **//
//**-------------------**//
float temp = 0;
float amp = 0;
float v1 = 0;
float v2 = 0;
float v3 = 0;
//word v4 = 0;

//**-------------**//
//** MOSFET PINS **//
//**-------------**//
/*
byte pwm3s = 11;
byte pwm2n = 10;
byte pwm2s = 9;
byte pwm1s = 6;
byte pwm1n = 5;
byte pwm3n = 3;
*/
byte pwm2s = 10;
byte pwm2n = 9;
byte pwm1s = 5;
byte pwm1n = 3;

//**------**//
//** MISC **//
//**------**//
float margin = 0.01; // min 10 mV difference btw cells


//**-----------------**//
//** Charging States **//
//**-----------------**//
typedef enum {
  noCharging,
  north2south,
  south2north
} chargingState;

//**-----------**//
//** Cell Sets **//
//**-----------**//
struct cellSet
{
  int id;             // Cell set id
  float* cellNorth;   // North cell voltage
  float* cellSouth;   // South cell voltage
  byte* pMosfet;      // p Mosfet pin
  byte* nMosfet;      // n Mosfet pin
  chargingState state;  // Cell set charging state
};

// Create Cell Sets
cellSet cell_1_2 = {0, &v2, &v1, pwm1s, pwm1n, noCharging};
cellSet cell_2_3 = {1, &v3, &v2, pwm2s, pwm2n, noCharging};
//cellSet cell_3_4 = {2, v4, v3, pwm3s, pwm3n, noCharging};


// Check the voltage of cells and decide to charging
void checkCells(cellSet* cells){
  //Serial.println("Cells checking");
  //Serial.print(*cells.cellNorth);
  //Serial.print(" ");
  //Serial.println(*cells.cellSouth);

  // Charge the South Cell
  if(*cells->cellNorth>(*cells->cellSouth + margin)){
    pinMode(cells->nMosfet,INPUT);
    pinMode(cells->pMosfet,OUTPUT);
    cells->state = north2south;
    //Serial.println("north to south");
    }
  
  // Charge the North Cell
  else if (*cells->cellSouth>(*cells->cellNorth + margin)){
    pinMode(cells->pMosfet,INPUT);
    pinMode(cells->nMosfet,OUTPUT);
    cells->state = south2north;
    //Serial.println("south to north");
  }

  // No Charging
  else
  {
    pinMode(cells->pMosfet,INPUT);
    pinMode(cells->nMosfet,INPUT);
    cells->state = noCharging;
  }
  //Serial.println("");
}

// Read Temperature and Ampere Sensor
void readSensors(){
  temp = (analogRead(tempSensor) / 1024.0) * 5000;
  temp /= 10; //Celsius Degree
  
  amp = (analogRead(ampSensor) / 1024.0) * 5000;
  amp = (amp - 2500 / 185); // Sensitivity = 185 for 5A, 100 for 20A and 66 for 30A versions
}


// Read Cells Voltage
void readCells(){

  v1 = analogRead(fb1);
  v2 = analogRead(fb2);
  v3 = analogRead(fb3);
  //v4 = analogRead(fb4);

  v1 = v1*5.00/1023;
  v2 = 2*v2*5.00/1023 - v1;
  v3 = 3*v3*5.00/1023 - v1 - v2;
  }

// Communicate with PC or other Arduino
void communicate(){
  Serial.println("Sensor Data");
  Serial.print("Temperature: ");
  Serial.println(temp);
  Serial.print("Current: ");
  Serial.println(amp);
  Serial.println("");
  
  Serial.println("Cell Voltage");
  
  Serial.print("Cell 1: ");
  Serial.print(v1);
  Serial.print(" ");
  
  Serial.print("Cell 2: ");
  Serial.print(v2);
  Serial.print(" ");
  
  Serial.print("Cell 3: ");
  Serial.println(v3);
  //Serial.println("");

 
  Serial.println("Cell Charging State");
  Serial.print("Converter ");
  Serial.print(cell_1_2.id);
  Serial.println("");
  
  Serial.print("State: ");
  // Print Charging State
  switch(cell_1_2.state)
  {
    case noCharging:
      Serial.print("No Charging");
      break;

    case north2south:
      Serial.print("North to South");
      break;

      case south2north:
      Serial.print("South to North");
      break;
  }
  Serial.println("");

  
  Serial.print("Converter ");
  Serial.print(cell_2_3.id);
  Serial.println("");
  
  Serial.print("State: ");
  // Print Charging State
  switch(cell_2_3.state)
  {
    case noCharging:
      Serial.print("No Charging");
      break;

    case north2south:
      Serial.print("North to South");
      break;

      case south2north:
      Serial.print("South to North");
      break;
  }
  Serial.println("\r\n\r\n");
}

// Disable Charging
void disableCharging(){
  // Release Timer0 for communication
  TCCR0A &= (0 << WGM01) & (0 << WGM00);
  TCCR0B &= (0 << WGM02);
  
  pinMode(pwm1s, INPUT);
  pinMode(pwm1n, INPUT);
  pinMode(pwm2s, INPUT);
  pinMode(pwm2n, INPUT);
}

// Enable Charging
void enableCharging(){
  // Config Timer0 Back
  TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
  TCCR0B = (1 << WGM02)  | (1 << CS00);

}


// Configure Timers
void setTimers()
{
  ICR1 = 80;  // Set Timer1 Freq
  OCR1B = 52; // Set Timer1 PIN10 Duty Cycle
  OCR1A = 26; // Set Timer1 PIN9  Duty Cycle
  
  OCR0B = 26; // Set Timer0 PIN5 Duty Cycle
  OCR0A = 80; // Set Timer0 Freq
  
  OCR2B = 52; // Set Timer2 PIN3 Duty Cycle
  OCR2A = 80; // Set Timer2 Freq

  // Disable PWM
  pinMode(pwm1s, INPUT);
  pinMode(pwm1n, INPUT);
  pinMode(pwm2s, INPUT);
  pinMode(pwm2n, INPUT);
  //pinMode(pwm3s, INPUT);
  //pinMode(pwm3n, INPUT);
  

  // Timer0 Config
  TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
  TCCR0B = (1 << WGM02)  | (1 << CS00);

  // Timer1 Config
  TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
  TCCR1B =  (1 << WGM12) | (1 << WGM13)| (1 << CS10);

  // Timer2 Config
  TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
  TCCR2B =  (1 << WGM02) | (1 << CS20);
}

void softDelay(int loopCount){
  for(int i=0; i<loopCount; i++);
}

void setup() {
  // Set PINs
  pinMode(tempSensor, INPUT);
  
  pinMode(fb1, INPUT);
  pinMode(fb2, INPUT);
  pinMode(fb3, INPUT);
  //pinMode(fb4, INPUT);

  // Start UART
  Serial.begin(9600);
  // Config timers
  setTimers();
}

 

void loop(){
  // Disable Charging in ADC read cycle
  disableCharging();
  softDelay(100);
  
  // Read ADCs
  readSensors();
  readCells();
  communicate();
  softDelay(100);
  
  // Enable Charging
  enableCharging();
  
  //check cell sets and charge
  checkCells(&cell_1_2);
  checkCells(&cell_2_3);
  //checkCells(cell_3_4);

  delay(2000);
}
