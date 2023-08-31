import time
import datetime
#Input a csv to make the correct number of lines:input AmmoniaOne

fpRead = "AmmoniaOne.csv"
fpWrite = "TenMillionEntries.csv" #change name accordingly

fRead = open(fpRead,"r")
fWrite = open(fpWrite, "w")

numLines = len(open(fpRead).readlines())
delimiter = ';'
numOfData =10000000 # number of datapoints wanted

numInserted = 0 # number of lines inserted

#Take desired amount of datapoints from input file
while ((numInserted < numLines-1) and (numInserted < numOfData)):
    fWrite.write(fRead.readline())
    numInserted +=1
saveLine = fRead.readline()
fWrite.write(saveLine)

#Make more datapoints when need more than available
if (numLines <= numOfData):
    saveLineSplit = saveLine.split(delimiter)
    timeString = saveLineSplit[3] #in this case on the third one change to make more general later
    format = "%m/%d/%Y %I:%M:%S %p"
    timeObject=datetime.datetime(*time.strptime(timeString, format)[:6])
    incrementTime= datetime.timedelta(0,0,0,0,1) # increment by 1 minute

    for i in range(numOfData-numLines+1):
            timeObject = timeObject + incrementTime
            saveLineSplit[3] = timeObject.strftime(format) #format and insert time into string
            fWrite.write(delimiter.join(saveLineSplit))
