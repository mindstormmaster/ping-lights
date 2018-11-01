from sense_hat import SenseHat
from multiping import MultiPing
import datetime
import time
import math
import atexit

@atexit.register
def goodbye():
    for x in range(0, 8):
        for y in range(0, 8):
            sense.set_pixel(x, y, 0, 0, 0)


BRIGHTNESS = 100
ERR_BRIGHTNESS = 150
PING_EXPECTED = 15
PING_MAX = 50
PURPLE = (100, 0, 100)
DARK = (0, 0, 0)

BLOCKOUTS = []
BLOCKOUTS.append(datetime.time(11, 0))
BLOCKOUTS.append(datetime.time(11, 55))

print ("blockouts: %s" % BLOCKOUTS)

sense = SenseHat()
sense.set_rotation(90)

# initialize results list
results = []
for i in range(0, 64):
    results.append(-1)


# green to red gradient
def PingWheel(ping, blinkCounter):
    WheelPos = ((ping - PING_EXPECTED) * (BRIGHTNESS / (PING_MAX - PING_EXPECTED)));

    if ping < 0:
        # ping return -1 on error
        return (0, 0, BRIGHTNESS)
    elif ping == 0:
        # ping returns 0 on timeout
        if blinkCounter % 2 == 0:
            return (ERR_BRIGHTNESS, 0, 0)
        else:
            return (5, 0, 0)
    elif WheelPos < 0:
        return (0, BRIGHTNESS, 0)
    elif WheelPos <= BRIGHTNESS:
        return (WheelPos, BRIGHTNESS-WheelPos, 0)
    else:
        return (ERR_BRIGHTNESS, 0, 0)

def timeUntilBlockout():
    now = datetime.datetime.now()
    for blockoutTime in BLOCKOUTS:
        blockout = datetime.datetime.now().replace(hour=blockoutTime.hour, minute=blockoutTime.minute, second=blockoutTime.second, microsecond=0)
        totalSeconds = (blockout - now).total_seconds()
        if totalSeconds < 16 * 60 and totalSeconds >= 0:
            # starting ten minutes before stated time
            return (blockout - now).total_seconds()
        elif totalSeconds < 0 and totalSeconds > -5 * 60:
            # and five minutes after the stated time
            return 0
    return -1

def displayResults():
    pixels = []
    for i, v in enumerate(results):
        pixels.append(PingWheel(v, 0))
    
    secsUntilBlockout = timeUntilBlockout()
    print ("secsUntilBlockout %s" % secsUntilBlockout)
    if secsUntilBlockout >= 0:
        # one warning per two minutes
        warningsToDisplay = 8 - math.floor(secsUntilBlockout / 60 / 2)
        print ("warningsToDisplay %s" % warningsToDisplay)
        for i in range(0, warningsToDisplay):
            pixels[i] = PURPLE
            pixels[i+8] = PURPLE
        for i in range(warningsToDisplay, 8):
            pixels[i] = DARK
            pixels[i+8] = DARK

    for i, v in enumerate(pixels):
        x = (i % 8)
        y = math.floor(i / 8)
        sense.set_pixel(x, y, v)


def ping():
    mp = MultiPing(['8.8.8.8'])
    mp.send()
    responses, no_responses = mp.receive(0.5)
    if '8.8.8.8' in responses:
        results.append(responses['8.8.8.8'])
    else:
        results.append(0)
    
    if len(results) > 64:
        results.pop(0)

    displayResults()

    time.sleep(0.3)
    sense.set_pixel(7, 7, DARK)

    now = datetime.time()
    microsecond_delay = 1000000 - now.microsecond
    time.sleep(microsecond_delay / 1000000)

while True:
    ping()
