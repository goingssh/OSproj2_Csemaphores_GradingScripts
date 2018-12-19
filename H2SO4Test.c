/* 
 * Test program for H2S04 molecule creation problem
 * Author: Sherri Goings
 * Last modified: 4/18/2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include "H2SO4.h"

// local function just used to randomly shuffle creation order of atoms 
void shuffle(int*, int);
void delay(int);

// tests H2SO4 molecule building given command-line arguments for # of each atom
// randomizes creation order for all atoms
int main(int argc, char* argv[]) {
  if (argc < 4) {
    printf("\nmust include num H, S, O atoms as command line arguments (e.g. './a.out 4 2 8')\n\n");
    exit(1);
  }

  // open all semaphores you use
  openSems();

  // set # to create of each atom (atoi converts a string to an int)
  int numhydros = atoi(argv[1]);
  int numsulfurs = atoi(argv[2]);
  int numoxys = atoi(argv[3]);
  
  // special test if numhydros = -1 where keep all sulfurs first,
  // note this and change numhydros to correct ratio w/sulfurs
  int sulfurFirst = 0;
  if (numhydros == -1) {
    sulfurFirst = 1;
    numhydros = numsulfurs*2;
  }
  const int total = numoxys+numhydros+numsulfurs; 

  // seed the random number generator with the current time
  srand(time(NULL));

  // add desired number of each type of atom (represented as a simple int) to an array
  // oxygen is represented as 1, hydrogen as 2, and sulfur as 3
  int order[total];
  int i;
  for (i=0; i<numsulfurs; i++) {
    order[i] = 3;
  }
  for (; i<numhydros+numsulfurs; i++) {
    order[i] = 2;
  }
  for (; i<total; i++) {
    order[i] = 1;
  }

  // order now has # of 1's, 2's, and 3's to reflect # of 3 types of atoms,
  // so just need to shuffle to get random order
  // unless special test where keep all sulfurs first
  if (!sulfurFirst) shuffle(order, total);

  // now create threads in shuffled order 
  pthread_t atoms[total];
  for (i=0; i<total; i++) {
    if (order[i]==1) pthread_create(&atoms[i], NULL, oxygen, NULL);
    else if (order[i]==2) pthread_create(&atoms[i], NULL, hydrogen, NULL);
    else if (order[i]==3) pthread_create(&atoms[i], NULL, sulfur, NULL);
    else printf("something went horribly wrong!!!\n");
    if (i%2==0) delay(rand()%2000);

  }

  // join all threads before letting main exit
  for (i=0; i<total; i++) {
    pthread_join(atoms[i], NULL);
  }

  // close and unlink all semaphores
  closeSems();
  
  return 0;
}

void shuffle(int* intArray, int arrayLen) {
  int i=0;
  for (i=0; i<arrayLen; i++) {
    int r = rand()%arrayLen;
    int temp = intArray[i];
    intArray[i] = intArray[r];
    intArray[r] = temp;
  }
}
/*
 * NOP function to simply use up CPU time
 * arg limit is number of times to run each loop, so runs limit^2 total loops
 */
void delay( int limit )
{
  int j, k;

  for( j=0; j < limit; j++ )
    {
      for( k=0; k < limit; k++ )
        {
        }
    }
}

