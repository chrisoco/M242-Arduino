
/* Width of data (how many ext lines).
*/
#define DATA_WIDTH   16

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

int light_configs [7][3] = {{0, 255, 240}, {34, 221, 240}, {137, 118, 240}, {4, 251, 180}, {16, 239, 105}, {64, 191, 225}, {129, 126, 150}};


BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;

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
    delayMicroseconds(5);
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
        delayMicroseconds(5);
        digitalWrite(clockPin, LOW);
    }

    return(bytesVal);
}

/* Dump the list of zones along with their current status.
*/
void eval_pin_values()
{
    for(int i = 0; i < DATA_WIDTH; i++)
    {
        if((pinValues >> i) & 1 ) {
          Serial.print("INPUT: ");
          Serial.println(i);
          mapInputToQueue(i);
        }
        
    }

    Serial.print("\r\n");
}

void display_led(int arrIndex)
{
  Serial.print("Showing Config: ");
  Serial.println(arrIndex);
  
  digitalWrite(latchPinOUT, LOW);
  // shift out the bits:
  shiftOut(dataPinOUT, clockPinOUT, MSBFIRST,  light_configs[arrIndex][2]); // REG 3.
  shiftOut(dataPinOUT, clockPinOUT, MSBFIRST,  light_configs[arrIndex][1]); // REG 2.
  shiftOut(dataPinOUT, clockPinOUT, MSBFIRST,  light_configs[arrIndex][0]); // REG 1.  
  digitalWrite(latchPinOUT, HIGH);
  
}

void mapInputToQueue(int i)
{
	switch (i) {
		case  1:
		case  5: addQ(1); break;
		case  0:
		case  3:
		case  7: addQ(2); break;
		case  2:
		case 12:
		case 13: addQ(3); break;
		case  4:
		case  8:
		case  9:
		case 14:
		case 15: addQ(4); break;
		case  6: addQ(5); break;
		case 10:
		case 11: addQ(6); break;
	}
}

void addQ(int val)
{
  Serial.print("Q: ");
  Serial.println(val);
  for(int i = 0; i < qSize; i++) {
    
	// Exit if already in Q exists
    if(queue[i] == val) { 
      break; 
    }
    
	// ADD
    if(queue[i] == 0) {
      queue[i] = val;
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
  Serial.print("Current Queue [");
  for(int i = 0; i < qSize; i++) {
   Serial.print(queue[i]);     
  }
  Serial.println("}"); 
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
    oldPinValues = pinValues;
	  display_led(0);
}

void loop()
{

  if(millis() > targetTime) {
    
	  display_led(popQ());    
    targetTime = millis() + 10000;
    printQ();
  }
  
  
    pinValues = read_shift_regs();
	
	if(pinValues != oldPinValues) {
		
		eval_pin_values();
		oldPinValues = pinValues;
	}
    
    delay(1000);
}
