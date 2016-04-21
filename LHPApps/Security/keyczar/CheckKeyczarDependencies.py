#-----------------------------------------------------------------------------
# BEWARE!!! This is mostly a prototype!!!
# code is changing very fast so don't rely on it :P
# author: stefano perticoni <s.perticoni@scsolutions.it>
#-----------------------------------------------------------------------------


import os
import time

Debug = False

print "##########################################################"
print "############## Checking Keyczar dependencies #############"
print "##########################################################"
print ""

curDir = os.getcwd()
print "current directory is: " + curDir

if Debug:
   for file in os.listdir(curDir):
       print file

try:
  import pyasn1
  print ""
  print "############## found pyasn1! ##############"
  print ""
except:
  print ""
  print "############## pyasn1 not found ##############"
  print "trying to install pyasn"
  os.chdir("pyasn1-0.0.8a")
  curDir = os.getcwd()
  print curDir
  os.system("python.exe setup.py install")
  os.chdir("..")
  print "current directory is: " + os.getcwd()
  print "############## pyasn1 installed successfully! ##############"
  print ""
  
try:
    
  import Crypto
  print ""
  print "############## found pycrypto! ##############"
  print ""
  
except:

  print ""
  print "############## pycrypto not found ##############"
  print "trying to install pycrypto"
  os.chdir("pycrypto-2.0.1")
  curDir = os.getcwd()
  print curDir
  result = os.system("python.exe setup.py install")
  
  if result == 0:
      # building succeeded
      pass
  
  else:
      print  "cannot build the library: installing exe package"
      os.chdir("dist")
      os.system("pycrypto-2.0.1.win32-py2.5.exe")
      os.chdir("..")
      
  os.chdir("..")
  print "current directory is: " + os.getcwd()
  print "############## pycrypto installed successfully! ##############"
  print ""
  
try:
  import simplejson
  print ""
  print "############## found simplejson! ##############"
  print ""
  
except:
  print ""
  print "############## simplejson not found ##############"
  print "trying to install simplejson"
  os.chdir("simplejson-2.0.3")
  curDir = os.getcwd()
  print curDir
  os.system("python.exe setup.py install")
  os.chdir("..")
  print "current directory is: " + os.getcwd()
  print "############## simplejson installed successfully! ##############"
  print ""
  
print "##########################################################"
print "############## Checking  dependencies finished ###########"
print "##########################################################"

time.sleep(2)