#include <math.h> 
#include <stdlib.h>

// components and magnitude of vector
typedef struct coordinates{
  float x;
  float y;
  float z;
  float magnitude;
  //float theta;
  //float phi
} coordinates;

//typedef struct {
//  int x;
//  int y;
//  int z;
//  int d;
//} plane;

float distance = 0;
coordinates s;
coordinates u;
coordinates v;
coordinates acceleration;
float t = 1; //whatever time the interupt is

int steps = 0;
int walking = 0; //the boolean one for deciding walking/running

// FUNCTION TO GET XYZ ACCELERATION AS A TIMER INTERRUPT
// GETS VALUES FROM PINS EVERY t SECONDS
// CALLLS MAGNITUDE AND SUVAT FUNCTIONS
// CHECKS IF ACCELERATION  AND VELOCITY IS WITHIN RANGE/IF RUNNING
// CHECKS IF IT SHOULD ADD A STEP


/*coordinates toPolar(polar vector){ // finds polar values
  vector.magnitude = sqrt(sq(vector.x) + sq(vector.y) + sq(vector.z));
  vector.theta = atan(vector.y, vector.x);
  vector.phi = atan(sqrt(sq(vector.x) + sq(vector.y)), vector.z);
  return vector;
}*/

/*coordinates toCartesian(polar vector){ // find cartesian values
  vector.x = vector.magnitude*cos(vector.theta)*sin(vector.phi);
  vector.y = vector.magnitude*sin(vector.theta)*sin(vector.phi);
  vector.z = vector.magnitude*cos(vector.phi);
  return vector;
}*/

struct coordinates calculateMagnitude(coordinates vector){
  vector.magnitude = sqrt(sq(vector.x) + sq(vector.y) + sq(vector.z));
  return vector;
}
  
void suvat(){ // calculates missing suvat values
  // displacement (s)
  s.x = u.x*t + 0.5*acceleration.x*sq(t);
  s.y = u.y*t + 0.5*acceleration.y*sq(t);
  s.z = u.z*t + 0.5*acceleration.z*sq(t);
  calculateMagnitude(s);
  // velocity (u)
  u = v;
  // final velocity (v)
  v.x=sqrt(sq(u.x)+2*acceleration.x*s.x);
  v.y=sqrt(sq(u.y)+2*acceleration.y*s.y);
  v.z=sqrt(sq(u.z)+2*acceleration.z*s.z);
  calculateMagnitude(v);
  // distance
  distance = distance + s.magnitude;
}

/*void stepped(){
  if(walking == true){
    if(acceleration.magnitude<0.5){
      step = step + 1;
    }
  }
}*/



















