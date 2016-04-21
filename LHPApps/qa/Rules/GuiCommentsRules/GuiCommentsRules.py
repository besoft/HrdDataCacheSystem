from xml.dom import minidom as xd
import re
from Rules.AbstractRule import AbstractRule
import os

class GuiCommentsRules(AbstractRule):
    def __init__(self):
        AbstractRule.__init__(self)
        self.DictionaryList = []
        
    def CheckFile(self,filename,match,num):
        r = open(filename, 'r')
        lines = r.readlines()
        r.close()
        
        lineNumber = 0
        for line in lines:
            lineNumber = lineNumber + 1
            x = re.compile(match)
            if(re.search(x, line)):
                founded =  re.findall('".+"\b*,|_\(".+"\b*\),|".+"\b*\);',re.split(x, line)[1])#sUBDIVIDE ELEMENTS OF THE GUI METHOD
                for f in founded:
                    line = line.replace(re.findall('".*"',f)[0],re.findall('".*"',f)[0].replace(',',' '))#REMOVE ','
                
                foundCorrectValue = False
                for el in num:
                    if (len(re.split(',',re.split(x, line)[1])) >= el):#CHECK IF THE GUI METHOD HAS A NUMBER OF ELEMENTS SUFFICIENT FOR TOOLTIP
                        if(line[len(line)-3:-1] == ');'):#REMOVE , IF PRESENT, ');' AT THE END OF THE LINE
                            line = line[:-3]
                        if(re.search('".*"',re.split(',',re.split(x, line)[1])[el-1]) != None ):
                            foundCorrectValue = True
                
                if(foundCorrectValue == False):
                    self.MarkedList.append("<item><class>" + (os.path.split(self.FullPathInputFile)[-1]) + "</class><line>" + str(lineNumber) + "</line>" + "</item>")


    def execute(self):
        f = open("./Rules/GuiCommentsRules/" + self.ParameterList[0], 'r')
        lines = f.readlines()
        f.close()
        for line in lines:
            num = []
            num = line.split()[1:]
            numInt = []
            for el in num:
                numInt.append(int(el))
                
            found = self.CheckFile( self.FullPathInputFile, line.split()[0], numInt )
            
        
        return self.MarkedList