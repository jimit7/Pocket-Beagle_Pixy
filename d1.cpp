
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <iostream>
#include <pixy.h>                               // camera Library
 
#include <time.h>
#include <wiringPi.h>                   //WiringPi Library for GPIO access
 
 
using namespace std;
 
// Flikka Initialisation Function Prototypes //
void initPixy(void);
void initGPIO(void);
 
// FLIKKA BEAN FUNCTION PROTOTYPES //
void kickBall(void);
void unKickBall(void);
void goalieMove(int stepsToMove);
void distToMatch(void);
 
void fieldRep(void);
void fieldPrint(void);
 
void ballTrackGoalie(void);
void goalieKick(void);
 
void rampForKick(void);
void spinMotor(void);
 
void xVelocity(void);
 
#define kPULSE 0        //PIN that sends LINEAR MOVEMENT PULSE to driver
#define kDIR 1          //PIN that controls direction
#define lPULSE 25
#define lDIR0 2 //PIN that controls KICK direction
 
// Flikka Motor Defines //
//~ #define kPULSE 20   //KICK pulse PIN - KICK PINS 20 downto 15
//~ #define lPULSE 0    //LINEAR pulse PIN - LINEAR PINS 0 to 8
//~
//~ //** DIRECTION PINS **//
//~ //LINEAR//
//~ #define lDIR0 1             //PIN that controls LINEAR direction
//~ #define lDIR1 2
//~ #define lDIR2 3
//~ #define lDIR3 4
//~
//~ //KICK//
//~ #define kDIR 19             //PIN that controls KICK direction
                                        //~ //Common to all KICK drivers
//~
//~ //** ENABLE PINS **//
//~ //LINEAR//
//~ #define enLIN0 5
//~ #define enLIN1 6
//~ #define enLIN2 7
//~ #define enLIN3 8
//~
//~ //KICK//
//~ #define enKICK0 18          //PIN that sends KICK MOVEMENT PULSE to driver
//~ #define enKICK1 17
//~ #define enKICK2 16
//~ #define enKICK3 15
 
#define pulsesKick 50                   //pulses given to motor that kicks 200 pulses = 1 revolution
#define pulsesLin 10                    //Pulses for the Linear Motor to drive movement. 1 Full Step Mode revolution = 200 pulses = 100mm
#define mmXPerPixel 0.0038125           //Given by length of playing field divded by X resolution
 
//** LIMIT SWITCH PINS **//
#define LIMITkick 21
#define LIMITlinear 22
 
//Zone Defines // These can be used to calibrate the various zones once the camera is properly installed in its final location //
 
#define halfway 160
 
// foosGoalie DEFINEs //
#define goalieKickCalibration 5
#define goalieAxleX 25
#define goalieThirdLowerLimit 65
#define goalieThirdUpperLimit 135
#define goalieKickRangeUpperLimit 12
 
// foosDefender ZONE //
 
 
//xVelocity Declarations
//This global variable will be used based on the velocity of the ball
int goalieKickRangeInnerLimit = 25;
 
int goalieKickCoordinate;
 
//Goalie Track Varibale Declarations
int goaliePos = 100;
int pixToMove;
int distToMove;
int stepsToMove;
 
int counterVelo = 0;
 
int locXprev = 0;       //COPY previous blocks into locXprev for trajectory calculations
int locYprev = 0;
 
int pixFromGoalie;
int wait = 1000;
int defaultWait = 1000;
 
int distFromZero = 0;                                           // Used to calculate the distance the goalie has moved
int goalFLAG = 0;
int counter = 0;
int kickFlag = 0;
 
// Flikka Pixy Declarations //
int lastY = 100;
 
//      Flikka Delays   //
void delayMicrosecondsNoSleep(int);
 
#define ROW 32
#define COLUMN 21
 
 
// Pixy Block buffer //
#define BLOCK_BUFFER_SIZE    25
struct Block blocks[BLOCK_BUFFER_SIZE];
 
static bool run_flag = true;
 
void handle_SIGINT(int unused)
{
  // On CTRL+C - abort! //
 
  run_flag = false;
}
 
int main(int argc, char * argv[])
{
        {
        int kickAttempts = 0;
        int i = 0;
        initPixy();
        initGPIO();
       
        //Catch CTRL+C (SIGINT) signals //
        signal(SIGINT, handle_SIGINT);
 
 
        //below this the program is stuck in loop until pixy.close() is executed and run_flag is set low
        printf("\n**Team Flikka Bean 2015 FoosYourself Project**\nTracking ball...\n");
       
        while(run_flag)
        {
 
    // Wait for new blocks to be available //
    while(!pixy_blocks_are_new() && run_flag);
 
    // Get blocks from Pixy //
    pixy_get_blocks(BLOCK_BUFFER_SIZE, &blocks[0]);
        //~ printf("%d\n", counter++);
        printf("locX: %3d locY: %3d \n", blocks[0].x, blocks[0].y);
        printf("locXprev: %3d locYprev: %3d \n", locXprev, locYprev);
       
        xVelocity(); //also controls kicking
       
        if(blocks[0].x >= locXprev+1 | blocks[0].x <= locXprev-1){
                ballHasNotMoved++;
                if(ballHasNotMoved > 5){
                        printf("ball is stuck or in goal /n");
                }
        }
       
#if 0   //toggle to enable/disbale goalieKick program
        //~ if(blocks[0].x < goalieKickCoordinate && kickAttempts < 2){
                        //~ printf("Attempt Kick %d\n", (kickAttempts+1));
                        //~ goalieKick();
                //~ }
                //~
        //~ //Did ball get kicked?
        //~ if(blocks[0].x < goalieKickCoordinate && (kickAttempts >= 1 && kickAttempts < 2)){
                        //~ kickAttempts++;
                        //~ printf("Attempting to kick again\n");
                        //~ goalieKick();
                        //~ printf("Attempts = %d\n", kickAttempts);
                        //~ }
                        //~
        //~ if(blocks[0].x < goalieKickCoordinate && kickAttempts >= 2 && locXprev == blocks[0].x){
                //~ printf("Ball is stuck or in the goal\n");
                        //~ }
        //~
        //~ if(blocks[0].x > goalieKickCoordinate && locXprev < goalieKickCoordinate){
                        //~ kickAttempts = 0;
                        //~ printf("Ball Kicked\n");
                        //~ }
 
//linear ball location matching for the goalie
 
//~ if(blocks[0].x < 260 && blocks[0].y < goalieThirdUpperLimit && blocks[0].y > goalieThirdLowerLimit && ((blocks[0].y < lastY-2) | (blocks[0].y > lastY+2))){
//~
                //~ ballTrackGoalie();
                //~ lastY = blocks[0].y;
                //~ if(blocks[0].x < goalieKickCoordinate){
                        //~ printf("goalieKick\n");
                        //~ goalieKick();
                //~ }
        //~ }
#endif 
    i++;
    locXprev = blocks[0].x;     //COPY previous blocks into locXprev for trajectory calculations
        locYprev = blocks[0].y;
  }
}
  pixy_close();
 
}
 
 
 
void xVelocity(){
        // This calculates the velocity of the ball along the x axis,
        int pixelsMoved = blocks[0].x-locXprev;
        float distanceMoved = mmXPerPixel*pixelsMoved;
        float veloX= distanceMoved*50;
       
        int xCoordinateToKick = goalieAxleX+veloX+goalieKickCalibration;
        if(xCoordinateToKick >= blocks[0].x){
                goalieKick();
        }
        //~ printf("\n************************\npixelsMoved: %d \ndistanceMoved : %f mmXPerPixel\nveloX : %f", pixelsMoved, distanceMoved, veloX);
        }
       
       
void initPixy(){
        int pixy_init_status;
        // Connect to Pixy //
        pixy_init_status = pixy_init();
 
        // Was there an error initializing pixy? //
        if(!pixy_init_status == 0)
        {
                // Error initializing Pixy //
                printf("pixy_init(): ");
                pixy_error(pixy_init_status);
 
                pixy_init_status = 1;
        }
        }
 
void initGPIO(){
        //initialise GPIO
        wiringPiSetup();
       
        //~ pinMode(enKICK0, OUTPUT);
        //~ pinMode(enKICK1, OUTPUT);
        //~ pinMode(enKICK2, OUTPUT);
        //~ pinMode(enKICK3, OUTPUT);
        pinMode(kPULSE, OUTPUT);
        pinMode(kDIR, OUTPUT);
        pinMode(lPULSE, OUTPUT);
        pinMode(lDIR0, OUTPUT);
       
        }
 
void goalieKick(){
        digitalWrite(kDIR, 1);
        for(int i = 0; i < pulsesKick; i++){                    //LOOP through i iterations
                        digitalWrite(kPULSE, HIGH);
                        delayMicrosecondsNoSleep(wait);
                        digitalWrite(kPULSE, LOW);
                        delayMicrosecondsNoSleep(wait);
                        if(wait > 700){
                                wait = wait-10;
                        }
                }
                       
        for(int i = 0; i < pulsesKick; i++){                    //LOOP through i iterations
                        digitalWrite(kPULSE, HIGH);
                        delayMicrosecondsNoSleep(wait);
                        digitalWrite(kPULSE, LOW);
                        delayMicrosecondsNoSleep(wait);
                        if(wait < 1000){
                                wait = wait+10;
                        }
                        }
        wait = defaultWait;
        digitalWrite(kDIR, 0);
        for(int i = 0; i < pulsesKick; i++){                    //LOOP through i iterations
                        digitalWrite(kPULSE, HIGH);
                        delayMicrosecondsNoSleep(wait);
                        digitalWrite(kPULSE, LOW);
                        delayMicrosecondsNoSleep(wait);
                        if(wait > 700){
                                wait = wait-10;
                        }
                        }
        for(int i = 0; i < pulsesKick; i++){                    //LOOP through i iterations
                        digitalWrite(kPULSE, HIGH);
                        delayMicrosecondsNoSleep(wait);
                        digitalWrite(kPULSE, LOW);
                        delayMicrosecondsNoSleep(wait);
                        if(wait > 1000){
                                wait = wait+10;
                        }
                        }
                wait = defaultWait;
                }
 
       
       
void kickBall(){
        //KICK
        kickFlag = 1;
        digitalWrite(kDIR, LOW); //ensure DIR is LOW first
        for(int i = 0; i < pulsesKick; i++){                    //LOOP through i iterations
                digitalWrite(kPULSE, HIGH);
                delayMicrosecondsNoSleep(wait);
                digitalWrite(kPULSE, LOW);
                delayMicrosecondsNoSleep(wait);
                }
        }
 
void unKickBall(){
        //UN-KICK
        kickFlag = 0;
        digitalWrite(kDIR,HIGH);
 
        for(int i = 0; i < pulsesKick; i++){                    //LOOP through i iterations
                digitalWrite(kPULSE, HIGH);
                delayMicrosecondsNoSleep(wait);
                digitalWrite(kPULSE, LOW);
                delayMicrosecondsNoSleep(wait);
                }
        }
 
void goalieMove(int stepsToMove){
        printf("goalie moving\n");
        for(int i = 0; i < stepsToMove; i++){                   //LOOP through stepsToMove iterations
                digitalWrite(lPULSE, HIGH);
                delayMicrosecondsNoSleep(wait);
                digitalWrite(lPULSE, LOW);
                delayMicrosecondsNoSleep(wait);
                }
                printf("goalie still \n");
        }
 
void ballTrackGoalie(){
        pixFromGoalie = abs(goaliePos-blocks[0].y);
        distToMove = pixFromGoalie*3.425;
        int stepsToMoveReal = distToMove*2;
        stepsToMove = stepsToMoveReal*.5;
       
        printf("\n ----------------\n pixFromGoalie = %d pixels\n", pixFromGoalie);
        printf("distToMove = %d mm \n", distToMove);
        printf("stepsToMove = %d steps\n", stepsToMove);
        printf("locY = %d \n", blocks[0].y);
        if(blocks[0].y > lastY){
                printf("locY > lastY\n");
                digitalWrite(lDIR0,0);
                goalieMove(stepsToMove);
                goaliePos = goaliePos+pixFromGoalie;
                printf("goaliePos = %d \n", goaliePos);
                stepsToMove = 0;
        }
        if(blocks[0].y == goaliePos){
                printf("On point\n");
        }
        if(blocks[0].y < lastY){
                printf("locY < lastY\n");
                digitalWrite(lDIR0,1);
                goalieMove(stepsToMove);
                goaliePos = goaliePos-pixFromGoalie;
                printf("goaliePos = %d \n", goaliePos);
                stepsToMove = 0;
        }
}
void fieldPrint(void){
        char field[ROW][COLUMN] = { {'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},
                                                                {'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},
                                                                {'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},
                                                                {'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},
                                                                {'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},
                                                                {'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},
                                                                {'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},
                                                                {'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},{'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x'},
                                                        };
        //Hold the (X & Y)/10 location of ball
        int fieldX1 = (0.1*blocks[0].x);
        int fieldY1 = (0.1*blocks[0].y);
 
        //Writes an O to the location the ball is in
        field[fieldX1][fieldY1] = 'O';
 
        // Prints each array contained in the 2D array 'field'
        for(int w = 0; w != 32; w++){
                        printf("%s \n", field[w]);
        }
}
 
void delayMicrosecondsNoSleep(int delay_us){
                long int start_time;
                long int time_difference;
                struct timespec gettime_now;
                clock_gettime(CLOCK_REALTIME, &gettime_now);
                start_time = gettime_now.tv_nsec;
                while(1)
                {
                        clock_gettime(CLOCK_REALTIME, &gettime_now);
                        time_difference = gettime_now.tv_nsec-start_time;
                        if(time_difference < 0){
                                time_difference += 1000000000;
                                //~ printf("TD = %ld \n"  , time_difference);
                        }
                        if(time_difference >(delay_us*1000))
                        //~ printf("TD = %ld \n"  , time_difference);
                        break;
                }
                }
       
void spinMotor(){
        int run = 0;
       
        if(run == 0){
        digitalWrite(kDIR, HIGH);
                for(int k = 0; k < pulsesKick; k++){                    //LOOP through k iterations
                        digitalWrite(kPULSE, 1);
                        delayMicrosecondsNoSleep(wait);
                        digitalWrite(kPULSE, 0);
                        delayMicrosecondsNoSleep(wait);
                        }
       
        digitalWrite(kDIR, LOW);
        for(int h = 0; h < pulsesKick; h++){                    //LOOP through h iterations
                        digitalWrite(kPULSE, 1);
                        delayMicrosecondsNoSleep(wait);
                        digitalWrite(kPULSE, 0);
                        delayMicrosecondsNoSleep(wait);
                        }
}
}
 
int setupLimit(){
        while(LIMITkick != 0){
                delayMicrosecondsNoSleep(1000);
                digitalWrite(kPULSE, 0);
                delayMicrosecondsNoSleep(1000);
                digitalWrite(kPULSE, 1);
                delayMicrosecondsNoSleep(1000);
                if(LIMITkick == 0 ){
                        printf("ZEROED\n");
                        return 1;
                }
        }
}
