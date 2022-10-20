#include<Wire.h>

#define SLAVE_ADD 4

unsigned long TIME_TO_POUR = 14000;
unsigned long TIME_ONE_SHOT = 4000;

byte ratio1 = 0;
byte ratio2 = 0;
byte ratio3 = 0;

byte isShot = 0;

int piezo = 4;

unsigned long piezoDelay = millis();

int i, duration, j;
char note;

 

int songLengthChoosing = 44;
int songLengthMaking = 62;
int songLengthFinished = 6;
bool isStarting = true;
bool isFinished = false;

 
// initialising the arrays with the notes for our songs

// Jingle bells
char melodyChoosing[] = {'e',' ','e', ' ', 'e', ' ', 'e', ' ' ,'e', ' ', 'e', ' ' ,'e', ' ', 'g' , ' ', 'c','d','e', ' ', 'f', ' ', 'f',' ', 'f', ' ', 'f','f', ' ', 'e', ' ', 'e', ' ', 'e', 'e', 'e', ' ', 'd', ' ' ,'d','e', 'd', 'g', ' '};

// The Pina Colada Song
char melodyMaking[] = {'e','e','e','e','d','c','c','e',' ', 'c','d','c','e','d','c','c',' ', 'e','e','e','d','c','c','e','d','c','c', ' ', 'g','g','g','g','g','g','a','e','d','c','d',' ', 'd', 'c', 'e', 'd', 'c','c',' ', 'e', 'e', 'e', 'd', 'c', 'c', 'e' , ' ', 'd', 'c', 'e', 'd', 'c', 'c', ' '}; 

// Custom finish Chime
char melodyFinished[] = {' ','f','g','b','c', ' '};

 

// beatlength for each note
double beatsChoosing[] = {1,1,1,1,4,1,1,1,1,1,4,1,2,0.5,2,0.5,4,1,7,0.5,1,1,1,1,3,1,1,2,0.5,1,1,1,1,1,1,2,0.5,1,1,2,2,5,4,0.5};
double beatsMaking[] = {1,1,1,2,1,1,2,7,1,1,1,1,2,1,1,5,7,1,1,2,1,1,2,7,2,1,1,2,1,1,5,6,1,1,1,1,1,1,1,1,1,1,6,2,1,1,5,1,1,7,2,1,1,2,1,1,2,7,2,1,6};
double beatsFinished[] = {2,1,1,1,4,3};

 
// tempo of each song
int tempoChoosing = 125;
int tempoMaking = 200;
int tempoFinished = 300;

int leftBottle = 10;
int middleBottle = 11;
int rightBottle = 12;

unsigned long t1 = millis();

int beatIdx = 0;


/**
 * reading data received from master Arduino
 */
void receiveEvent (int number) {
     isShot = Wire.read();
     ratio1 = Wire.read();
     ratio2 = Wire.read();
     ratio3 = Wire.read();
}

void setup() {
  Serial.begin(9600);
  pinMode(piezo, OUTPUT);
  pinMode(leftBottle, OUTPUT);
  pinMode(middleBottle, OUTPUT);
  pinMode(rightBottle, OUTPUT);
  // setting all relays to off
  digitalWrite(leftBottle, HIGH);
  digitalWrite(middleBottle, HIGH);
  digitalWrite(rightBottle, HIGH);
  //beginning I2C in slave mode
  Wire.begin(SLAVE_ADD);
  // on Receive, go to the function taking in the inputs from the master
  Wire.onReceive(receiveEvent);
  

}

void loop() {
  if (isFinished) {
    Serial.println(String(millis() - piezoDelay));
    // Finish chime
    if (millis() - piezoDelay >= duration + tempoFinished / 10) { 
          piezoDelay = millis();
          duration = beatsFinished[i] * tempoFinished;
          note = melodyFinished[i];   
      
          if (note != ' '){
            tone(piezo, noteToFrequency(note, songLengthFinished), duration);
          }
          i++;
          if (i == songLengthFinished){
            i = 0;
            isFinished = false;
          }
       } 
  }
  isStarting = true;
  if ((ratio1 != 0 || ratio2 != 0 || ratio3 != 0) && isShot == 0) {
    if (isStarting) i = 0;
    isStarting = false;
    
    t1 = millis();
    int totalRatio = ratio1 + ratio2 + ratio3;
    // we do not pour for the same amount of time for every drink
    // this is because each drink might use 2 or more dispensers simultaneously
    // this means that there is a higher rate of flow when more dispensers are active
    // thus we need a shorter time if this is the case
    while (millis() - t1 <= TIME_TO_POUR * (double) maximum(ratio1, ratio2, ratio3) / totalRatio) {
         if (millis() - piezoDelay >= duration + tempoMaking / 10) { // use this to check for how much time has passed instead of using a delay, to keep our program in a tight loop
          piezoDelay = millis();
          duration = beatsMaking[i] * tempoMaking;
          note = melodyMaking[i];   
      
          if (note != ' '){
            tone(piezo, noteToFrequency(note, songLengthMaking), duration); // play a note if it is not a pause
          }
          i++;  // increase the index of where we are in the array of the current melody
          if (i == songLengthMaking){
            i = 0;  //reset the song to the beginning once it gets to the end
          }
       } 
       Serial.println(String(TIME_TO_POUR * ((double)maximum(ratio1, ratio2, ratio3) / totalRatio)) + " " + String(millis() - t1));

        // for the following if statements, we turn on or off each dispenser depending on its ratio
        if (millis() - t1 <= TIME_TO_POUR * ((double)ratio1 / totalRatio)) {
          digitalWrite(leftBottle, LOW);
        } else {
          digitalWrite(leftBottle, HIGH);
        }
    
        if (millis() - t1 <= TIME_TO_POUR * ((double)ratio2 / totalRatio)) {
            digitalWrite(middleBottle, LOW);
          } else {
            digitalWrite(middleBottle, HIGH);
          }
      
          if (millis() - t1 <= TIME_TO_POUR * ((double)ratio3 / totalRatio)) {
            digitalWrite(rightBottle, LOW);
          } else {
            digitalWrite(rightBottle, HIGH);
          }
       }
       // once pouring is done, reset the ratios to stop pouring further  
       ratio1 = 0;
       ratio2 = 0;
       ratio3 = 0;
       isStarting = true;
       i = 0;
       isFinished = true;
       piezoDelay = millis();
  }  else if (isShot == 1) {
    if (isStarting) i = 0;
    isStarting = false;
    t1 = millis(); 
    while(millis() - t1 <= TIME_ONE_SHOT * (ratio1 + 1)) {  // we use ratio1 to indicate whether this is a single or a double shot
      if (millis() - piezoDelay >= duration + tempoMaking / 10) { 
        piezoDelay = millis();
        duration = beatsMaking[i] * tempoMaking;
        note = melodyMaking[i];   
    
        if (note != ' '){
          tone(piezo, noteToFrequency(note, songLengthMaking), duration);
        }
        i++;
        if (i == songLengthMaking){
          i = 0;
        }
     } 
      digitalWrite(leftBottle, LOW);
    }
    digitalWrite(leftBottle, HIGH);
    isShot = 0;
    ratio1 = 0;
    isStarting = true;
    i = 0;
    isFinished = true;
    piezoDelay = millis();
  } else if (!isFinished){
    //Serial.println(i);
    if (millis() - piezoDelay >= duration + tempoChoosing / 10) { 
      piezoDelay = millis();
      duration = beatsChoosing[i] * tempoChoosing;
      note = melodyChoosing[i];   
  
      if (note != ' '){
        tone(piezo, noteToFrequency(note, songLengthChoosing), duration);
      }
      i++;
      if (i == songLengthChoosing){
        i = 0;
      }
    } 
  }
}


/**
 * Returns the maximum value from 3 bytes
 */
int maximum(byte r1, byte r2, byte r3) {
  if (r1 > r2) {
    if (r1 >= r3) {
      return r1;
    } else {
      return r3;
    }
  } else {
    if (r2 > r3) {
      return r2;
    } else {
      return r3;
    }
  }
}


/**
 * converts each note to a frequency to be played by the piezo
 */
int noteToFrequency(char note, int songLength)
{
  char notes[] = {'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C'};
  int numOfNotes = 8;
  int frequencies[] = {262, 294, 330, 349, 392, 440, 494, 523};
  
  for (int j = 0; j < numOfNotes; j++) {
    
    if (note == notes[j]) {
      return frequencies[j];
    }
    
  }
}
