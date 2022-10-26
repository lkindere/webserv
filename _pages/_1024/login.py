#!/usr/bin/python

import cgi

form = cgi.FieldStorage()

uname = form.getvalue('uname')
psw = form.getvalue('psw')

print 'Content-type: text/html\n\n'

print """
<!DOCTYPE html"
<html>
<head>
<title>login</title>
<meta name="author" content="mk" />
<meta http-equiv="content-type" content="text/html; />
</head>

<body>
"""
print '<h3>Hello,',uname,psw,'!</h3>'
print """
</body>
</html>
"""