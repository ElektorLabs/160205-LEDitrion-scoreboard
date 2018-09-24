#include <TimerOne.h>



const uint8_t leds[7] = {2, 3, 4, 5, 6, 7, 8};
const uint8_t seg[4] = {9, 10, 11, 12};
const uint8_t dotPin = 13;
int curSeg = 0;

//two buttons per pin. Connected to analog inputs
const uint8_t button1 = A0;
const uint8_t button2 = A1;
const uint8_t jumper = A2;
byte buttonsPressed = 0;
#define S1 0
#define S2 1
#define S3 2
#define S4 3


typedef enum {
  CAS_PHASE=0,
  RAS_PHASE,
  DIGIT_WRITE
}fsm_state_t;

typedef enum {
  SCOREBOARD=0,
  TIMER,
  CLOCK
} function_t;

volatile uint8_t seconds=0;
volatile uint8_t hours=0;
volatile uint8_t minutes=0;
volatile uint32_t timestamp_centisec=0;


volatile uint16_t count = 0;

unsigned long timer = 0;
volatile bool running = false;
uint8_t dots = 0x0F; //bit 0 = dot 1, bit 1 = dot 2 ...

void writeDisplay( void );
void TimeKeeper( void );
void increment_time_one_second( void );
void increment_time_one_minute( void );
void increment_time_one_hour( void );
void decrement_time_one_minute( void );
void decrement_time_one_hour( void );

void timer_mode ( void );
void scoreboard_mode( void );
void clock_mode( void );

function_t mode=CLOCK;

void setup() {
  // put your setup code here, to run once:
 int modepin_vlaue=512;
  for (int i = 0; i < 7; i++)
    pinMode(leds[i], OUTPUT);
  for (int i = 0; i < 4; i++)
    pinMode(seg[i], OUTPUT);
  pinMode(dotPin, OUTPUT);

  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(jumper, INPUT);

  Serial.begin(115200);
  Timer1.initialize(500); //500us
  Timer1.attachInterrupt(writeDisplay);
  dots=0x0B;

  modepin_vlaue=analogRead(A2);
  if(modepin_vlaue<100){
    mode=TIMER;
    Serial.println("TIMER MODE"); 
  } else if(modepin_vlaue>650){
    Serial.println("SCOREBOARD MODE"); 
    mode=SCOREBOARD;
  } else {
    Serial.println("CLOCK MODE");
    mode=CLOCK;
  }
  
  
  
}

void loop() {

  // put your main code here, to run repeatedly:

     
  switch (mode ){
    case CLOCK:{
      clock_mode();
    } break ;

    case TIMER:{
      timer_mode();
    } break;

    case SCOREBOARD:{
      scoreboard_mode();
    } break;

    default:{
       clock_mode();
    } break;
  }


}

void scoreboard_mode(  ){

  dots = 0x0F;
  
    if (analogRead(button1) > 500 && ((buttonsPressed >> S1) & 1) != 1) {//S1 pressed -> score 1 +
      buttonsPressed |= (1 << S1);
      if (count / 100 != 99){
        count += 100;
         Serial.println(" +100 ");
      }
        
    }
    if (analogRead(button1) <= 500)
      buttonsPressed &= ~(1 << S1);
    if (analogRead(button1) < 20 && ((buttonsPressed >> S2) & 1) != 1) {//S2 pressed -> score 1 -
      buttonsPressed |= (1 << S2);
      if (count / 100 != 0){
        count -= 100;
        Serial.println(" -100 ");
      }
    }
    if (analogRead(button1) >= 20)
      buttonsPressed &= ~(1 << S2);
    if (analogRead(button2) > 500 && ((buttonsPressed >> S3) & 1) != 1) {//S3 pressed -> score 2 +
      buttonsPressed |= (1 << S3);
      if (count % 100 != 99){
        count++;
         Serial.println(" +1 ");
      }
    }
    if (analogRead(button2) <= 500)
      buttonsPressed &= ~(1 << S3);
    if (analogRead(button2) < 20 && ((buttonsPressed >> S4) & 1) != 1) {//S4 pressed -> score 2 -
      buttonsPressed |= (1 << S4);
      if (count % 100 != 0){
        count--;
          Serial.println(" +1 ");
      }
    }
    if (analogRead(button2) >= 20)
      buttonsPressed &= ~(1 << S4);

}

void timer_mode (  ){

   dots = 0x0B;
    if ((analogRead(button1) > 500 || analogRead(button1) < 20) && ((buttonsPressed >> S1) & 1) != 1) {
      buttonsPressed |= (1 << S1);
      running = true;
    }
    if (analogRead(button1) >= 20 && analogRead(button1) <= 750)
      buttonsPressed &= ~(1 << S1);
      
    if ((analogRead(button2) > 500 || analogRead(button2) < 20) && ((buttonsPressed >> S2) & 1) != 1) {
      buttonsPressed |= (1 << S2);
      if (running == true)
        running = false;
      else { //reset timer
       timestamp_centisec=0;
      }
    }
    if (analogRead(button2) >= 20 && analogRead(button2) <= 750)
      buttonsPressed &= ~(1 << S2);

    
    
}

void clock_mode(  ){

      if (analogRead(button1) > 400 && ((buttonsPressed >> S1) & 1) != 1) {//S1 pressed -> score 1 +
      buttonsPressed |= (1 << S1);
      increment_time_one_hour();
      /* update rtc */
    }
    
    if (analogRead(button1) <= 400){
      buttonsPressed &= ~(1 << S1);
    }
    
    if (analogRead(button1) < 20 && ((buttonsPressed >> S2) & 1) != 1) {//S2 pressed -> score 1 -
      buttonsPressed |= (1 << S2);  
      //increment_time_one_hour();
      decrement_time_one_hour();
    }
    
    if (analogRead(button1) >= 20){
      buttonsPressed &= ~(1 << S2);
    }
    
    if (analogRead(button2) > 400 && ((buttonsPressed >> S3) & 1) != 1) {//S3 pressed -> score 2 +
      buttonsPressed |= (1 << S3);
      increment_time_one_minute();
    }
    
    if (analogRead(button2) <= 400){
      buttonsPressed &= ~(1 << S3);
    }
    
    if (analogRead(button2) < 20 && ((buttonsPressed >> S4) & 1) != 1) {//S4 pressed -> score 2 -
      buttonsPressed |= (1 << S4);  
      decrement_time_one_minute();
    }
    
    if (analogRead(button2) >= 20){
      buttonsPressed &= ~(1 << S4);
    }
}






void increment_time_one_second(){

  seconds++;
  if(seconds>=60){
    seconds=0;
    increment_time_one_minute();
  }

}

void increment_time_one_minute(){

  minutes++;
  if(minutes>=60){
    minutes = 0;
    increment_time_one_hour();
    
  }
}

void increment_time_one_hour(){
  hours++;
  if(hours>=24){
    hours=0;
  }
}

void decrement_time_one_minute(){

  if(minutes==0){
    minutes=59;
    decrement_time_one_hour();
  } else {
    minutes--;
  }
  
}

void decrement_time_one_hour(){ 
  if(hours==0){
    hours=23;
  } else {
    hours--;
  }
}


byte toSegment(int count)
{
  byte segments = 0;
  switch (count)
  {
    case 0:
      segments = B00111111;
      break;
    case 1:
      segments = B00000110;
      break;
    case 2:
      segments = B01011011;
      break;
    case 3:
      segments = B01001111;
      break;
    case 4:
      segments = B01100110;
      break;
    case 5:
      segments = B01101101;
      break;
    case 6:
      segments = B01111101;
      break;
    case 7:
      segments = B00000111;
      break;
    case 8:
      segments = B01111111;
      break;
    case 9:
      segments = B01101111;
      break;
    default:
      segments = 0;
      break;
  }
  return segments;
}

void TimeKeeper(){
  
    static uint16_t second_prescaler = 0;
    if(true == running){ 
      if(second_prescaler%20==0){
        timestamp_centisec++;
      }
    }
    if(second_prescaler>=2000){
      increment_time_one_second();
      second_prescaler=0;
      if( CLOCK == mode ){
        dots^=0x04;
      }
     
    } else {
      second_prescaler++;
    }
}


/* Write display has multiple state now to avoid delays in the isr */
void writeDisplay() {
  static fsm_state_t state=CAS_PHASE;

  int number[4];
  TimeKeeper();
  switch(state){
    case CAS_PHASE: {
      for (int i = 0; i < 4; i++){
        digitalWrite(seg[i], LOW);
      }
      state=RAS_PHASE;
    } break;
    
    case RAS_PHASE:{

      switch ( mode ){

        case CLOCK:{
          number[0] = minutes % 10;
          number[1] = (minutes / 10) % 10;
          number[2] = hours % 10;
          number[3] = (hours / 10) % 10;
        } break;

        case TIMER:{
             /* if we below 60 seconds ( 60000ms) we show ms */
          if(timestamp_centisec<(uint32_t)6000){
            count = (uint16_t)timestamp_centisec;
            number[0] = count % 10;
            number[1] = (count / 10) % 10;
            number[2] = (count / 100) % 10;
            number[3] = (count / 1000) % 10;
          } else {
            /* we show now minutes */
            count = ((uint32_t)timestamp_centisec/100); /* Value in seconds */
            count = count / 60 *100;
            count = count + ( ((uint32_t)timestamp_centisec/100) % 60);
            number[0] = count % 10;
            number[1] = (count / 10) % 10;
            number[2] = (count / 100) % 10;
            number[3] = (count / 1000) % 10;
          }
                
        }break;

        case SCOREBOARD:{
          number[0] = count % 10;
          number[1] = (count / 10) % 10;
          number[2] = (count / 100) % 10;
          number[3] = (count / 1000) % 10;
        } break;

        default:{
          
        } break;
      
      }
      
      byte toWrite = toSegment(number[curSeg]);
      for (int i = 0; i < 7; i++){
        digitalWrite(leds[i], (toWrite >> i) & 1);
      }
      
      digitalWrite(dotPin, (dots >> curSeg) & 1);
      state=DIGIT_WRITE;
    } break;
  
    case DIGIT_WRITE:{
    digitalWrite(seg[curSeg], HIGH);
  
    curSeg++;
      if (curSeg >= 4){
        curSeg = 0;
      }
      state = CAS_PHASE;
    }break;

    default:{
  
    } break;
  }
}

