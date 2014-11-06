import itertools
import random
import subprocess
import sys
import time

ARRAY_MIN_SIZE = 50
ARRAY_MAX_SIZE = 151
ARRAY_INCREMENT = 50

MIN_PRECISION = 10
MAX_PRECISION = 1000

MIN_THREADS = 1
MAX_THREADS = 3

NUM_TESTS = 10

random.seed()

def genNums(size):
    numlist = ''
    for i in range(size * size):
        numlist += str(random.randint(0, 10))
    return numlist

def printInfo(count, size, threads, precision, output):
    outputList = []
    for line in output.stdout:
        outputList.append(line)
    print count, ',',
    print size, ',',
    print threads, ',',
    print precision, ',',
    print outputList[0].rstrip('\n'), ',',
    print outputList[1].rstrip('\n')

#def printOutput(output):
    #for line in output.stdout:
        #sys.stdout.write(line)

tArrays = []
tSizes = []
for size in range(ARRAY_MIN_SIZE, ARRAY_MAX_SIZE, ARRAY_INCREMENT):
    tArrays.append(genNums(size))
    tSizes.append(str(size))

tThreads = []
for i in range(MIN_THREADS, MAX_THREADS):
    tThreads.append(str(i))

tPrecisions = []
precision = MIN_PRECISION
while precision <= MAX_PRECISION:
    tPrecisions.append(str(precision))
    precision = precision * 10

# Headers
print 'n,size,threads,precision,iterations,exectime'
# Run the program
runCount = 0
for i in range(NUM_TESTS):
    for a in range(len(tArrays)):
        for t in tThreads:
            for p in tPrecisions:
                command =  list(itertools.chain([
                        './relax.out',
                        tSizes[a],
                        t,
                        p],
                        tArrays[a]))
                output = subprocess.Popen(
                        command,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
                printInfo(runCount, tSizes[a], t, p, output)
                runCount += 1
