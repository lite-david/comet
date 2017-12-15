import sys
number_of_registers = 32
offset_in_log_file = 3

with open(sys.argv[1]) as f:
	file1 = filter(None, (line.rstrip() for line in f))


with open(sys.argv[2]) as f:
	file2 = filter(None, (line.rstrip() for line in f))

def tracker(filelines,n):
	curr = ""
	for line in filelines:
		contents = line.split(';')
		try:
			if curr != contents[n]:
				yield contents[0], contents[n]
			curr = contents[n]
		except IndexError as err:
			pass	
 		
for reg in xrange(0,number_of_registers):
	for a,b in zip(tracker(file1,reg+offset_in_log_file),tracker(file2,reg+offset_in_log_file)):
		if a[1] != b[1]:
			print "Difference found in reg " + str(reg)
			print a[0],a[1], b[0], b[1]
			break
