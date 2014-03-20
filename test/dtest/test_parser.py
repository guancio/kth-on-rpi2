#!/usr/bin/python

import sys
from subprocess import call

errors = [] 

while 1:
	try:
		line = sys.stdin.readline()
	except KeyboardInterrupt:
		break
	if not line:
		break
	line = line[:-1]
	print line
	if (line.find("FAIL") >= 0):
		errors.append(line)
	if (line.find("TEST COMPLETED") >= 0):
		call(["pkill", "qemu"])

print "TEST COMPLETED"
if (len(errors) == 0):
	print "NO ERROR"
	exit()
print ("%d ERRORs" % (len(errors)))
print "\n".join(errors)