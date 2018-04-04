#include <QuickStats.h>

// Example program for use with QuickStats.h 

//#include "QuickStats.h"
 
//int numreadings = 9;
//float readings[]= {1.0,2.2,4.8,3.3,6.1,2.2,3.8,7.0,2.2};

int numreadings = 80;
int i, counter;
//float readings[]= {1.4, 2.4, 3.4, 1.4, 2.4, 0.4, 3.4, 4.4, 5.4, 1.4, 1.4, 2.4, 3.4, 9.4, 0.4, 1.4, 1.4, 0.4, 0.4, 0.4, 3.4, 1.4, 1.4, 2.4, 3.4, 3.4, 1.4, 1.4, 2.4, 2.4, 3.4, 4.4, 5.4, 2.4, 2.4, 3.4, 4.4, 5.4, 1.4, 2.4, 2.4, 0.4, 0.4, 2.4, 7.4, 8.4, 7.4, 7.4, 7.4, 8.4, 3.4, 4.4, 2.4, 3.4, 2.4, 8.4, 0.4, 3.4, 4.4, 5.4, 2.4, 1.4, 4.4, 5.4, 2.4, 2.4, 3.4, 4.4, 5.4, 0.4, 2.4, 2.4, 0.4, 0.4, 2.4, 7.4, 8.4, 7.4, 7.4, 7.4, 8.4, 3.4, 1.4, 2.3, 3.3, 1.3, 2.3, 0.3, 3.3, 4.3, 5.3, 1.3, 1.3, 2.3, 3.3, 9.3, 0.3, 1.3, 1, 0.1};
//float readings[]= {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
//float readings[]= {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

//80 data heterogen
float readings[] = {1, 2, 3, 1, 2, 0, 3, 4, 5, 1, 1, 2, 3, 9, 0, 1, 1, 0, 0, 0, 3, 1, 1, 2, 3, 3, 1, 1, 2, 2, 3, 4, 5, 2, 2, 3, 4, 5, 1, 2, 2, 0, 0, 2, 7, 8, 7, 7, 7, 8, 3, 4, 2, 3, 2, 8, 0, 3, 4, 5, 2, 1, 4, 5, 2, 2, 3, 4, 5, 0, 2, 2, 0, 0, 2, 7, 8, 7, 7, 7};


//float readings[100];

QuickStats stats; //initialize an instance of this class

void setup()
{
  Serial.begin(9600); 
//  for(i=0; i<100; i++){
//    readings[i] = counter;
//    counter++;
//  }
  Serial.println("Descriptive Statistics");
  Serial.print("Average: ");
  Serial.println(stats.average(readings,numreadings));
  Serial.print("Geometric mean: ");
  Serial.println(stats.g_average(readings,numreadings));
  Serial.print("Minimum: ");
  Serial.println(stats.minimum(readings,numreadings));
  Serial.print("Maximum: ");
  Serial.println(stats.maximum(readings,numreadings));
  Serial.print("Standard Deviation: ");
  Serial.println(stats.stdev(readings,numreadings));
  Serial.print("Standard Error: ");
  Serial.println(stats.stderror(readings,numreadings));
  Serial.print("Coefficient of Variation (%): ");
  Serial.println(stats.CV(readings,numreadings));
  Serial.print("Median: ");
  Serial.println(stats.median(readings,numreadings));
  Serial.print("Mode: ");
  Serial.println(stats.mode(readings,numreadings,0.00001));
}
 
void loop()
{
}
