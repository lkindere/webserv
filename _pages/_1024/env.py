#!/usr/bin/python
import sys, os
 
print "Content-Type: text/html\n\n";
 
os.environ["TEST_VAR"] = "go4expert.com"
for name, value in os.environ.items():
        print "%s\t= %s <br/>" % (name, value)