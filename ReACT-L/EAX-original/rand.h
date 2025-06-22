#ifndef __RAND__
#define __RAND__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <random> // ����ִ��������
#include <ctime>

extern void InitURandom( int );    
extern void InitURandom( void );   

class TRandom {
private:
    std::mt19937_64 engine; // Mersenne Twister����
 public:
  TRandom();
  ~TRandom();
  int Integer( int minNumber, int maxNumber ); 
  double Double( double minNumber, double maxNumber );
  void Permutation( int *array, int numOfelement, int numOfSample );
  double NormalDistribution( double mu, double sigma );
  void Shuffle( int *array, int numOfElement );
};

extern TRandom* tRand;


#endif


