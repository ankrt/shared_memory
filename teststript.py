import os
import random
import time

ARRAY_MIN_SIZE = 50
ARRAY_MAX_SIZE = 1001
ARRAY_INCREMENT = 50

MIN_PRECISION = 10
MAX_PRECISION = 1000

MIN_THREADS = 1
MAX_THREADS = 2

random.seed()

def genNums(size):
    numlist = ''
    for i in range(size * size):
        numlist += str(random.randint(0, 10)) + ' '
    return numlist

def printInfo(count, size, threads, precision):
    print "N: ", count
    print "\tsize:\t\t", size
    print "\tthreads:\t", threads
    print "\tprecision:\t", precision


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


# Run the program
runCount = 0
for a in range(len(tArrays)):
    for t in tThreads:
        for p in tPrecisions:
            printInfo(runCount, tSizes[a], t, p)
            os.system('./relax.out ' + tSizes[a] + ' ' + t + ' ' + p + ' ' + tArrays[a])
            #print './relax.out ' , tSizes[a] , t , p , tArrays[a]
            runCount += 1
            print ""
