/* How many shift register chips are daisy-chained.
*/
#define NUMBER_OF_SHIFT_CHIPS   2

/* Width of data (how many ext lines).
*/
#define DATA_WIDTH   NUMBER_OF_SHIFT_CHIPS * 8

/* Width of pulse to trigger the shift register to read and latch.
*/
#define PULSE_WIDTH_USEC   5

/* Optional delay between shift register reads.
*/
#define POLL_DELAY_MSEC   10

#define BYTES_VAL_T unsigned int


// 165
int ploadPin        = 4;  // Connects to Parallel load pin the 165
int clockEnablePin  = 5;  // Connects to Clock Enable pin the 165
int dataPin         = 6; // Connects to the Q7 pin the 165
int clockPin        = 7; // Connects to the Clock pin the 165

// 595
int latchPinOUT =  8;      //Pin connected to ST_CP
int clockPinOUT = 12;      //Pin connected to SH_CP
int dataPinOUT  = 11;      //Pin connected to DS

int queue[] = {0, 0, 0, 0, 0, 0};
int qSize = 6;
unsigned long targetTime = millis() + 10000;

BYTES_VAL_T pinValues;

/* This function is essentially a "shift-in" routine reading the
 * serial Data from the shift register chips and representing
 * the state of those pins in an unsigned integer (or long).
*/
BYTES_VAL_T read_shift_regs()
{
    long bitVal;
    BYTES_VAL_T bytesVal = 0;

    /* Trigger a parallel Load to latch the state of the data lines,
    */
    digitalWrite(clockEnablePin, HIGH);
    digitalWrite(ploadPin, LOW);
    delayMicroseconds(PULSE_WIDTH_USEC);
    digitalWrite(ploadPin, HIGH);
    digitalWrite(clockEnablePin, LOW);

    /* Loop to read each bit value from the serial out line
     * of the SN74HC165N.
    */
    for(int i = 0; i < DATA_WIDTH; i++)
    {
        bitVal = digitalRead(dataPin);

        /* Set the corresponding bit in bytesVal.
        */
        bytesVal |= (bitVal << ((DATA_WIDTH-1) - i));

        /* Pulse the Clock (rising edge shifts the next bit).
        */
        digitalWrite(clockPin, HIGH);
        delayMicroseconds(PULSE_WIDTH_USEC);
        digitalWrite(clockPin, LOW);
    }

    return(bytesVal);
}

/* Dump the list of zones along with their current status.
*/
void display_pin_values()
{
    for(int i = 0; i < DATA_WIDTH; i++)
    {
        if((pinValues >> i) & 1 ) {
          Serial.print("1");
          /* DOSHIT */
        } else {
          Serial.print("0");
        }
        
    }

    Serial.print("\r\n");
}

void display_led(int reg1, int reg2, int reg3)
{
  
  digitalWrite(latchPinOUT, LOW);
  // shift out the bits:
  shiftOut(dataPinOUT, clockPinOUT, MSBFIRST,  58); // REG 3.
  shiftOut(dataPinOUT, clockPinOUT, MSBFIRST, 187); // REG 2.
  shiftOut(dataPinOUT, clockPinOUT, MSBFIRST,  68); // REG 1.
  
  digitalWrite(latchPin, HIGH);
  
}

void mapInputToQueue(int i)
{
  switch (i) {
    case 0:
    case 1:
    case 3:
      Serial.println("0");
      // addQ(1);
      break;
    case 4:
      Serial.println("1");
      break;
    case 5:
      Serial.println("2");
      break;
    case 6:
      Serial.println("3");
      break;
  }
}

void mapQueueToConfig()
{
  // Another switch to do Shiat.
}

void addQ(int val)
{
  
  for(int i = 0; i < qSize; i++) {
    
    if(queue[i] == val) { 
      // Serial.println("RETURN"); 
      break; 
    }
    
    if(queue[i] == 0) {
      queue[i] = val;
      // Serial.println("ADDED"); 
      break;
    }
    
  }
  
}

int popQ()
{
  int val = queue[0];
  
  for(int i = 0; i < qSize; i++) {
    
    if(i == qSize - 1) {
     queue[qSize - 1] = 0;
    } else {
      queue[i] = queue[i + 1];
    }
    
  }
  
  return val;
  
}

void printQ()
{
  for(int i = 0; i < qSize; i++) {
   Serial.print(queue[i]);     
  }
  Serial.println("."); 
}



void setup()
{
    Serial.begin(9600);

    /* Initialize our digital pins...
    */
    pinMode(latchPinOUT, OUTPUT);
    pinMode(clockPinOUT, OUTPUT);
    pinMode(dataPinOUT , OUTPUT);
      
    pinMode(ploadPin, OUTPUT);
    pinMode(clockEnablePin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, INPUT);

    digitalWrite(clockPin, LOW);
    digitalWrite(ploadPin, HIGH);

    /* Read in and display the pin states at startup.
    */
    pinValues = read_shift_regs();
    display_pin_values();
    oldPinValues = pinValues;
}

void loop()
{

  if(millis() > targetTime) {
    
    // popQ() ez Clap
     Serial.println("Ampeln UMSCHALTEN");
    
    targetTime = millis() + 10000;
    
  }
  
/* Read the state of all zones.
*/
    read_shift_regs();
    
    display_pin_values();
    oldPinValues = pinValues;

    delay(1000);
}
