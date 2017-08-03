#Convert binary serial file to ASCII

import binascii
import os

file = raw_input("Data file \n")

file = open(file, 'r')
output = open('ASCII.txt', 'a')

for line in file:
	line = line.split()

	for word in line:
		words = binascii.b2a_uu(word)

		output.append(words )

file.close()
output.close()