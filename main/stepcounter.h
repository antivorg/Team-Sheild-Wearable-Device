////////////////////////all the declerations

// components and magnitude of vector
typedef struct coordinates{
  float x;
  float y;
  float z;
  float magnitude;
  //float theta;
  //float phi
} coordinates;

int times=0;
//float distance = 0; // total distance
//coordinates s; // displacement
//coordinates u; // velocity of the last milisecond
//coordinates v; // velocity of this milisecond
//coordinates acceleration; // acceleration of this milisecond
float t = 0.001; // how long the interupt is, 1 milisecond

int steps = 0; // step counter
//int walking = 0; //the boolean one for deciding walking/running
//float angle;
int lasttime;
int high=0;
float x[100];
float y[100];
float z[100];

int totalx=0;
int totaly=0;
int totalz=0;

////////////////////setup


void setup() {
  Serial.begin(9600);
  pinMode(A0 , INPUT); //a.x
  pinMode(A1 , INPUT); //a.y
  pinMode(A2 , INPUT); //a.z
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  // sets up timer0 interrupt to be called for every millisecond
  cli();
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  sei();
  // sets pins as inputs
}


/////////// does everything


void getAcceleration(){
  x[times%25] = analogRead(A0);
  if(times%25==0){
    for(int i=0;i<100;i++){
      totalx+=x[i];
    }
    acceleration.x=totalx/25;
    if(acceleration.x<0){
      acceleration.x=4.65*(9.81/0.33)*(acceleration.x+323)/1023;
    }
    else{
      acceleration.x=4.65*(9.81/0.33)*(acceleration.x-324)/1023;
    }
    totalx=0;
    if(abs(acceleration.x)<0.3){
      acceleration.x=0;
    }
  }
  y[times%25] = analogRead(A1);
  if(times%25==0){
    for(int i=0;i<100;i++){
      totaly+=y[i];
    }
    acceleration.y=(totaly/25)+320;
    acceleration.y=4.65*(9.81/0.33)*(acceleration.y)/1023;
    if(acceleration.y<0){
      acceleration.y+=0.5;
    }
    else{
      //acceleration.y-=3;
    }
    totaly=0;
    if(abs(acceleration.y)<0.16){
      acceleration.y=0;
    }
  }
  z[times%25] = analogRead(A2);
  if(times%25==0){
    for(int i=0;i<100;i++){
      totalz+=z[i];
    }
    acceleration.z = totalz/25;
    acceleration.z=4.65*(9.81/0.33)*(acceleration.z-270)/1023;
    if(acceleration.z<0){
      acceleration.z+=0.41;
    }
    else{
      acceleration.z-=0.41;
    }
    totalz=0;
    if(abs(acceleration.z)<0.16){
      acceleration.z=0;
    }
  }
  acceleration.magnitude = sqrt(sq(acceleration.x) + sq(acceleration.y) + sq(acceleration.z));
  if(abs(acceleration.x)>1.3 && times>600){
      digitalWrite(13, HIGH);
      steps++;
      times=0;
      Serial.println(steps);
      Serial.println(acceleration.x);
  }
  else if(times==500){
    digitalWrite(13, LOW);
    times++;
  }
  else{
    times++;
  }
}

///////////////// timer

SIGNAL(TIMER0_COMPA_vect) {
  cli();
  getAcceleration();
  //if(times%25==0){
    //suvat();
  //}
  //stepped();
  //times++;
  //if(times>600){
    //if(high==0){
    //}
    //else{
      //digitalWrite(13, LOW);
      //high=0;
    //}
    //Serial.println('a');
    //Serial.println(angle);
    //Serial.println('m');
    //Serial.println(steps);
    //Serial.println(acceleration.x);
  //}
  sei();
}


/////////////////////////////////////////unused	:'(
/*
void loop() {
  //delay(1000);
  //Serial.println(v.magnitude);
  //Serial.println(v.z);
}

struct coordinates calculateMagnitude(coordinates vector){ // calculates magnitud
  vector.magnitude = sqrt(sq(vector.x) + sq(vector.y) + sq(vector.z));
  return vector;
}

void suvat(){ // calculates components of missing suvat values and their magnitudes
  // displacement (s)
  s.x = u.x*t + 0.5*acceleration.x*sq(t);
  s.y = u.y*t + 0.5*acceleration.y*sq(t);
  s.z = u.z*t + 0.5*acceleration.z*sq(t);
  s.magnitude = sqrt(sq(s.x) + sq(s.y) + sq(s.z));
  // velocity (u)
  u = v;
  // final velocity (v)
  v.x=sqrt(sq(u.x)+abs(2*acceleration.x*s.x));
  v.y=sqrt(sq(u.y)+abs(2*acceleration.y*s.y));
  v.z=sqrt(sq(u.z)+abs(2*acceleration.z*s.z));
  //if(v.magnitude==v.magnitude = sqrt(sq(v.x)/* + sq(v.y) + sq(v.z)*//*);){
  //}
  v.magnitude = sqrt(sq(v.x)/* + sq(v.y) + sq(v.z)*//*);
  if(v.magnitude<1){
    v.magnitude=0;
  }
  else{
    if((lasttime%60-times%60)>50 && high==0){
      steps++;
      digitalWrite(13, HIGH);
      high=1;
      lasttime=times;
      times=0;
      Serial.println('a');
    }
  }
  // distance
  distance = distance + s.magnitude; // calculates total distance
}
*/