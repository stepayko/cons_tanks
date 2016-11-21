#include <stdio.h>  //printf etc
#include <windows.h> //console commands
#include <conio.h>  //getch
#include <time.h>   // for srand init
#include "maps.h"

#define mapSizeY 100
#define mapSizeX 125
#define blockSize 5
#define botsCount 4

short gameMatrix[mapSizeY][mapSizeX];
short currentMapMatrix[mapSizeY][mapSizeX];
int moves = 0;	//	gamer commands count
short key;		//	presed key

HANDLE hCon;

typedef enum eDirect {
   DIR_UP = 0,
   DIR_DOWN,
   DIR_LEFT,
   DIR_RIGHT,
   DIR_UNKNOWN
}eDirect;

typedef enum eMapSimbols {
    MPS_IRON = 1,
    MPS_BRICK,
    MPS_BRICK2,	//reserved
    MPS_BRICK3,	//reserved
    MPS_EMPTY,
    MPS_GRASS,
    MPS_SHOT
}eMapSimbols;

typedef struct Tank{
 short tnkNumber;
 short tnkDir;
 COORD tnkPosition;
 short color;
 short dead;
 short shotExist;
 short shotDir;
 COORD shotPosition;
 }Tank;

Tank tankPlayer;

Tank tankBot[botsCount];
short botKey[botsCount];
short botDelay[botsCount];

void drawMap (short *map);	// map print to screen &  currentMapMatrix filling
void printMapSimbol (int a);
void printTank (Tank *tnk); // draw tank on screen and put it to game matrix
int getKey(void);
int findGorWall (int y, int x);	//returns 0 if no wall in front of tank, 1 if it is
int findVertWall (int y, int x);	//returns 0 if no wall in front of tank, 1 if it is
int checkWall (Tank *tnk, int direction); //returns 0 if no wall in front of tank, 1 if it is
void moveTank (Tank *tnk, short key); //tank movement & cleaning it`s track
void prnInfoLine(int moves);
void playerProcess();
void shotProcess(Tank *tnk);
void botProcess (Tank *tnk, short tankNumber);
void createShot (Tank *tnk);
void killer (Tank *tnk);	//shoot hit processing
void setTanks (void);
void tankDestroy(Tank *tnk);
void checkWin(void);

int main()
{
    short i;
    hCon = GetStdHandle (STD_OUTPUT_HANDLE);
    system("mode con cols=125 lines=101");
    CONSOLE_CURSOR_INFO cursor;			//cursor adjustment
    cursor.dwSize = 20;		// height 1 to 100
    cursor.bVisible = 0;		//1- visible, 0 - hidden
    SetConsoleCursorInfo(hCon, &cursor);

    srand (time(0));

    drawMap(map1);
    setTanks();
    printTank(&tankPlayer);
    prnInfoLine(moves);

    while (1) //main game cycle
    {
        Sleep(30); // delay - 30 ms (33 FPS)
        playerProcess();
        shotProcess(&tankPlayer);
        for (i=0; i < botsCount; i++)
            botProcess(&tankBot[i], i);
        checkWin();
    }
}


void playerProcess(void)
{
    if (kbhit() == 0)	//if key was not pressed - return
        return;
    key = getKey();
    moveTank(&tankPlayer, key);
    printTank(&tankPlayer);
    moves++;
    prnInfoLine(moves);
}

void drawMap (short *map)   // map print to screen &  currentMapMatrix filling
{
    int i, j, mapElement = 0, count = 1, line = 0;
    system("cls");
    COORD mapStart = {0, 0};
    SetConsoleCursorPosition (hCon,mapStart);

    for (i=0; i < mapSizeY; i++)
    {
        for (j=0; j < mapSizeX; j++)
        {
            currentMapMatrix[i][j] = gameMatrix[i][j] = map[mapElement];  // filling currentMapMatrix & gameMatrix from map array
            printMapSimbol(map[mapElement]);	// prints map to screen
            count++;

            if (count % blockSize == 0)		//next map block
            mapElement++;

            if (count % mapSizeX == 0){
                line++;
                if (line % blockSize != 0) //repeat map line
                    mapElement -= mapSizeX/blockSize;
            }
            count++;
        }
    }
}

void printMapSimbol (int a)
{
    switch (a)
    {
        case MPS_EMPTY:
            printf(" ");
            break;
        case MPS_IRON:
            printf("%c",219);
            break;
        case MPS_GRASS:
            printf("%c",249);
            break;
        case MPS_BRICK:
            printf("%c",177);
            break;
        case MPS_SHOT:
            printf("*");
            break;

        default:
            printf(" ");
            break;
    }
}

void printTank (Tank *tnk)
{	//draw tank on screen and put it to game matrix
    short i, j;
    SetConsoleTextAttribute(hCon, tnk->color);
    COORD tmp = tnk->tnkPosition;
    tmp.Y -= 2;	//moveingcursor position from tank center to top left corner to draw it
    tmp.X -= 2;


    for (i=0; i < blockSize; i++) //put tank in game matrix
    {
        for(j=0; j < blockSize; j++)
        {
            gameMatrix[tmp.Y + i][tmp.X + j] = tnk->tnkNumber;
            if (tnk->dead != 0)
                   gameMatrix[tmp.Y + i][tmp.X + j] = MPS_EMPTY;
        }
    }

    switch (tnk->tnkDir)
    {
        case DIR_UP:	//UP
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",201,205,186,205,187);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",186,218,186,191,186);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",186,192,208,217,186);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c###%c",186,186);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",200,205,205,205,188);
            break;

        case DIR_DOWN:	//DOWN
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",201,205,205,205,187);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c###%c",186,186);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",186,218,210,191,186);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",186,192,186,217,186);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",200,205,186,205,188);
            break;

        case DIR_LEFT: //LEFT
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",201,205,205,205,187);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c#%c",186,218,191,186);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c#%c",205,205,181,186);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c#%c",186,192,217,186);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",200,205,205,205,188);
            break;

        case DIR_RIGHT: //RIGHT
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",201,205,205,205,187);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c#%c%c%c",186,218,191,186);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c#%c%c%c",186,198,205,205);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c#%c%c%c",186,192,217,186);
            tmp.Y++;
            SetConsoleCursorPosition (hCon,tmp);
            printf("%c%c%c%c%c",200,205,205,205,188);
            break;
    }
    SetConsoleTextAttribute(hCon, 7);	//standart console text color
}

int getKey(void)
{
    switch(getch())
    {
        case 224:
            switch	(getch())
            {
                case 72:
                    return DIR_UP;
                case 80:
                     return DIR_DOWN;
                case 75:
                     return DIR_LEFT;
                case 77:
                     return DIR_RIGHT;

                default:
                    return DIR_UNKNOWN;
            }
        case 32:	//space
            createShot(&tankPlayer);
            return DIR_UNKNOWN;

        case 27:	//escape
            system("cls");
            system("mode con cols=80 lines=30");
            printf("\n\n\n\n                           !!!  THANK YOU FOR PLAYING  !!!\n\n\n\n ");
            exit(0);
            break;
        case 49:	// key 1
            setTanks ();
            tankPlayer.tnkPosition.X = 67;
            tankPlayer.tnkPosition.Y = 9;
            drawMap(map1);
            printTank(&tankPlayer);
            prnInfoLine(moves);
            break;
        case 50:	// key 2
            setTanks ();
            tankPlayer.tnkPosition.X = 62;
            tankPlayer.tnkPosition.Y = 97;
            drawMap(map2);
            printTank(&tankPlayer);
            prnInfoLine(moves);
            break;
        case 51:	// key 3
            setTanks ();
            tankPlayer.tnkPosition.X = 57;
            tankPlayer.tnkPosition.Y = 22;
            drawMap(map3);
            printTank(&tankPlayer);
            prnInfoLine(moves);
            break;

        default:
            return DIR_UNKNOWN;
     }
}

int findGorWall (int y, int x) //returns 0 if no wall in front of tank, 1 if it is
{
    int i;
    for (i=0;i<blockSize;i++){
        if(gameMatrix[y][x+i]<5)
            return(1);
    }
    return (0);
}

int findVertWall (int y, int x) //returns 0 if no wall in front of tank, 1 if it is
{
    int i;
    for (i=0;i<blockSize;i++)
    {
        if(gameMatrix[y+i][x]<5)
            return(1);
    }
    return (0);
}

int checkWall (Tank *tnk, int direction) { //returns 0 if no wall in front of tank, 1 if it is
    COORD tmp = tnk->tnkPosition;
    switch (direction)
    {
        case DIR_UP:
            tmp.Y = tnk->tnkPosition.Y - 3;
            tmp.X = tnk->tnkPosition.X - 2;
            if (tmp.Y < 0)	//checks screen limits
                return (1);
            if (findGorWall(tmp.Y, tmp.X) == 0)
                return(0);
            break;
        case DIR_DOWN:
            tmp.Y = tnk->tnkPosition.Y + 3;
            tmp.X = tnk->tnkPosition.X - 2;
            if (tmp.Y > mapSizeY - 1)	//checks screen limits
                return (1);
            if (findGorWall(tmp.Y, tmp.X) == 0)
                return(0);
            break;
        case DIR_LEFT:
            tmp.Y = tnk->tnkPosition.Y - 2;
            tmp.X = tnk->tnkPosition.X - 3;
            if (tmp.X < 0)	//checks screen limits
                return (1);
            if (findVertWall(tmp.Y, tmp.X) == 0)
                return(0);
            break;
        case DIR_RIGHT:
            tmp.Y = tnk->tnkPosition.Y - 2;
            tmp.X = tnk->tnkPosition.X + 3;
            if (tmp.X > mapSizeX - 1)	//checks screen limits
                return (1);
            if (findVertWall(tmp.Y, tmp.X) == 0)
                return(0);
            break;
    }
    return(1);
}

void moveTank (Tank *tnk, short key)
{
    short i;
    if (tnk->dead != 0 || key > 3)//if tank is dead or key is not a direction
        return;

 //   if (key != tnk->tnkDir){	//rotation on one place allow
          tnk->tnkDir = key;	//tank rotation
 //       return;
 //   }

    if  (checkWall(tnk, key) == 1) // if wall in front of tank
        return;

    COORD tmp = tnk->tnkPosition;
    switch (key)
    {
        case DIR_UP:
            tmp.X -= 2;	// cursor move to clean tank track
            tmp.Y += 2 ;
            SetConsoleCursorPosition (hCon, tmp);
            for (i = 0; i < blockSize; i++){
                printMapSimbol(currentMapMatrix[tmp.Y][tmp.X]); // cleaning tank track on screen
                gameMatrix[tmp.Y][tmp.X] = currentMapMatrix[tmp.Y][tmp.X];	// cleaning tank track on screen
                tmp.X++;	//next
            }
            tnk->tnkPosition.Y--;		//moves tank position UP
            break;

        case DIR_DOWN:
            tmp.X -= 2;
            tmp.Y -= 2;
            SetConsoleCursorPosition (hCon, tmp);
            for (i = 0; i < blockSize; i++){
                printMapSimbol(currentMapMatrix[tmp.Y][tmp.X]);
                gameMatrix[tmp.Y][tmp.X] = currentMapMatrix[tmp.Y][tmp.X];
                tmp.X++;
            }
            tnk->tnkPosition.Y++;	//moves tank position DOWN
            break;

        case DIR_LEFT:
            tmp.X += 2;
            tmp.Y -= 2;
            for (i = 0; i<blockSize; i++){
                SetConsoleCursorPosition (hCon, tmp);
                printMapSimbol(currentMapMatrix[tmp.Y][tmp.X]);
                gameMatrix[tmp.Y][tmp.X] = currentMapMatrix[tmp.Y][tmp.X];
                tmp.Y++;
            }
            tnk->tnkPosition.X--;	//moves tank position LEFT
            break;

        case DIR_RIGHT:
            tmp.X -= 2;	//смещаем курсор чтоб затереть зад танка
            tmp.Y -= 2;
            for (i = 0; i < blockSize; i++){
                SetConsoleCursorPosition (hCon, tmp);
                printMapSimbol(currentMapMatrix[tmp.Y][tmp.X]);
                gameMatrix[tmp.Y][tmp.X] = currentMapMatrix[tmp.Y][tmp.X];
                tmp.Y++;
            }

            tnk->tnkPosition.X++; //moves tank position RIGHT
            break;
    }
}

void prnInfoLine(int moves)
{
    SetConsoleTextAttribute(hCon, 236);	//console text color change
    COORD infoLine = {10, mapSizeY};
    SetConsoleCursorPosition (hCon, infoLine);
    printf("USE ARROWS TO MOVE. SPACE TO FIRE. PUSH 1-3 TO CHANGE MAP. ESC-EXIT   Position %3d,%3d moves%5d"
        ,tankPlayer.tnkPosition.X, tankPlayer.tnkPosition.Y, moves);
    SetConsoleTextAttribute(hCon, 7);	//console text color return to default
}

void shotProcess(Tank *tnk)
{
    if (tnk->shotExist == 0)
        return;
    if (tnk->dead != 0)
    	return;
    SetConsoleTextAttribute(hCon, tnk->color);
    COORD tmp = tnk->shotPosition;
    switch(tnk->shotDir)
    {
        case DIR_UP:
            if (gameMatrix[tmp.Y][tmp.X] < 5 || tmp.Y < 0){	//if wall, tank or screen end
                tnk->shotExist = 0;
                if (tmp.Y >= 0)
                    killer(tnk);
            }
            else
            {
                SetConsoleCursorPosition(hCon, tnk->shotPosition); //draw bullet
                printMapSimbol(MPS_SHOT);
                tnk->shotPosition.Y--;	//move bullet
            }

            SetConsoleTextAttribute(hCon, 7);
            tmp.Y++;	// clear prev frame bullet
            if (tmp.Y == tnk->tnkPosition.Y-2) //but not tank
                break;
            SetConsoleCursorPosition(hCon, tmp);
            printMapSimbol(currentMapMatrix[tmp.Y][tmp.X]); // clear prev frame bullet
            break;

        case DIR_DOWN:
            if (gameMatrix[tmp.Y][tmp.X] < 5 || tmp.Y == mapSizeY){
                tnk->shotExist = 0;
                if (tmp.Y < mapSizeY)
                    killer(tnk);
            }
            else
            {
                SetConsoleCursorPosition(hCon,tnk->shotPosition);
                printMapSimbol(MPS_SHOT);
                tnk->shotPosition.Y++;
            }
            SetConsoleTextAttribute(hCon, 7);
            tmp.Y--;
            if (tmp.Y == tnk->tnkPosition.Y + 2)
                break;
            SetConsoleCursorPosition(hCon, tmp);
            printMapSimbol(currentMapMatrix[tmp.Y][tmp.X]);
            break;

        case DIR_LEFT:
            if (gameMatrix[tmp.Y][tmp.X] < 5 || tmp.X < 0){
                tnk->shotExist = 0;
                if (tmp.X >= 0)
                    killer(tnk);
            }
            else
            {
                SetConsoleCursorPosition(hCon,tnk->shotPosition);
                printMapSimbol(MPS_SHOT);
                tnk->shotPosition.X--;
            }
            SetConsoleTextAttribute(hCon, 7);
            tmp.X++;
            if (tmp.X == tnk->tnkPosition.X - 2)
                break;
            SetConsoleCursorPosition(hCon, tmp);
            printMapSimbol(currentMapMatrix[tmp.Y][tmp.X]);
            break;

        case DIR_RIGHT:
            if (gameMatrix[tmp.Y][tmp.X] < 5 || tmp.X == mapSizeX){
                tnk->shotExist = 0;
                if (tmp.X < mapSizeX)
                    killer(tnk);
            }
            else{
                SetConsoleCursorPosition(hCon, tnk->shotPosition);
                printMapSimbol(MPS_SHOT);
                tnk->shotPosition.X++;
            }
            SetConsoleTextAttribute(hCon, 7);
            tmp.X--;
            if (tmp.X==tnk->tnkPosition.X+2)
                break;
            SetConsoleCursorPosition(hCon,tmp);
            printMapSimbol(currentMapMatrix[tmp.Y][tmp.X]);
        break;
    }
}

void botProcess (Tank *tnk, short tankNumber)
{
    if (tnk->dead != 0)
        return;
    if (botDelay[tankNumber]%(12 + (tankNumber*3)) == 0) 	// direction change
    {
        botKey[tankNumber] = rand() % 4;
        botDelay[tankNumber] = 0;
        tnk->tnkDir = botKey[tankNumber];
    }

    if (checkWall(tnk, tnk->tnkDir) == 1)  //if wall - change direction
        tnk->tnkDir = rand() % 4;

    if (botDelay[tankNumber] % 3 == 0  && tnk->shotExist == 0)	//shooting
        createShot(tnk);

    shotProcess(tnk);
    moveTank (tnk, botKey[tankNumber]);
    printTank (tnk);

    botDelay[tankNumber]++;
}

void createShot (Tank *tnk)
{
    if (tnk->shotExist != 0)
        return;
    tnk->shotExist = 1;
    tnk->shotDir = tnk->tnkDir;
    tnk->shotPosition = tnk->tnkPosition;	//put bullet to tank position
    switch (tnk->shotDir){	//move bullet out of tank body
        case DIR_UP:
            tnk->shotPosition.Y = tnk->tnkPosition.Y - 3;
            break;
        case DIR_DOWN:
            tnk->shotPosition.Y = tnk->tnkPosition.Y + 3;
            break;
        case DIR_LEFT:
            tnk->shotPosition.X = tnk->tnkPosition.X - 3;
            break;
        case DIR_RIGHT:
            tnk->shotPosition.X = tnk->tnkPosition.X + 3;
            break;
    }
}

void killer (Tank *tnk)
{
    short hitObj = gameMatrix[tnk->shotPosition.Y][tnk->shotPosition.X];
    short x = tnk->shotPosition.X, y = tnk->shotPosition.Y, i;
    COORD tmp;
    tmp.X = x;
    tmp.Y = y;
    SetConsoleTextAttribute(hCon, 7);
    switch (hitObj) {
        case MPS_IRON:
            break;
        case MPS_BRICK:
            if (tnk->shotDir == DIR_UP || tnk->shotDir == DIR_DOWN){ //if this is vertical shoot
                tmp.X -= 2;
                for(i=0; i < blockSize; i++)
                {
                    if (gameMatrix[y][x-2+i] == MPS_BRICK)
                    {
                        gameMatrix[y][x-2+i]=currentMapMatrix[y][x-2+i]=MPS_EMPTY;
                        SetConsoleCursorPosition(hCon,tmp);
                        printMapSimbol(currentMapMatrix[y][x-2+i]);
                    }
                    tmp.X++;
                }
            }
            else    // horisontal shoot
            {
                tmp.Y -= 2;
                for(i = 0; i < blockSize; i++)
                {
                    if (gameMatrix[y-2+i][x] == MPS_BRICK)
                    {
                        gameMatrix[y-2+i][x] = currentMapMatrix[y-2+i][x] = MPS_EMPTY;
                        SetConsoleCursorPosition(hCon,tmp);
                        printMapSimbol(currentMapMatrix[y-2+i][x]);
                    }
                    tmp.Y++;
                }
            }
            break;
        case -10: // -10 is player tank
            tankDestroy(&tankPlayer);
            SetConsoleTextAttribute(hCon, 12);
            drawMap (gameOver);
            COORD tmp;
            tmp.X = 31;
            tmp.Y = 47;
            SetConsoleCursorPosition(hCon,tmp);
            SetConsoleTextAttribute(hCon,233);
            printf("YOU CAN RESTART GAME BY PRESSING 1,2,3 OR HIT ESC TO EXIT");
            SetConsoleTextAttribute(hCon,7);
            getKey();
            break;
    }
    if (hitObj <= 0 && hitObj !=- 10)   //bot tank
            tankDestroy(&tankBot[hitObj*-1]);

}

void setTanks (void)
{
    tankPlayer.tnkNumber = -10;
    tankPlayer.tnkPosition.X = 67;
    tankPlayer.tnkPosition.Y = 9;
    tankPlayer.tnkDir = 0;
    tankPlayer.color = 42;
    tankPlayer.shotExist = 0;
    tankPlayer.dead = 0;

    tankBot[0].tnkNumber = 0;
    tankBot[0].tnkPosition.X = 7;
    tankBot[0].tnkPosition.Y = 7;
    tankBot[0].tnkDir = 1;
    tankBot[0].color = 12;
    tankBot[0].shotExist = 0;
    tankBot[0].dead = 0;

    tankBot[1].tnkNumber =-1;
    tankBot[1].tnkPosition.X = 117;
    tankBot[1].tnkPosition.Y = 92;
    tankBot[1].tnkDir = 3;
    tankBot[1].color = 9;
    tankBot[1].shotExist = 0;
    tankBot[1].dead = 0;

    tankBot[2].tnkNumber = -2;
    tankBot[2].tnkPosition.X = 7;
    tankBot[2].tnkPosition.Y = 92;
    tankBot[2].tnkDir = 2;
    tankBot[2].color = 11;
    tankBot[2].shotExist = 0;
    tankBot[2].dead = 0;

    tankBot[3].tnkNumber = -3;
    tankBot[3].tnkPosition.X = 117;
    tankBot[3].tnkPosition.Y = 7;
    tankBot[3].tnkDir = 0;
    tankBot[3].color = 14;
    tankBot[3].shotExist = 0;
    tankBot[3].dead = 0;
}

void tankDestroy(Tank *tnk)
{
    short i;
    tnk->dead = 1;
    for(i = 0; i < 10; i++){
        tnk->color = 192;
        if (i%2 == 0)
            tnk->color = 76;
        printTank(tnk);
        Sleep(30);
    }
    tnk->color = 8;
    printTank(tnk);
}

void checkWin(void)
{
    short i;
    for (i = 0; i < botsCount; i++)
    {
        if (tankBot[i].dead == 0)
            return;
    }
    SetConsoleTextAttribute(hCon, 10);
    drawMap(win);
    COORD tmp;
    tmp.X = 36;
    tmp.Y = 46;
    SetConsoleCursorPosition(hCon, tmp);
    SetConsoleTextAttribute(hCon, 233);
    printf("YOU CAN RESTART GAME BY PRESSING 1,2,3 OR HIT ESC TO EXIT");
    SetConsoleTextAttribute(hCon, 7);
    getKey();
}
