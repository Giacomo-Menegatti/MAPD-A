/*  Sketch to aoutomatically load a program in the SAP-1 memory
 *  The program uses 1 74hc595 to address the 8 bit memory 
 *  and 4 pins to control the 4-bit memory address. 
 *  Press the button to load a new line on the SAP.
 *  At the end the contents of the memory can be checked
 *  by writing the memory address in the serial monitor.
*/

#define SHIFT_DATA 2
#define SHIFT_CLK 3
#define SHIFT_LATCH 4
#define ADDRESS_0 5
#define ADDRESS_3 8
#define BUTTON_PIN 13    //This bypass the button on the board  

const int MAX_PROGRAM_LENGTH = 16;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

byte program[MAX_PROGRAM_LENGTH]= {
    //Fibonacci 
  0x51, 0x4E, 0x50, 0xE0,
  0x2E, 0x4F, 0x1E, 0x4D,
  0x1F, 0x4E, 0x1D, 0x70,
  0x63, 0x00, 0x00, 0x00  
  

  /*  //Triangular Numbers
  0x51, 0x4D, 0x50, 0x2E,
  0x4F, 0x51, 0x2E, 0x4E,
  0x1F, 0xE0, 0x70, 0x63,
  0x00, 0x00, 0x00, 0x00  
  */
/*
  0x1F, 0xE0, 0xF0, 0x0, 
  0x0, 0x0, 0x0, 0x0, 
  0x0, 0x0, 0x0, 0x0, 
  0x0, 0x0, 0x0, 0xFF
  */
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Write the memory address
*/
void setAddress(byte address){  
  for(int pin=ADDRESS_3; pin >= ADDRESS_0; pin--){
    digitalWrite(pin, address & 1);   //true (1) if the last bit is 1, false (0) else 
    address = address >> 1;           //Shift right the byte to access the following bit
  }
}

/*
 * Write a byte of the program
*/
void writeByte(byte data){  
  digitalWrite(SHIFT_LATCH, LOW); // Latch OFF, so the change doesn't affect the output
  shiftOut(SHIFT_DATA, SHIFT_CLK, LSBFIRST, data);  //Load the byte in the SHIFT REGISTER
  digitalWrite(SHIFT_LATCH, HIGH);  // Latch ON to change the outputs 
  digitalWrite(SHIFT_LATCH, LOW);   // and OFF again
}

/* 
 * Print the program (memory address : bin format : hex format : assembly)
*/

void printProgram(){
  Serial.println("Address : BIN : HEX : ASSEMBLY");

  for(int line=0; line<MAX_PROGRAM_LENGTH; line++){
    Serial.print("Line 0x");
    Serial.print(line, HEX);
    byte data = program[line];
    Serial.print(" : 0b");
    Serial.print(data, BIN);
    Serial.print(" : 0x");
    Serial.print(data, HEX);
    Serial.print(" : ");
    Serial.println(assembly(data));
  }
  Serial.println("\n\n");
}

/*
 * Byte to assembly converter
*/

String assembly(byte data){
  byte value = data & 15;  //Takes the last nibble 
  byte instruction = data >> 4; //Takes the first nibble
  String command = "";          //Name of the command
  bool hasOperand = true;       //If the command has an operand or not
  switch(instruction){
    case 0x0: command = "NOP "; hasOperand=false; break;
    case 0x1: command = "LDA "; break;    
    case 0x2: command = "ADD "; break;
    case 0x3: command = "SUB "; break;
    case 0x4: command = "STA "; break;
    case 0x5: command = "LDI "; break;
    case 0x6: command = "JMP "; break;
    case 0x7: command = "JMC "; break;
    case 0x8: command = "JZ "; break;
    case 0xE: command = "OUT "; hasOperand=false; break;
    case 0xF: command = "HLT "; hasOperand=false; break;
    default: value = data; //If the command is not recognized, the value is the whole byte
  }
  return hasOperand ? command + " 0x" + String(value, HEX) : command;   //Return the String command + value
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(57600);

  pinMode(SHIFT_DATA, OUTPUT);    //Setting all the pins to OUTPUT
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  for(int pin=ADDRESS_0; pin<= ADDRESS_3; pin++){
    pinMode(pin, OUTPUT);
  }

  Serial.println("////////////////////////////////////////////////////////////////////////////////// \n");
  Serial.println("SAP -1 Program Loader\n");
  Serial.println("////////////////////////////////////////////////////////////////////////////////// \n");

  printProgram(); //Print the program loaded in the sketch
  
  for(byte i=0; i<MAX_PROGRAM_LENGTH; i++){  // For all 16 lines

    setAddress(i);
    writeByte(program[i]);
    Serial.print("Writing at memory address ");
    Serial.print(i);
    Serial.print(" : ");
    Serial.println(program[i], BIN);
    
    digitalWrite(BUTTON_PIN, false);
    delay(250);
    digitalWrite(BUTTON_PIN, true);

    delay(50); 
  }


}

void loop() {
}


