# Wormy (a Nibbles clone)
# By Al Sweigart al@inventwithpython.com
# http://inventwithpython.com/pygame
# Released under a "Simplified BSD" license

#KRT 14/06/2012 modified Start Screen and Game Over screen to cope with mouse events
#KRT 14/06/2012 Added a non-busy wait to Game Over screen to reduce processor loading from near 100%
import random, pygame, sys, smbus, time
from pygame.locals import *

FPS = 15
MULTIPLIER = 18
WINDOWWIDTH = 96*MULTIPLIER
WINDOWHEIGHT = 54*MULTIPLIER
CELLSIZE = 3*MULTIPLIER
assert WINDOWWIDTH % CELLSIZE == 0, "Window width must be a multiple of cell size."
assert WINDOWHEIGHT % CELLSIZE == 0, "Window height must be a multiple of cell size."
CELLWIDTH = int(WINDOWWIDTH / CELLSIZE)
CELLHEIGHT = int(WINDOWHEIGHT / CELLSIZE)


#             R    G    B
WHITE     = (255, 255, 255)
BLACK     = (  0,   0,   0)
RED       = (255,   0,   0)
DARKRED   = (155,   0,   0)
GREEN     = (  0, 255,   0)
DARKGREEN = (  0, 155,   0)
DARKGRAY  = ( 40,  40,  40)
BGCOLOR = BLACK

UP = 'up'
DOWN = 'down'
LEFT = 'left'
RIGHT = 'right'

HEAD = 0 # syntactic sugar: index of the worm's head

def main():
    global FPSCLOCK, DISPLAYSURF, BASICFONT, flamecell, bus_data, X, Y

    pygame.init()
    FPSCLOCK = pygame.time.Clock()
    DISPLAYSURF = pygame.display.set_mode((WINDOWWIDTH, WINDOWHEIGHT))
    BASICFONT = pygame.font.Font('freesansbold.ttf', 18)
    pygame.display.set_caption('Snake- Qwiic Joystick')
    flamefile = 'Pixel-flame.png'
    flame = pygame.image.load(flamefile).convert_alpha()
    flamecell = pygame.transform.scale(flame,(CELLSIZE,CELLSIZE))
    
    showStartScreen()
    while True:
        runGame()
        showGameOverScreen()


def runGame():
    global bus_data, X, Y
    
    ## I2C bus start
    bus = smbus.SMBus(1)
    addr = 0x20
    
    
    # Set a random start point.
    startx = random.randint(5, CELLWIDTH - 6)
    starty = random.randint(5, CELLHEIGHT - 6)
    wormCoords = [{'x': startx,     'y': starty},
                  {'x': startx - 1, 'y': starty},
                  {'x': startx - 2, 'y': starty}]
    direction = RIGHT

    # Start the apple in a random place.
    apple = getRandomLocation()


    while True: # main game loop
        for event in pygame.event.get(): # event handling loop          
            if event.type == QUIT:
                terminate()
            elif event.type == KEYDOWN:
                if (event.key == K_LEFT or event.key == K_a) and direction != RIGHT:
                    direction = LEFT
                elif (event.key == K_RIGHT or event.key == K_d) and direction != LEFT:
                    direction = RIGHT
                elif (event.key == K_UP or event.key == K_w) and direction != DOWN:
                    direction = UP
                elif (event.key == K_DOWN or event.key == K_s) and direction != UP:
                    direction = DOWN
                elif event.key == K_ESCAPE:
                    terminate()
                    
        # Reads Qwiic Joystick
        try:
            bus_data = bus.read_i2c_block_data(addr, 0x03, 5)
        except Exception as e:
            print(e)
        X = (bus_data[0]<<8 | bus_data[1])>>6
        Y = (bus_data[2]<<8 | bus_data[3])>>6
        if X < 450:
            direction = RIGHT
        elif 575 < X:
            direction = LEFT
        elif Y < 450:
            direction = DOWN
        elif 575 < Y:
            direction = UP
        

        # check if the worm has hit itself or the edge
        if wormCoords[HEAD]['x'] == -1 or wormCoords[HEAD]['x'] == CELLWIDTH or wormCoords[HEAD]['y'] == -1 or wormCoords[HEAD]['y'] == CELLHEIGHT:
            return # game over
        for wormBody in wormCoords[1:]:
            if wormBody['x'] == wormCoords[HEAD]['x'] and wormBody['y'] == wormCoords[HEAD]['y']:
                return # game over

        # check if worm has eaten an apply
        if wormCoords[HEAD]['x'] == apple['x'] and wormCoords[HEAD]['y'] == apple['y']:
            # don't remove worm's tail segment
            apple = getRandomLocation() # set a new apple somewhere
        else:
            del wormCoords[-1] # remove worm's tail segment


        # move the worm by adding a segment in the direction it is moving
        if direction == UP:
            newHead = {'x': wormCoords[HEAD]['x'], 'y': wormCoords[HEAD]['y'] - 1}
        elif direction == DOWN:
            newHead = {'x': wormCoords[HEAD]['x'], 'y': wormCoords[HEAD]['y'] + 1}
        elif direction == LEFT:
            newHead = {'x': wormCoords[HEAD]['x'] - 1, 'y': wormCoords[HEAD]['y']}
        elif direction == RIGHT:
            newHead = {'x': wormCoords[HEAD]['x'] + 1, 'y': wormCoords[HEAD]['y']}
        wormCoords.insert(0, newHead)
        DISPLAYSURF.fill(BGCOLOR)
        drawGrid()
        drawWorm(wormCoords)
        drawApple(apple)
        drawScore(len(wormCoords) - 3)
        pygame.display.update()
        FPSCLOCK.tick(FPS)

def drawPressKeyMsg():
    pressKeySurf = BASICFONT.render('Press a key to play.', True, DARKGRAY)
    pressKeyRect = pressKeySurf.get_rect()
    pressKeyRect.topleft = (WINDOWWIDTH - 20*MULTIPLIER, WINDOWHEIGHT - 3*MULTIPLIER)
    DISPLAYSURF.blit(pressKeySurf, pressKeyRect)



# KRT 14/06/2012 rewrite event detection to deal with mouse use
def checkForKeyPress():
    global bus_data
    
    for event in pygame.event.get():
        if event.type == QUIT:      #event is quit 
            terminate()
        elif event.type == KEYDOWN:
            if event.key == K_ESCAPE:   #event is escape key
                terminate()
            else:
                return event.key   #key found return with it
    # no quit or key events in queue so return None
    
    ## I2C bus start
    bus = smbus.SMBus(1)
    addr = 0x20
    
    # Reads Qwiic Joystick
    try:
        bus_data = bus.read_i2c_block_data(addr, 0x03, 5)
    except Exception as e:
        print(e)
    
    if bus_data[4] == 0:
        return True
    
    return None

    
def showStartScreen():
    #titleFont = pygame.font.Font('freesansbold.ttf', 10*MULTIPLIER)
    #titleSurf1 = titleFont.render('Wormy!', True, WHITE, DARKGREEN)
    #titleSurf2 = titleFont.render('Wormy!', True, GREEN)

    #degrees1 = 0
    #degrees2 = 0
    
#KRT 14/06/2012 rewrite event detection to deal with mouse use
    pygame.event.get()  #clear out event queue
    
    while True:
        #DISPLAYSURF.fill(BGCOLOR)
        #rotatedSurf1 = pygame.transform.rotate(titleSurf1, degrees1)
        #rotatedRect1 = rotatedSurf1.get_rect()
        #rotatedRect1.center = (WINDOWWIDTH / 2, WINDOWHEIGHT / 2)
        #DISPLAYSURF.blit(rotatedSurf1, rotatedRect1)

        #rotatedSurf2 = pygame.transform.rotate(titleSurf2, degrees2)
        #rotatedRect2 = rotatedSurf2.get_rect()
        #rotatedRect2.center = (WINDOWWIDTH / 2, WINDOWHEIGHT / 2)
        #DISPLAYSURF.blit(rotatedSurf2, rotatedRect2)

        drawPressKeyMsg()
#KRT 14/06/2012 rewrite event detection to deal with mouse use
        if checkForKeyPress():
            return
        pygame.display.update()
        FPSCLOCK.tick(FPS)
        #degrees1 += 3 # rotate by 3 degrees each frame
        #degrees2 += 7 # rotate by 7 degrees each frame


def terminate():
    pygame.quit()
    sys.exit()


def getRandomLocation():
    return {'x': random.randint(0, CELLWIDTH - 1), 'y': random.randint(0, CELLHEIGHT - 1)}


def showGameOverScreen():
    gameOverFont = pygame.font.Font('freesansbold.ttf', 15*MULTIPLIER)
    gameSurf = gameOverFont.render('Game', True, WHITE)
    overSurf = gameOverFont.render('Over', True, WHITE)
    gameRect = gameSurf.get_rect()
    overRect = overSurf.get_rect()
    gameRect.midtop = (WINDOWWIDTH / 2, 10)
    overRect.midtop = (WINDOWWIDTH / 2, gameRect.height + 10 + 25)

    DISPLAYSURF.blit(gameSurf, gameRect)
    DISPLAYSURF.blit(overSurf, overRect)
    drawPressKeyMsg()
    pygame.display.update()
    pygame.time.wait(500)
#KRT 14/06/2012 rewrite event detection to deal with mouse use
    pygame.event.get()  #clear out event queue 
    while True:
        if checkForKeyPress():
            return
#KRT 12/06/2012 reduce processor loading in gameover screen.
        pygame.time.wait(100)

def drawScore(score):
    scoreSurf = BASICFONT.render('Score: %s' % (score), True, WHITE)
    scoreRect = scoreSurf.get_rect()
    scoreRect.topleft = (WINDOWWIDTH - 12*MULTIPLIER, 1*MULTIPLIER)
    DISPLAYSURF.blit(scoreSurf, scoreRect)


def drawWorm(wormCoords):
    for coord in wormCoords:
        x = coord['x'] * CELLSIZE
        y = coord['y'] * CELLSIZE
        wormSegmentRect = pygame.Rect(x, y, CELLSIZE, CELLSIZE)
        pygame.draw.rect(DISPLAYSURF, DARKRED, wormSegmentRect)
        wormInnerSegmentRect = pygame.Rect(x + 4, y + 4, CELLSIZE - 8, CELLSIZE - 8)
        pygame.draw.rect(DISPLAYSURF, RED, wormInnerSegmentRect)


def drawApple(coord):
    x = coord['x'] * CELLSIZE
    y = coord['y'] * CELLSIZE
    DISPLAYSURF.blit(flamecell, (x,y))


def drawGrid():
    for x in range(0, WINDOWWIDTH, CELLSIZE): # draw vertical lines
        pygame.draw.line(DISPLAYSURF, DARKGRAY, (x, 0), (x, WINDOWHEIGHT))
    for y in range(0, WINDOWHEIGHT, CELLSIZE): # draw horizontal lines
        pygame.draw.line(DISPLAYSURF, DARKGRAY, (0, y), (WINDOWWIDTH, y))


if __name__ == '__main__':
    main()
