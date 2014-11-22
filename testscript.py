import itertools
import random
#import subprocess
import os
import sys
import time

ARRAY_MIN_SIZE = 200
ARRAY_MAX_SIZE = 5001
ARRAY_INCREMENT = 20

MIN_PRECISION = 10
MAX_PRECISION = 10

MIN_THREADS = 1
MAX_THREADS = 9

NUM_TESTS = 10

random.seed()

def genNums(size):
    numlist = ''
    for i in range(size * size):
        #numlist += str(random.randint(0, 10))
        numlist += str(random.randint(0, 1))
    return numlist

def printInfo(count, size, threads, precision, output, et):
    outputList = output.split()
    print count, ',',
    print size, ',',
    print threads, ',',
    print precision, ',',
    #print outputList[0], ',',
    print et

#def printOutput(output):
    #for line in output.stdout:
        #sys.stdout.write(line)

tArrays = []
tSizes = []
for size in range(ARRAY_MIN_SIZE, ARRAY_MAX_SIZE, ARRAY_INCREMENT):
    #tArrays.append(genNums(size))
    tArrays.append(1)
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
print 'n,size,threads,precision,walltime'
# Run the program
runCount = 0
for a in range(len(tArrays)):
    for t in tThreads:
        for p in tPrecisions:
            t_cumulative = 0
            for i in range(NUM_TESTS):
                command =  ' '.join(list(itertools.chain([
                        './relax.out',
                        tSizes[a],
                        t,
                        p],
                        #tArrays[a],
                        )))
                t_start = time.time();
                output = os.popen(command).read()
                t_end = time.time();
                t_elapsed = t_end - t_start
                t_cumulative += t_elapsed
            t_average = t_cumulative / NUM_TESTS
            printInfo(runCount, tSizes[a], t, p, output, t_average)
            runCount += 1
