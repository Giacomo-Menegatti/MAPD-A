import sys

name = sys.argv[1]

program_file = 'output.txt' if name == None else name

program = {'CODE' : [], 'VALUE' : []}

assembly = {'NOP': 0x0, 'LDA': 0x1, 'ADD': 0x2,
             'SUB': 0x3, 'STA': 0x4, 'LDI': 0x5,
             'JMP': 0x6, 'JC':0x7, 'JZ':0x8,
             'OUT': 0xE, 'HLT':0xF}

PROGRAM_LEN = 16

def printProgram():
  for i in range(PROGRAM_LEN):
    print(f'0x{i:0X} : {program["CODE"][i]} \t 0x{program["VALUE"][i]:02X} \t {program["VALUE"][i]:08b} ')


with open(program_file) as f:
  lines = f.readlines()
  while len(lines) < PROGRAM_LEN:
    lines.append('0 \n') #Append zeros if there are missing lines

  for l in lines:
    l = l.strip() # Removing any white space and newline
    data = l.split()  #split command and values

    if not assembly.get(data[0]) == None: #If it is in the dictionary it is a command

      if data[0]=='NOP' or data[0]=='HLT' or data[0]=='OUT': #Commands without values (0 by default)
        program['CODE'].append(f'{data[0]} 0x{0:0X}')
        program['VALUE'].append(assembly[data[0]]*0x10 + 0x0)
      else:                                                   #Commands with a value
        value = int(data[1], PROGRAM_LEN)  
        program['CODE'].append(f'{data[0]} 0x{value:0X}')
        program['VALUE'].append(assembly[data[0]]*0x10 + value)
    
    else:                                                    #Memory data, without a command
      value = int(data[0], PROGRAM_LEN)
      program['CODE'].append(f'0x{value:02X}')
      program['VALUE'].append(value)

printProgram()

import pyfirmata
import time

board = pyfirmata.Arduino('/dev/ttyUSB0') #Change the port if the Arduino doesn't show up
print('Communication started')

address_pin = [board.digital[i] for i in [8,7,6,5]] #Pins for the memory address
for pin in address_pin: pin.write(0)

data_pin = board.digital[2]      #Shift register pins
clock_pin = board.digital[3]
latch_pin = board.digital[4]

button_pin = board.digital[13]  #Pin that copies the temporary data inside the register, active LOW
button_pin.write(1)

def set_address(address):
  '''
    Function to set the address
  '''
  for i in range(4):
    bit = (address >> i) & 1
    address_pin[i].write(bit)

def send_byte(byte, bitOrder = 'MSBFIRST'): 
  '''
    Function to send a byte 
  '''
  latch_pin.write(0)  #Latch low so the writing doesn't affect the output
  for i in range(8):
    if (bitOrder == 'LSBFIRST'): #LSBFIRST
      data_pin.write(byte >> i & 1) #Write the bit to the serial data line
    else:                        #MSBFIRST
      data_pin.write(byte >> 7-i & 1) 

    clock_pin.write(1)  #Clock signal to shift the data
    clock_pin.write(0) 

  latch_pin.write(1)  #Latch high to change the output
  time.sleep(0.01)  #Wait 10 ms
  latch_pin.write(0)  #Latch low

for i in range(PROGRAM_LEN):

  set_address(i)
  print(f'Accessing memory address {i}: {i:04b}')
  send_byte(program['VALUE'][i], 'LSBFIRST')
  print(f'\tWriting the instruction {program["CODE"][i]} : {program["VALUE"][i]:08b} ')

  button_pin.write(0) #copy the value in the memory
  time.sleep(0.20)
  button_pin.write(1)

  time.sleep(0.1) #Wait before sending another line






