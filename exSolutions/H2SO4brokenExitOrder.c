/* 
 * Solution to H2S04 molecule creation problem - broken, removed synch for exit order
 * Author: Sherri Goings
 * Last modified: 10/8/2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include "H2SO4.h"

int checkSem(sem_t*, char*);

// ------------------------------------------------------------------------------------------------
// the following semaphore declarations will be specific to each student's solution
sem_t* hydro_sem;
sem_t* oxy_sem;
sem_t* hydrowait_sem;
sem_t* oxywait_sem;
sem_t* sulfurwait_sem;
sem_t* molready_sem;
// student specific semaphore declaration ends here
// ------------------------------------------------------------------------------------------------

void openSems() {

  // open all used semaphores
  hydro_sem = sem_open("hydrosmphr1", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(hydro_sem, "hydrosmphr1") == -1) {
    hydro_sem = sem_open("hydrosmphr1", O_CREAT|O_EXCL, 0466, 0);
  }

  oxy_sem = sem_open("oxysmphr1", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(oxy_sem, "oxysmphr1") == -1) {
    oxy_sem = sem_open("oxysmphr1", O_CREAT|O_EXCL, 0466, 0);
  }

  hydrowait_sem = sem_open("hydrowaitsmphr1", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(hydrowait_sem, "hydrowaitsmphr1") == -1) {
    hydrowait_sem = sem_open("hydrowaitsmphr1", O_CREAT|O_EXCL, 0466, 0);
  }

  oxywait_sem = sem_open("oxywaitsmphr1", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(oxywait_sem, "oxywaitsmphr1") == -1) {
    oxywait_sem = sem_open("oxywaitsmphr1", O_CREAT|O_EXCL, 0466, 0);
  }

  sulfurwait_sem = sem_open("sulfurwaitsmphr1", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(sulfurwait_sem, "sulfurwaitsmphr1") == -1) {
    sulfurwait_sem = sem_open("sulfurwaitsmphr1", O_CREAT|O_EXCL, 0466, 0);
  }

  molready_sem = sem_open("molreadysmphr1", O_CREAT|O_EXCL, 0466, 1);
  while (checkSem(molready_sem, "molreadysmphr1") == -1) {
    molready_sem = sem_open("molreadysmphr1", O_CREAT|O_EXCL, 0466, 1);
  }
}
 
void closeSems() {
  sem_close(oxy_sem);
  sem_unlink("oxysmphr1");
  sem_close(hydro_sem);
  sem_unlink("hydrosmphr1");
  sem_close(oxywait_sem);
  sem_unlink("oxywaitsmphr1");
  sem_close(hydrowait_sem);
  sem_unlink("hydrowaitsmphr1");
  sem_close(sulfurwait_sem);
  sem_unlink("sulfurwaitsmphr1");
  sem_close(molready_sem);
  sem_unlink("molreadysmphr1");
}

void* oxygen(void* args) {
  // produce an oxygen molecule
  printf("oxygen produced\n");
  fflush(stdout);

  // post on oxygen semaphore to signal that an oxygen atom has been produced
  sem_post(oxy_sem);  

  // wait until released
  sem_wait(oxywait_sem);

  printf("oxygen exited\n");
  fflush(stdout);

  sem_post(sulfurwait_sem);
  return (void*) 0;
}

void* hydrogen(void* args) {
  // produce a hydrogen molecule
  printf("hydrogen produced\n");
  fflush(stdout);

  // post on hydrogen semaphore to signal that a hydrogen atom has been produced
  sem_post(hydro_sem);

  // wait until released
  sem_wait(hydrowait_sem);

  printf("hydrogen exited\n");
  fflush(stdout);

  sem_post(sulfurwait_sem);
  return (void*) 0;
}

void* sulfur(void* args) {
  // produce an sulfur molecule
  printf("sulfur produced\n");
  fflush(stdout);

  // wait for any molecules in the process of being produced but not yet finished (i.e.
  // only let one sulfur at a time past here to start actually "bonding" with other atoms)
  sem_wait(molready_sem);
  
  // sulfur waits for 2 hydrogens and 4 oxygens to be produced if they haven't already
  int errs[6];
  int i;
  //printf("sulfer going to wait on hydro sem *2\n");
  fflush(stdout);
  for (i=0; i<2; i++) {
    errs[i] = sem_wait(hydro_sem);
  }
  //printf("sulfer awake after waiting on hydro sem\n");
  fflush(stdout);

  //printf("sulfer going to wait on oxy sem *4\n");
  fflush(stdout);
  for (i=2; i<6; i++) {
    errs[i] = sem_wait(oxy_sem);
  }
  //printf("sulfer awake after waiting on oxy sem\n");
  fflush(stdout);
  for (i=0; i<6; i++) {
    if (errs[i]==-1) printf("error on wait #%d in sulfur, error # %d\n", i, errno);
  }
  // printf("sulfer going to wait on molready sem\n");
  //fflush(stdout);
  // now have ingredients for a complete molecule, but may have to wait for all components
  // of previously made molecule to exit before can make this next molecule
  
  //printf("sulfer awake after waiting on molready sem\n");
  //fflush(stdout);

  // finally can produce current molecule
  printf("molecule produced\n");
  fflush(stdout);

  // wake up 2 waiting hydrogens 
  for (i=0; i<2; i++) {
    sem_post(hydrowait_sem);
  }
  
  // wake up 4 waiting oxygens
  for (i=0; i<4; i++) {
    sem_post(oxywait_sem);
  }
  sched_yield();
  // sulfur itself exits
  printf("sulfur exited\n");
  fflush(stdout);

  // let any waiting sulfurs know that this molecule is finished and the next may be created
  sem_post(molready_sem);

  //printf("sulfer has posted to molready sem\n");
  fflush(stdout);
  
  return (void*) 0;
}  

int checkSem(sem_t* sema, char* filename) {
  if (sema==SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore %s already exists, unlinking and reopening\n", filename);
      fflush(stdout);
      sem_unlink(filename);
      return -1;
    }
    else {
      printf("semaphore %s could not be opened, error # %d\n", filename, errno);
      fflush(stdout);
      exit(1);
    }
  }
  return 0;
}

