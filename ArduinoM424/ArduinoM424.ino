/**
 *  BBBaden Modul 424
 *	
 *	Author: O'Connor Chris In18z
 *  Date:	02/07/2021
 *
 */

// Shift Register Input (165)
int ploadPin        = 4; 	// Connects to Parallel load pin
int clockEnablePin  = 5; 	// Connects to Clock Enable pin
int dataPin         = 6; 	// Connects to the Q7 pin
int clockPin        = 7; 	// Connects to the Clock pin

// Shift Register Output (595)
int latchPinOUT =  8;      	//Pin connected to ST_CP
int clockPinOUT = 12;      	//Pin connected to SH_CP
int dataPinOUT  = 11;      	//Pin connected to DS

// Queue and size
int qSize   = 6;
int queue[] = {0, 0, 0, 0, 0, 0};

// Timer Interrupt
unsigned long targetTime = millis() + 10000;

// Configurations for LEDs
int light_configs [7][3] = {{0, 255, 240}, {34, 221, 240}, {137, 118, 240}, {4, 251, 180}, {16, 239, 105}, {64, 191, 225}, {129, 126, 150}};

#define BYTES_VAL_T unsigned int
BYTES_VAL_T pinValues;
BYTES_VAL_T oldPinValues;

/* 
 * This function is essentially a "shift-in" routine reading the
 * serial Data from the shift register chips and representing
 * the state of those pins in an unsigned integer.
 * Source: https://playground.arduino.cc/Code/ShiftRegSN74HC165N/
 */
BYTES_VAL_T read_shift_regs()
{
	long bitVal;
	BYTES_VAL_T bytesVal = 0;

	// Trigger a parallel Load to latch the state of the data lines,
	digitalWrite(clockEnablePin, HIGH);
	digitalWrite(ploadPin, LOW);
	delayMicroseconds(5);
	digitalWrite(ploadPin, HIGH);
	digitalWrite(clockEnablePin, LOW);

	/* Loop to read each bit value from the serial out line
	* of the SN74HC165N.
	*/
	for(int i = 0; i < 16; i++)
	{
		bitVal = digitalRead(dataPin);

		// Set the corresponding bit in bytesVal.
		bytesVal |= (bitVal << ((16-1) - i));

		// Pulse the Clock (rising edge shifts the next bit).
		digitalWrite(clockPin, HIGH);
		delayMicroseconds(5);
		digitalWrite(clockPin, LOW);
	}

	return(bytesVal);
}

/* 
 * Evaluate Input Pin Values.
 */
void eval_pin_values()
{
	for(int i = 0; i < 16; i++)
	{
		if((pinValues >> i) & 1 ) {
		  Serial.print("INPUT: ");
		  Serial.println(i);
		  mapInputToQueue(i);
		}

	}

	Serial.print("\r\n");
}

/*
 * Display Configuration on LEDs.
 * Source: https://www.arduino.cc/en/Tutorial/Foundations/ShiftOut
 */
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

/*
 * Map Input Sensor ID to Configuration ID.
 */
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

	// Initialize pins...
	pinMode(latchPinOUT, OUTPUT);
	pinMode(clockPinOUT, OUTPUT);
	pinMode(dataPinOUT , OUTPUT);
	  
	pinMode(ploadPin, OUTPUT);
	pinMode(clockEnablePin, OUTPUT);
	pinMode(clockPin, OUTPUT);
	pinMode(dataPin, INPUT);

	digitalWrite(clockPin, LOW);
	digitalWrite(ploadPin, HIGH);

	// Read Shift Register and Display Config 0 (default).
	pinValues    = read_shift_regs();
	oldPinValues = pinValues;
	display_led(0);
}

/*
 * MAIN
 */
void loop()
{
	// Timer Interrupt - Display 1. config from Queue
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

	delay(500);
}
