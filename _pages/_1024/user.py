#!/usr/bin/python
import sys, os
 
print "Content-Type: text/html\n\n";

user = os.environ.get('REMOTE_USER')
if len(user) != 0:
    print "Logged in as: ", user
else:
    print "Not logged in"