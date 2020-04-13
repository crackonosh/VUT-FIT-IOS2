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
#include <semaphore.h>
#include <sys/mman.h>

// GLOBAL VARIABLES
FILE *oFile = NULL;

int *outputCount = NULL;        // A
int *immNotRegistered = NULL;   // NE
int *immNotAllowed = NULL;      // NC
int *immInBuilding = NULL;      // NB

sem_t *entrance = NULL; // immigrants can enter one by one
sem_t *registrations = NULL; // immigrants can register one by one
sem_t *judgeInBuilding = NULL; // if judge inside, immigrants cannot leave 

// mutex for judge needed

// FUNCTION HEADERS
int init ();
void cleanup ();
void checkParameter (int parameter, char *msg);
void immigrantsGenerator (int count, int maxTime);
void processImmigrant (int number);


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
  
  if (init() == -1)
  {
    cleanup();
    return 35;
  }

  // create immigrants generator and judge process
  pid_t id = fork();
  if (id == 0)
  {
    immigrantsGenerator(imCount, imGenTime);
  }
  else
  {

  }


  // END OF PROGRAM
  wait(NULL);
  cleanup();
  exit(0);
  return 0;
}

// !!! F U N C T I O N S !!!

/**
 * Opens file for writing, initializes global variables and creates semaphores
 * 
 * @return int
 */
int init ()
{
  oFile = fopen("proj2.out", "w");

  outputCount = mmap(NULL, sizeof(*outputCount), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  *outputCount = 1;

  immNotRegistered = mmap(NULL, sizeof(*immNotAllowed), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  immNotAllowed = mmap(NULL, sizeof(*immNotAllowed), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  immInBuilding = mmap(NULL, sizeof(*immInBuilding), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

  if ((entrance = sem_open("/xhaisl00-entrance", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;
  if ((entrance = sem_open("/xhaisl00-registrations", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;
  if ((entrance = sem_open("/xhaisl00-judgeInBuilding", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;

  return 0;
}
/**
 * Cleans up everything init created
 * 
 * @return void
 */
void cleanup ()
{
  munmap(outputCount, sizeof(outputCount));

  munmap(immNotRegistered, sizeof(immNotRegistered));
  munmap(immNotAllowed, sizeof(immNotAllowed));
  munmap(immInBuilding, sizeof(immInBuilding));

  sem_unlink("/xhaisl00-entrance");
  sem_unlink("/xhaisl00-registrations");
  sem_unlink("/xhaisl00-judgeInBuilding");

  if (oFile != NULL) fclose(oFile);
}
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

/**
 * Generates "count" number of immigrants every rand<0,maxTime>
 * 
 * @param int count
 * @param int maxTime
 * @return void
 */
void immigrantsGenerator (int count, int maxTime)
{
  // renew random
  time_t t;
  srand((unsigned) time(&t));
  int i; 
  for (i = 1;i <= count;i++) 
  { 
    // set process to sleep for rand <0,maxTime>
    if (maxTime > 0)
    {
      struct timespec ts;
      ts.tv_sec = (rand() % maxTime) / 1000;
      ts.tv_nsec = ((rand() % maxTime) % 1000) * 1000000;
      nanosleep(&ts, NULL);
    }

    // body of imigrant
    pid_t iId = fork();
    if (iId == 0) 
    { 
      processImmigrant(i);
      exit(0);
    }
  }
  exit(0);
}
/**
 * Performs all expected actions for immigrant process with number
 * 
 * @param int number
 * @return void
 */
void processImmigrant (int number)
{
  printf("%d\t: IMM %d\t\t : starts\n", *outputCount, number);
  *outputCount += 1;
  // implement random sleeping 

  // printf("%d    : IMM %d      : enters\n", *outputCount, i);
  // *outputCount += 1;
  // sleep(1); 
  // printf("%d    : IMM %d      : checks\n", *outputCount, i);
  // *outputCount += 1;
  // sleep(1); 
  // printf("%d    : IMM %d      : wants certificate\n", *outputCount, i);
  // *outputCount += 1;
  // sleep(1); 
  // printf("%d    : IMM %d      : got certificate\n", *outputCount, i);
  // *outputCount += 1;
}
