/**
 * File: proj2.c
 * 
 * Author: Lukas Hais
 * Login: xhaisl00
 * Date: April 11
 * Course: IOS
 * 
 * Summary of File:
 *    Using semaphores in real world.
 */
// ----- INCLUDES -----
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h> 
#include <stdbool.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "proj2.h"
// ----- END INCLUDES -----

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

  // error
  if (id == -1)
  {
    fprintf(stderr, "Unable to perform fork. Exiting...");
    cleanup();
    exit(1);
  }
  // child
  else if (id == 0)
    immigrantsGenerator(imCount, imGenTime, imLeaveTime);
  // parent
  else
  {
    // renew random
    time_t t;
    srand((unsigned) time(&t));

    judge = fork();
    if (judge == -1)
    {
      fprintf(stderr, "Unable to create judge. Aborting!");
      cleanup();
      exit(1);
    }
    else if (judge == 0)
    {
      // set process to sleep for rand <0,maxTime>
      if (judGenTime > 0)
      {
        ts.tv_sec = (rand() % judGenTime) / 1000;
        ts.tv_nsec = ((rand() % judGenTime) % 1000) * 1000000;
        nanosleep(&ts, NULL);
      }
      while (*remainingImmigrants > 0)
      {
        processJudge(judApprovalTime);

        // set process to sleep for rand <0,maxTime>
        if (judGenTime > 0)
        {
          ts.tv_sec = (rand() % judGenTime) / 1000;
          ts.tv_nsec = ((rand() % judGenTime) % 1000) * 1000000;
          nanosleep(&ts, NULL);
        }
      }
      writeToFile("JUDGE\t\t: finishes\n");
      exit(0);
    }
  }


  // END OF PROGRAM
  wait(&judge);
  wait(0);
  cleanup();
  exit(0);
  return 0;
}

// ----- FUNCTION DEFINITION -----
int init ()
{
  oFile = fopen("proj2.out", "w");

  outputCount = mmap(NULL, sizeof(*outputCount), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

  remainingImmigrants = mmap(NULL, sizeof(*remainingImmigrants), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  immRegistered = mmap(NULL, sizeof(*immRegistered), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  immNotAllowed = mmap(NULL, sizeof(*immNotAllowed), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);
  immInBuilding = mmap(NULL, sizeof(*immInBuilding), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0);

  // if semaphores were initialized before, return -1 -> will perform cleanup
  if ((registered = sem_open("/xhaisl00-registered", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;
  sem_wait(registered); // null it before usage
  if ((judgeInBuilding = sem_open("/xhaisl00-judgeInBuilding", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;
  if ((fileWrite = sem_open("/xhaisl00-fileWrite", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) return -1;

  return 0;
}
void cleanup ()
{
  munmap(outputCount, sizeof(outputCount));

  munmap(remainingImmigrants, sizeof(remainingImmigrants));
  munmap(immRegistered, sizeof(immRegistered));
  munmap(immNotAllowed, sizeof(immNotAllowed));
  munmap(immInBuilding, sizeof(immInBuilding));

  sem_unlink("/xhaisl00-registered");
  sem_unlink("/xhaisl00-judgeInBuilding");
  sem_unlink("/xhaisl00-fileWrite");

  if (oFile != NULL) fclose(oFile);
}
void checkParameter (int parameter, char *msg)
{
  if (parameter < 0 || parameter > 2000)
  {
    fprintf(stderr, "Invalid number %s.\n", msg);
    exit(1);
  }
}
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
      ts.tv_sec = (rand() % maxTime) / 1000;
      ts.tv_nsec = ((rand() % maxTime) % 1000) * 1000000;
      nanosleep(&ts, NULL);
    }

    // body of imigrant
    pid_t iId = fork();
    if (iId == -1)
    {
      fprintf(stderr, "Unable to create immigrant. Aborting!");
      *remainingImmigrants = 0;
      exit(1);
    }
    if (iId == 0) 
    { 
      processImmigrant(i, maxLeaveTime);
      exit(0);
    }
  }
  exit(0);
}
void processImmigrant (int number, int leaveTime)
{
  // renew random
  time_t t;
  srand((unsigned) time(&t));

  // START OF IMMIGRANT
  writeToFile("IMM %d\t\t: starts\n", number);

  // JUDGE IN BUILDING CHECK
  sem_wait(judgeInBuilding);
    writeToFile(
      "IMM %d\t\t: enters\t\t: %d\t: %d\t: %d\n",
      number, ++(*immNotAllowed),
      *immRegistered, ++(*immInBuilding)
    );
  sem_post(judgeInBuilding);

  // CHECK IN REGISTRATIONS
  writeToFile(
    "IMM %d\t\t: checks\t\t: %d\t: %d\t: %d\n",
    number, *immNotAllowed,
    ++(*immRegistered), *immInBuilding
  );

  // WANTS CERTIFICATE
  sem_wait(registered);
  writeToFile(
    "IMM %d\t\t: wants certificate\t: %d\t: %d\t: %d\n",
    number, *immNotAllowed,
    *immRegistered, *immInBuilding
  );

  // RANDOM SLEEP BETWEEN GETTING CERTIFICATE
  if (leaveTime > 0)
  {
    ts.tv_sec = (rand() % leaveTime) / 1000;
    ts.tv_nsec = ((rand() % leaveTime) % 1000) * 1000000;
    nanosleep(&ts, NULL);
  }

  // GETS CERTIFICATE
  writeToFile(
    "IMM %d\t\t: got certificate\t: %d\t: %d\t: %d\n",
    number, *immNotAllowed,
    *immRegistered, *immInBuilding
  );

  sem_wait(judgeInBuilding);
  sem_post(judgeInBuilding);

  writeToFile(
    "IMM %d\t\t: leaves\t\t: %d\t: %d\t: %d\n",
    number, *immNotAllowed,
    *immRegistered, --(*immInBuilding)
  );
}
void processJudge (int approvalMaxTime)
{
  // renew random
  time_t t;
  srand((unsigned) time(&t));

  // JUDGE WANTS TO ENTER BUILDING
  writeToFile("JUDGE\t\t: wants to enter\n");

  // JUDGE ENTERS BUILDING
  sem_wait(judgeInBuilding);
    writeToFile(
      "JUDGE\t\t: enters\t\t: %d\t: %d\t: %d\n",
      *immNotAllowed, *immRegistered, *immInBuilding
    );

    if (*immNotAllowed != *immRegistered)
    {
      // WAITS FOR NOT REGISTERED IMMIGRANTS
      writeToFile(
        "JUDGE\t\t: waits for imm\t: %d\t: %d\t: %d\n",
        *immNotAllowed, *immRegistered, *immInBuilding
      );
    }

    // JUDGE STARTS CONFIRMATION
    writeToFile(
      "JUDGE\t\t: starts confirmation\t: %d\t: %d\t: %d\n",
      *immNotAllowed, *immRegistered, *immInBuilding
    );

    // RANDOM SLEEP BETWEEN END OF CONFIRMATION
    if (approvalMaxTime > 0)
    {
      ts.tv_sec = (rand() % approvalMaxTime) / 1000;
      ts.tv_nsec = ((rand() % approvalMaxTime) % 1000) * 1000000;
      nanosleep(&ts, NULL);
    }

    // END OF CONFIRMATION
    writeToFile(
      "JUDGE\t\t: ends confirmation\t: %d\t: %d\t: %d\n",
      *immNotAllowed = 0, 0, *immInBuilding
    );

    int tmp = *immRegistered;
    *immRegistered = 0;
    // SEND SIGNAL TO ALL CONFIRMED IMMIGRANTS IN BUILDING THAT THEY MAY LEAVE
    for (int i = tmp; i > 0; i--)
    {
      sem_post(registered);
      (*remainingImmigrants)--;
    }

    // RANDOM SLEEP BEFORE LEAVE
    if (approvalMaxTime > 0)
    {
      ts.tv_sec = (rand() % approvalMaxTime) / 1000;
      ts.tv_nsec = ((rand() % approvalMaxTime) % 1000) * 1000000;
      nanosleep(&ts, NULL);
    }

    writeToFile(
      "JUDGE\t\t: leaves\t\t: %d\t: %d\t: %d\n",
      *immNotAllowed, *immRegistered, *immInBuilding
    );
  sem_post(judgeInBuilding);
}
void writeToFile (const char *s, ...)
{
  sem_wait(fileWrite);
    // create argument list for formatted string *s
    va_list args;
    va_start(args, s);

    // print row number and then formatted string *s
    fprintf(oFile, "%d\t: ", ++(*outputCount));
    vfprintf(oFile, s, args);
    // send everything to file!
    fflush(oFile);

    // destrou argument list
    va_end(args);
  sem_post(fileWrite);
}
// ----- END FUNCTION DEFINITION -----
