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
#include <stdbool.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

// GLOBAL VARIABLES
FILE *oFile = NULL;

int *outputCount = NULL;        // A
int *immRegistered = NULL;      // NE
int *immNotAllowed = NULL;      // NC
int *immInBuilding = NULL;      // NB
int *remainingImmigrants = NULL; // used for judge

sem_t *entrance = NULL; // immigrants can enter one by one
sem_t *registrations = NULL; // immigrants can register one by one
sem_t *judgeInBuilding = NULL; // if judge inside, immigrants cannot leave 

// mutex for judge needed

// FUNCTION HEADERS

int init ();
void cleanup ();
void checkParameter (int parameter, char *msg);
void immigrantsGenerator (int count, int maxTime, int maxLeaveTime);
void processImmigrant (int number, int leaveTime);
void processJudge (int approvalMaxTime);


/**
 * Main function
 * 
 * @param int argc 
 * @param char **argv
 * @return int
 */
int main (int argc, char **argv)
{
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
  
  if (init() == -1)
  {
    cleanup();
    return 35;
  }
  *remainingImmigrants = imCount;
  

  // create immigrants generator and judge process
  pid_t judge = 0;
  pid_t id = fork();
  if (id == 0)
  {
    immigrantsGenerator(imCount, imGenTime, imLeaveTime);
  }
  else
  {
    // renew random
    time_t t;
    srand((unsigned) time(&t));

    judge = fork();
    if (judge == 0)
    {
      // set process to sleep for rand <0,maxTime>
      if (judGenTime > 0)
      {
        struct timespec ts;
        ts.tv_sec = (rand() % judGenTime) / 1000;
        ts.tv_nsec = ((rand() % judGenTime) % 1000) * 1000000;
        nanosleep(&ts, NULL);
      }
      while (*remainingImmigrants != 0)
      {
        processJudge(judApprovalTime);

        // set process to sleep for rand <0,maxTime>
        if (judGenTime > 0)
        {
          struct timespec ts;
          ts.tv_sec = (rand() % judGenTime) / 1000;
          ts.tv_nsec = ((rand() % judGenTime) % 1000) * 1000000;
          nanosleep(&ts, NULL);
        }
      }
      exit(0);
    }
  }


  // END OF PROGRAM
  //wait(&judge);
 // wait(NULL);
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

  remainingImmigrants = mmap(NULL, sizeof(*remainingImmigrants), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  immRegistered = mmap(NULL, sizeof(*immRegistered), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  immNotAllowed = mmap(NULL, sizeof(*immNotAllowed), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  immInBuilding = mmap(NULL, sizeof(*immInBuilding), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

  if ((entrance = sem_open("/xhaisl00-entrance", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;
  if ((registrations = sem_open("/xhaisl00-registrations", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;
  sem_wait(registrations);
  if ((judgeInBuilding = sem_open("/xhaisl00-judgeInBuilding", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;

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

  munmap(remainingImmigrants, sizeof(remainingImmigrants));
  munmap(immRegistered, sizeof(immRegistered));
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
void immigrantsGenerator (int count, int maxTime, int maxLeaveTime)
{
  // renew random
  time_t t;
  srand((unsigned) time(&t));
  for (int i = 1;i <= count;i++) 
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
      processImmigrant(i, maxLeaveTime);
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
void processImmigrant (int number, int leaveTime)
{
  // renew random
  time_t t;
  srand((unsigned) time(&t));

  *outputCount += 1;
  printf("%d\t: IMM %d\t\t: starts:\n", *outputCount, number);

  sem_wait(judgeInBuilding);
  sem_post(judgeInBuilding);
  sem_wait(entrance);

  *outputCount += 1;
  *immInBuilding += 1;
  *immNotAllowed += 1;
  printf(
    "%d\t: IMM %d\t\t: enters:\t\t: %d\t: %d\t: %d\n",
    *outputCount,
    number,
    *immNotAllowed,
    *immRegistered,
    *immInBuilding
  );

  *outputCount += 1;
  *immRegistered += 1;
  printf(
    "%d\t: IMM %d\t\t: checks:\t\t: %d\t: %d\t: %d\n",
    *outputCount,
    number,
    *immNotAllowed,
    *immRegistered,
    *immInBuilding
  );
  sem_post(entrance);

  sem_wait(registrations);
  
  *outputCount += 1;
  printf(
    "%d\t: IMM %d\t\t: wants cetificate\t: %d\t: %d\t: %d\n",
    *outputCount,
    number,
    *immNotAllowed,
    *immRegistered,
    *immInBuilding
  );

  if (leaveTime > 0)
  {
    struct timespec ts;
    ts.tv_sec = (rand() % leaveTime) / 1000;
    ts.tv_nsec = ((rand() % leaveTime) % 1000) * 1000000;
    nanosleep(&ts, NULL);
  }

  *outputCount += 1;
  printf(
    "%d\t: IMM %d\t\t: got cetificate\t: %d\t: %d\t: %d\n",
    *outputCount,
    number,
    *immNotAllowed,
    *immRegistered,
    *immInBuilding
  );

  sem_wait(judgeInBuilding);
  sem_post(judgeInBuilding);

  *outputCount += 1;
  *immInBuilding -= 1;
  printf(
    "%d\t: IMM %d\t\t: leaves\t\t: %d\t: %d\t: %d\n",
    *outputCount,
    number,
    *immNotAllowed,
    *immRegistered,
    *immInBuilding
  );
  *remainingImmigrants -= 1;
}



void processJudge (int approvalMaxTime)
{
  // renew random
  time_t t;
  srand((unsigned) time(&t));


  *outputCount += 1;
  printf("%d\t: JUDGE\t\t: wants to enter\n", *outputCount);

  sem_wait(judgeInBuilding);
  *outputCount += 1;
  printf(
    "%d\t: JUDGE\t\t: enters\t\t: %d\t: %d\t: %d\n",
    *outputCount,
    *immNotAllowed,
    *immRegistered,
    *immInBuilding
  );
  //sem_post(entrance);


  if (*immNotAllowed != *immRegistered)
  {
    *outputCount += 1;
    printf(
      "%d\t: JUDGE\t\t: waits for imm\t: %d\t: %d\t: %d\n",
      *outputCount,
      *immNotAllowed,
      *immRegistered,
      *immInBuilding
    );
  }

  *outputCount += 1;
  printf(
    "%d\t: JUDGE\t\t: starts confirmation\t: %d\t: %d\t: %d\n",
    *outputCount,
    *immNotAllowed,
    *immRegistered,
    *immInBuilding
  );

  if (approvalMaxTime > 0)
  {
    struct timespec ts;
    ts.tv_sec = (rand() % approvalMaxTime) / 1000;
    ts.tv_nsec = ((rand() % approvalMaxTime) % 1000) * 1000000;
    nanosleep(&ts, NULL);
  }

  *outputCount += 1;
  *immNotAllowed = 0;
  *immRegistered = 0;
  printf(
    "%d\t: JUDGE\t\t: ends confirmation:\t: %d\t: %d\t: %d\n",
    *outputCount,
    *immNotAllowed,
    *immRegistered,
    *immInBuilding
  );

  for (int i = 0; i < *immInBuilding; i++)
    sem_post(registrations);

  if (approvalMaxTime > 0)
  {
    struct timespec ts;
    ts.tv_sec = (rand() % approvalMaxTime) / 1000;
    ts.tv_nsec = ((rand() % approvalMaxTime) % 1000) * 1000000;
    nanosleep(&ts, NULL);
  }
  *outputCount += 1;
  printf(
    "%d\t: JUDGE\t\t: leaves\t\t: %d\t: %d\t: %d\n",
    *outputCount,
    *immNotAllowed,
    *immRegistered,
    *immInBuilding
  );
  sem_post(judgeInBuilding);
}
