# vim: set ts=4 nu ai:
import re, string
import sys

def get_pc_instruction_registers(line):
	split_line = line.split(";")
	pc = split_line[1]
	temp = split_line[2].split()[0]
	instruction = re.split('[A-Z]',temp)[0]
	opcode = temp.split(instruction)[-1]
	reg = [0]*32
	reg = split_line[3:]
	return pc, instruction, opcode, reg

def get_registers(line):
	split_line = line.split(";")
	reg = [0]*32
	reg = split_line[3:]
	return reg

def compare_registers(reg1, reg2):
	for i,j in zip(reg1,reg2):
		# print i==j
		# print i,j
		if i != j:
			if(len(i) > 8):
				continue
			else:
				print "Error at reg " + i,j
				return -1
	return 0

with open(sys.argv[1]) as f:
	correct_op = f.read().splitlines()

with open(sys.argv[2]) as f:
	op_to_be_verified = f.read().splitlines()

l_correct_op = len(correct_op)
l_op_to_be_verified = len(op_to_be_verified)

j = 0
pc_line = 0
prev_pc1 = 0
prev_opcode = ""
next_opcode = ""
ld_dest = ""
flag_1 = 0
flag_2 = 0
for i in xrange(0,l_correct_op):
	pc1, instruction1, opcode1, reg1 = get_pc_instruction_registers(correct_op[i])
	if(opcode1 == "SYSTEM"):
		break
	
	if(flag_2 == 1):
		j+=1
		flag_2 = 0

	if(flag_1 == 1):
		temp = correct_op[i].split(';')[2].translate(None, string.punctuation)
		if(ld_dest in temp.split(' ')):
			flag_2 = 1
		elif(opcode1 == "JALR"):
			hex_ins = int(instruction1,16)
			the_val = (hex_ins & 0xf8000) >> 15
			if "r"+str(the_val) == ld_dest:
				flag_2 = 1
		else:
			flag_2 = 0	
		flag_1 = 0

	if(opcode1 == "LDW" or opcode1 == "LDBU"):
		ld_dest = correct_op[i].split(';')[2].split(' ')[1]
		flag_1 = 1		

	pc2, instruction2, opcode2, reg2 = get_pc_instruction_registers(op_to_be_verified[j])
	#print pc1, pc2
	if(flag_2 == 1):
		reg2 = get_registers(op_to_be_verified[j+5])
	else:
		reg2 = get_registers(op_to_be_verified[j+4])
	pc_line = j
	if opcode1[0] == 'J' or opcode1[0] == 'B':
		pc3, instruction3, opcode3, reg3 = get_pc_instruction_registers(correct_op[i+1])
		if int(pc3,16) - int(pc1,16) != 4:
			j+=2
	#print "pcs being compared are " + str(pc1) + " " + str(pc2) 
	if pc1 != pc2:
		print "Error in PC in correct_op in line " + str(i) + "and in op line " + str(pc_line)
		print pc1,instruction1,opcode1,pc2,instruction2
		break
	if instruction1 != instruction2:
		print "Error in PC in correct_op in line " + str(i) + "and in op line " + str(pc_line)
		break
	if compare_registers(reg1,reg2) == -1:
		print "Error incorrect register values in correct_op line" + str(i) + "and in op line" + str(pc_line)
		break
	j+=1
print "Completed .."
