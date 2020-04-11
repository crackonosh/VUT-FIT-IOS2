/**
 * File: proj2.c
 * 
 * Author: Lukas Hais
 * Login: xhaisl00
 * Date: April 11
 * Course: IOS
 * 
 * Summary of File:
 *  TODO
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <time.h>

// GLOBAL VARIABLES
int outputRow = 1;

// FUNCTION HEADERS
void checkParameter (int parameter, char *msg);
void immigrantsGenerator (int count, int maxTime);


/**
 * Main function
 * 
 * @param int argc 
 * @param char **argv
 * @return int
 */
int main (int argc, char **argv)
{
#pragma region initialization
  if (argc != 6)
  {
    fprintf(stderr, "Invalid number of arguments used.\n");
    fprintf(stderr, "Exiting program...\n");
    return 1;
  }
  
  // variable declaration
  int imCount, imGenTime, judGenTime, imLeaveTime, judApprovalTime;
  char *endptr;

  // taking arguments from cmdline
  imCount = strtol(argv[1], &endptr, 10);
  imGenTime = strtol(argv[2], &endptr, 10);
  judGenTime = strtol(argv[3], &endptr, 10);
  imLeaveTime = strtol(argv[4], &endptr, 10);
  judApprovalTime = strtol(argv[5], &endptr, 10);

  // PARAMETER CHEKCS
  if (*endptr)
  {
    fprintf(stderr, "One of arguments is not a number.\n");
    return 1;
  }
  checkParameter(imGenTime, "for immigrant generation.");
  checkParameter(judGenTime, "for judge generation.");
  checkParameter(imLeaveTime, "for immigrant's leave time.");
  checkParameter(judApprovalTime, "for judge's approval time.");
#pragma endregion
  

  immigrantsGenerator(imCount, imGenTime);

  // END OF PROGRAM
  wait(NULL);
  printf("\n\nExiting program...\n");
  return 0;
}

// !!! F U N C T I O N S !!!

/**
 * Function checks if parameter is within the allowed range.
 * Otherwise prints error msg to stderr.
 * 
 * @param int parameter
 * @param char *msg
 * @return void
 */
void checkParameter (int parameter, char *msg)
{
  if (parameter < 0 || parameter > 2000)
  {
    fprintf(stderr, "Invalid number %s.\n", msg);
    exit(1);
  }
}

void immigrantsGenerator (int count, int maxTime)
{
  time_t t;
  int i; 
  pid_t pid[count]; 
  srand((unsigned) time(&t));
  for (i = 0;i < count;i++) 
  { 
      sleep(rand() % maxTime / 1000);
      // body of imigrant
      if ((pid[i] = fork()) == 0) 
      { 
          printf("%d    : IMM %d      : starts\n", outputRow++, i);
          sleep(1); 
          printf("%d    : IMM %d      : enters\n", outputRow++, i);
          sleep(1); 
          printf("%d    : IMM %d      : checks\n", outputRow++, i);
          sleep(1); 
          printf("%d    : IMM %d      : wants certificate\n", outputRow++, i);
          sleep(1); 
          printf("%d    : IMM %d      : got certificate\n", outputRow++, i);
          exit(1);
      }
      
  } 
  wait(NULL);
}
