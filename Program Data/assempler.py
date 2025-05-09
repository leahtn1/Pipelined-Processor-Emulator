#Assempler.py

#The basis of this assembler was provided by chatgpt, and was modified for my custom instruction set

from enum import Enum

class Types(Enum):
    R_TYPE = 1
    I_TYPE = 2
    J_TYPE = 3


def to_binary(value, bits):
    """Convert a number to a binary string with zero-padding, e.g., 5 => '000101'."""
    if value < 0:
        value = (1 << bits) + value  # For negative numbers: do two's complement manually
    return format(value, f'0{bits}b')  # Format as binary string, pad to 'bits' length

# Dictionary that maps instruction names (like 'add') to 5-bit opcode strings
OPCODES = { 
    'nop' : '00000',
    'addi' : '01000',
    'subi' : '01001',
    'bnq' : '01010', 
    'sll' : '01011',
    'slr' : '01100',
    'sto' : '01101',
    'ld' : '01110',
    'andi' : '01111',
    'ori' : '11000',
    'beq' : '11001',

    'add' : '10000',
    'sub' : '10001',
    'and' : '10010',
    'or' : '10011',
    'not' : '10100',
    'sllr' : '10101',
    'jmp' : '00001'
}

LABELS = {}     #these contain loop jump points

def assemble_line(line, num):
    line = line.strip()  # Remove whitespace from beginning and end
    if not line or line.startswith('#'):
        return None  # Ignore blank lines or comments

    # Remove commas and split into words: "add $1, $2, $3" => ['add', '$1', '$2', '$3']
    line = line.split('//')[0].strip()
    tokens = line.replace(',', '').split()
    args = []
    
    if tokens[0] in OPCODES:
        op = tokens[0]  
        args = tokens[1:]  # Remaining items are arguments (e.g., ['$1', '$2', '$3'])
    elif tokens[0] not in LABELS:       
        label = tokens[0]
        LABELS[label] = num         #assign a name to jump points in the program (line numbers)
        #print("label assigned: ", label)
        op = tokens[1]  
        args = tokens[2:]  # Remaining items are arguments (e.g., ['$1', '$2', '$3'])
    else:
        print("Parse error. This label has been used")
        return None
        
    #print('\n\nARGS: ', args)
        


    if op not in OPCODES:
        print(f"Unknown opcode: {op}")
        return None  # Stop if the opcode is not recognized
    elif OPCODES[op].startswith('01') or OPCODES[op].startswith('11'):
        instr_type = Types.I_TYPE
    elif OPCODES[op].startswith('10'):
        instr_type = Types.R_TYPE
    else:
        instr_type = Types.J_TYPE

    

    opcode = OPCODES[op]  # Get binary string for opcode

    bin_args = []  # This will hold binary strings for each argument
    
    if opcode == '00000':
        bin_args = ['0']*27
        #print('\n\nNOP RETURNED')

    elif instr_type == Types.I_TYPE:
        source = destination = imm = '' 
        for i, arg in enumerate(args):
            #print('\nAnalysing: ', arg)
            if arg.startswith('//'):
                break  # Ignore the rest of the line after a comment
            
            elif arg.startswith('t'):  # Register, e.g., t3
                val = int(arg[1:])  # Strip off the 't' and convert the rest to an int
                #print('its a register addy')
                if i == 0:
                    destination = to_binary(val, 5)
                else:
                    source = to_binary(val, 5)
            else:
                try:
                    val = int(arg)
                    #print('value is an immediate int')
                    imm = to_binary(val, 17)
                    #print('imm ', imm)
                except:
                    if arg in LABELS:           #if its a branch instruction, the last 17 bits are the line to jump to
                        imm = to_binary(LABELS[arg], 17)
                        #print('val is a label. imm ', imm)
                    else:
                        print('Parse error. Branch point not defined')
                        
        if op == 'sto':       #store instructions
            bin_args.extend([destination, source, imm])
        else:   #regular instructions
            bin_args.extend([source, destination, imm])
            

    elif instr_type == Types.R_TYPE:
        source1 = source2 = destination = ''
        for i, arg in enumerate(args):
            #print('\nAnalysing: ', arg)
            if arg.startswith('//'):
                break  # Ignore the rest of the line after a comment
            elif arg.startswith('t'):  # Register, e.g., t3
                val = int(arg[1:])
                if i == 0:
                    destination = to_binary(val, 5)
                elif i == 1:
                    source1 = to_binary(val, 5)
                else:
                    source2 = to_binary(val, 5)
        
        bin_args.extend([source1, source2, destination, '000000000000'])
        
    elif instr_type == Types.J_TYPE:
        for arg in args:
            if arg.startswith('//'):
                break  # Ignore the rest of the line after a comment
            elif arg.startswith("1") or arg.startswith("0"):
                val = int(arg)
                addy = to_binary(val, 27)
            else:
                print("ERROR: Incorrect J_Type syntax")
        
        bin_args.extend(addy)
        
            


    # Join opcode and arguments into one full binary instruction
    return opcode + ''.join(bin_args)

def assemble_file(input_path, output_path):
    with open(input_path) as f, open(output_path, 'w') as out:
        for j, line in enumerate(f):  # Read file line-by-line
            binary = assemble_line(line, j)  # Assemble each line to binary
            if binary:
                out.write(binary + '\n')  # Write binary instruction to file
                print(f"{line.strip():<60} => {binary}")  # Print for debugging


assemble_file('assembly.txt', 'binary.txt')
print(LABELS)
