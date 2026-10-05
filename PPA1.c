#include <stdio.h>
#include "MacUILib.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// PREPROCESSOR DIRECTIVE CONSTANTS
// ================================

#define NUMROWS 3 //for 3 rows
#define MAXSTRINGS 10 //max # of strings
#define MAXSTRINGLENGTH 30 //max string length
#define BOARDwidth 25 //width of board
#define DELAYTIME 100000 //delay between each frame of animation here its 0.1 seconds


// GLOBAL VARIABLES
// ================================
int exitFlag;   // Program Exiting Flag, used to determine whether to leave the program loop and shutdown the program
//note: 0 = keep running and non zero means exit program

char inputChar;  // Input Character, used to store the most recent input character for the main program logic to use

// GAME BOARD
// ================================
// main board has 5 rows
// rows 0 to 2 -> rolling string row 3 -> answer/ques. row 4-> score

char gameboard[5][BOARDwidth];

// Strings read from strings.txt
// ================================
char candidatestrings[MAXSTRINGS][MAXSTRINGLENGTH];
int numberofstrings; //# of strings successfully read from strings.txt

// Information about the current round
// ================================
int selectedstrings[NUMROWS]; //stores randomly selected string indices for current round.
// eg selectedstrings[0]=7 ie row 1 is showing candidatestrings[7]

int startingposition[NUMROWS]; //stores horizontal starting point of each string
// note a (-) number means the string is outside left side of display

int correctRow; //stores which row has the right answer
int score; //your score
int rolling; //tells us if strings are still moving scross screen 1 means still going 0 means done

int waitingforanswer; //if program is waiting for player answer 1 is waaiting 0 is not





// FUNCTION PROTOTYPES
// ================================
// Declare function prototypes here, so that we can organize the function implementation after the main function for code readability.

// These are the six required program loop function to build our "McMaster Terminal Program Engine"
void Initialize(void);
void GetInput(void);
void RunLogic(void);
void DrawScreen(void);
void LoopDelay(void);
void CleanUp(void);


// Add more function prototypes here as needed
void startnewround(void);
void buildgameboard(void);
void chooserandomstrings(void);


// MAIN PROGRAM
// ===============================
// This is the "Program Loop" as described in the PPA1 flow chart.  Read Lab Documents to visualize how a persisting program works under the hood.
// Typically, when you use some sort of "Software Design Engine", this part is scaffolded - you will only be responsible of scripting the functions.
//
// In future PPA's and Projects, you are expected to be able to set up a program loop like this from scratch whenever a persisting program is required.
int main(void)
{

    // Start-Up
    Initialize();

    // Program Loop
    while(!exitFlag)  
    {
        GetInput();

        RunLogic();

        DrawScreen();

        LoopDelay();
    }

    // Tear-Down
    CleanUp();

    return 0;

}





// INITIALIZATION ROUTINE
// ===============================
// This routine is run only once before the start of the program loop - aka Start-Up Routine.
// You would typically do the following in this function:
//  1. Initialize global variables
//  2. Allocated memories for dynamic variables (we will talk about later)
//  3. Call the relevant initialization functions to enable certain library supports.

void Initialize(void)
{
    // Call the MacUI Library Initialization Function 
    MacUILib_init();

    // [TODO]: Initialize variables
    exitFlag = 0;  // 0 - do not exit, non-zero - exit the program
    inputChar = '\0'; //no input character yet
    score = 0;

    srand(time(NULL)); //time(null) gives the current time using it as the seed makes the random choices different each launch
    FILE* myFile; //create variable called myFile that points to a file
    myFile = fopen("strings.txt", "r"); //open strings.txt in read mode
    if(myFile == NULL) //gotta check if the file actually opened
    {
        MacUILib_printf("Uh oh error :c! Couldn't open strings.txt");
        exitFlag = 1; //display error and exit
        return;
    }
    numberofstrings = 0; //start with 0 strings loaded
    while(numberofstrings < MAXSTRINGS && //while fewer than max strings loaded
                            fscanf(myFile, "%29s", candidatestrings[numberofstrings])==1) //try to read one string from myFile
    {
        numberofstrings++; //add one to the total num of strings loaded
    }
    fclose(myFile); //done using file so close it

    if(numberofstrings<NUMROWS) //checks if enough strings to play the game
    {
        MacUILib_printf("Uh ohh! not enough strings in strings.txt\n");
        exitFlag=1;
        return;
    }

    startnewround(); //start the first game round!!

    
    MacUILib_clearScreen();  // Clear the screen before starting the program loop
}






// INPUT COLLECTION ROUTINE - "Observe"
// ===============================
// In our program loop setup, we always try to first determine whether there is any incoming input.
// This routine can either be "blocking" (synchronous input) or "non-blocking" (asynchronous input)
// In our project and preparation applications, you should use Asynchronous Mode to ensure the program/game runs smoothly even without any input.

void GetInput(void)
{

    
    inputChar = '\0'; //reset input char every loop to allow it to process a key once
    if(MacUILib_hasChar()) //has the user pressed a key?
    {
        inputChar = MacUILib_getChar(); //if key pressed get key save it to inputchar

    }

}




// MAIN LOGIC ROUTINE - "Think"
// ===============================
// The second action in our program loop is to execute the main program logic.  
// In this routine, we should determine the outcome of the logic using 
//    a) current status / state / behaviour of the program, and 
//    b) the most recent input
// The outcome of the logic then will be drawn on the screen.
//
// In later activities, we will delve deeper into this design philosophy later in the course - Finite State Machine design principle.

void RunLogic(void)
{    
    int row;
    // SHUTDOWN COMMAND
    // ===============================
    if(inputChar == '!') //I want to use ! as the shutdown key
    {
        exitFlag = 1;
        return;
    }
    // move the strings
    // ===============================
    if(rolling)
    {
        for(row=0; row<NUMROWS; row++) //move every string one position to the right
        {
            startingposition[row]++;
        }
        if(startingposition[0] > BOARDwidth - 1 + strlen(candidatestrings[selectedstrings[0]]) &&
   startingposition[1] > BOARDwidth - 1 + strlen(candidatestrings[selectedstrings[1]]) &&
   startingposition[2] > BOARDwidth - 1 + strlen(candidatestrings[selectedstrings[2]])) //Boardwidth-1 is the last valid screen position > means moved past
        {
            rolling = 0; //rolling part has finished
            waitingforanswer = 1; //now wait for players answers
        }
    }

    //NOW: Check players answer

    if(waitingforanswer)
    {
        if(inputChar >= '1' && inputChar <= '3') //valid row inputs are 1 2 and 3
        {
          int playeranswer = inputChar  - '1'; //convert character into a row index
          if(playeranswer == correctRow)
          {
            score = score+10; //correct answer yaaaay
            startnewround(); // start another round
          }
          else
          {
            exitFlag=1; //wrong answer so game ends
          }     
        }
    }
    //Build the game board
    // ===============================
    buildgameboard();

}







// DRAW ROUTINE - "Act"
// ===============================
// This routine creates the output of the program for a given iteration, using the resultant parameters from the RunLogic() routine.
// In the completed program, this should be the only routine calling the MACUILib_printf().
// If we apply the animation technique discussed in the lab document, we can then create a smooth marquee display animation on the terminal.
//
// Animation Basics (Review):
//  In every iteration
//      - Clear the screen (remove the printed contents from the previous iteration, a.k.a. previous frame)
//      - Use the calculated parameters from RunLogic(), draw line-by-line on the terminal screen to build the desired display contents for this iteration
//        (a.k.a. draw the current frame)
//
//  If we repeat the actions above at a suitable rate, the contents on the screen will look like it's moving smoothly on the screen,
//  as if we are flipping through pages of pictures.
//  - If too slow, the animation will look choppy (frame rate too low)
//  - If too fast, you won't see the animation at all (frame rate too high)

void DrawScreen(void)
{
    int row;
    int column;

    MacUILib_clearScreen(); //clear the previous frame from screen

    //make the game board

    for(row = 0; row<5; row++)
    {
        for(column = 0; column < BOARDwidth; column++)
        {
            MacUILib_printf("%c", gameboard[row][column]); //print one character at a time
        }

        MacUILib_printf("\n"); //go the the next line after each row
    }
}





// DELAY ROUTINE - "Wait"
// ===============================
// Sometimes referred to as the "stupidifier", this routine is only intended to slow down the program loop so that
// 1. The animation is not too fast to be appreciated
// 2. The main logic is not executed too often, such that the game won't act too ahead of the player's reaction.  Otherwise, the game will be impossible to be beaten.

void LoopDelay(void)
{
    MacUILib_Delay(DELAYTIME); // wait 0.1 sec before running next program loop to control animation speed

}





// TEAR-DOWN ROUTINE
// ===============================
// This routine is run only once at the end of the program right before shutdown, intended to clean up all the resources used by the program.
// This routine is VERY IMPORTANT to prevent memory leak.  We will cover this after the midterm.

void CleanUp(void)
{    
    // For now, you only need to call MacUILib_uninit() routine to shut down the MacUILib module.
    MacUILib_uninit();

    // In PPA2, you will have to create Player, Enemies, and Bullets on Heap.
    //  This means you will have to deallocate them here before game shutdown to prevent memory leakage.
}



//start new round

void startnewround(void)
{
    int row;
    chooserandomstrings(); // choose new random strings for this round.
    for(row=0; row < NUMROWS; row++)
    {
    startingposition[row] = -strlen(candidatestrings[selectedstrings[row]]);
    }

    correctRow= rand()%NUMROWS;
    rolling=1; //the strings are now rolling
    waitingforanswer=0; //not asking for answer yet


}

//choose random strings

void chooserandomstrings(void)
{
    int row;
    int randomIndex;
    int duplicate;
    int previousrow;

    for (row=0; row<NUMROWS; row++) //need different string for each row.
    {
        duplicate =1;
        while(duplicate) //keep choosing random string until we get one that hasnt been chosen
        {
            duplicate=0;
            randomIndex = rand() % numberofstrings; //choose a random index from the candidate string array
            //check if string already selected in earlier row 
            for (previousrow = 0; previousrow < row; previousrow++)
            {
                if(selectedstrings[previousrow] == randomIndex)
                {
                    duplicate =1;
                }
            }
        }
        selectedstrings[row] = randomIndex; //save the randomly chosen string index.

    }
}

//build game board
void buildgameboard(void)
{
    int row;
    int column;
    int stringPosition;
    int stringlength;
    for(row=0; row<5; row++)
    {
        for(column=0; column< BOARDwidth; column++)
        {
            gameboard[row][column] = ' '; //fill every position with a blank space

        }
    }
    //add the row numbers
    //row 1
    gameboard[0][0] = '1';
    gameboard[0][1] = '|';
    //row 2
    gameboard[1][0] = '2';
    gameboard[1][1] = '|';
    //row 3
    gameboard[2][0] = '3';
    gameboard[2][1] = '|';

 //put rolling strings onto board
for(row = 0; row < NUMROWS; row++)
{
    stringlength = strlen(candidatestrings[selectedstrings[row]]);

    for(column = 2; column < BOARDwidth; column++)
    {
        stringPosition = column - 2 - startingposition[row];

        if(stringPosition >= 0 && stringPosition < stringlength)
        {
            gameboard[row][column] =
                candidatestrings[selectedstrings[row]][stringPosition];
        }
    }
}


// Answer line
if(waitingforanswer)
{
    // Print "== " first
    gameboard[3][0] = '=';
    gameboard[3][1] = '=';
    gameboard[3][2] = ' ';

    // Find the length of the correct target string
    stringlength = strlen(candidatestrings[selectedstrings[correctRow]]);

    // Display the target string
    for(column = 0; column < stringlength && column + 3 < BOARDwidth; column++)
    {
        gameboard[3][column + 3] =
            candidatestrings[selectedstrings[correctRow]][column];
    }
}


// Score line
gameboard[4][0] = 'S';
gameboard[4][1] = 'C';
gameboard[4][2] = 'O';
gameboard[4][3] = 'R';
gameboard[4][4] = 'E';
gameboard[4][5] = ':';
gameboard[4][6] = ' ';

if(score < 10)
{
    gameboard[4][7] = '0' + score;
}
else
{
    gameboard[4][7] = '0' + (score / 10);
    gameboard[4][8] = '0' + (score % 10);
}
}