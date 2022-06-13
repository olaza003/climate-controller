/* 
* Demo Link: https://youtu.be/Oo0vbSWfjl0 
*/
#include <IRremote.hpp> // https://github.com/Arduino-IRremote/Arduino-IRremote#api

enum States{INIT , OFF, ON, SM_Sensor, SM_Button, Button} SM1_state = OFF;

int receiver = 7;
const int X = 8;
const int trigPin = 9; //echo
const int echoPin = 10; //echo
IRrecv irrrecv(receiver);
decode_results results;
//3125149440 == power button

//lights
const int b_size = 4;
const int b[b_size] = {2, 6, 11, 13}; //up to down
int b_buf = 0x00;


uint32_t Previous;
const int IR_light = 12;


const int speedPin = 5; //fan
const int motorIn1 
= 3; //fan (high)
const int motorIn2 
= 4; //fan (low)
//int mSpeed = 0;

unsigned long lastRan;
unsigned long period;


//ultrasonic: https://www.youtube.com/watch?v=ZejQOX69K5M&t=158s
//ir remote: https://www.youtube.com/watch?v=Y2HC3bdFoPw&ab_channel=PaulMcWhorter
//also ir remote: https://www.youtube.com/watch?v=3jeSfsnQOWk&ab_channel=Fungineers
//dc fan: https://www.youtube.com/watch?v=fPLEncYrl4Q&ab_channel=PaulMcWhorter
long duration; 
int distance;

typedef struct task {
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
    
} task;

int delay_gcd;
const unsigned short tasksNum = 1;
task tasks[tasksNum];

void resetBuffer() {
    for(int i = 0; i < b_size; i++) {
        // Note this is an arduino function call to the pins
        digitalWrite(b[i], LOW);
    }
}
    
void writeBuffer(unsigned char b_temp, int size = b_size)
{
    for (int i = (size - 1); i >= 0; i--) {
        if ((b_temp >> i) & 0x01) {
        digitalWrite(b[i], HIGH);
        }
  }
}



//period = 50ms
bool check = false;
int counter, sensor_timer; //checks time
int button_press; //many times the button is pressed;
bool godlight = false;//idk y the 0x01 light not working in normal way :(
int SM_timer; //if reaches 1 min we go to off; since period is 10ms it will be 6000
int SM1_Tick(int state){
  switch(state){
    
    case INIT: 
        state = OFF;
    break;

    case OFF:
         state = OFF;
         Serial.println("OFF");
         Serial.println(IrReceiver.decodedIRData.decodedRawData);
          //remote pressed
         if(check)
         {
          state = ON;
          Serial.println("trying to switch");
          counter = 0;
          check = false;
         }
         resetBuffer();
    break;

    case ON:
          state = ON;
          Serial.println("ON");
           //remote pressed
           if(check)
           {
              state = OFF;
              check = false;
              break;
           }

          if(digitalRead(X) == LOW){ 
            state = SM_Button;
            SM_timer=0; 
            button_press = 0;
            break;
          }
          resetBuffer();
          
          if(distance <= 8){
            state = SM_Sensor;
            SM_timer = 0;
          }
    
          
     break;

     case SM_Sensor:
          state = SM_Sensor;
          Serial.println("SM_sensor");
        if(check || SM_timer >= 60) //remote pressed
           {
              state = OFF;
              check = false;
              
           }
            SM_timer++;
       break;


       case SM_Button:
             state = SM_Button;
             Serial.println("SM_Button");
        if(check|| SM_timer >= 60) //remote pressed
           {
              check = false;
              state = OFF;
           }
           SM_timer++;
           if(digitalRead(X) == LOW){state = Button; button_press++;}
           if(button_press == 5){ button_press = 0;}

       break;

       case Button:
            if(digitalRead(X) == LOW){state = Button;}
            else{state = SM_Button;}
       break;
  }

  switch(state){
    
    case INIT:

    break; 

    case OFF:
          resetBuffer();
          digitalWrite(IR_light,LOW);//the light next to IR is off
          clockwise(0);
    break;

    case ON:
          digitalWrite(IR_light,HIGH);//the light next to IR is on
          resetBuffer();
          clockwise(0);
    break;

    case SM_Sensor:

          if(distance <= 8 && distance > 6){ //25%
            writeBuffer(0x01);
            clockwise(100);
            godlight = true;
            Serial.print("Fan speed: ");
            Serial.println(10);
          }
          else if(distance <= 6 && distance > 4){ //50%
            writeBuffer(0x03);
            clockwise(150);
            Serial.print("Fan speed: ");
            Serial.println(150);
          }
          else if(distance <= 4 && distance > 2){ //75%
            writeBuffer(0x07);
            clockwise(200);
            Serial.print("Fan speed: ");
            Serial.println(200);
          }
          else if(distance <= 2){ //100%
            writeBuffer(0x0F);
            clockwise(250);
            Serial.print("Fan speed: ");
            Serial.println(250);
          }
          else{ //0%
            resetBuffer();
            clockwise(0);
            godlight = false;
            Serial.print("Fan speed: ");
            Serial.println(0);
          }
          
          if(distance <= 8){
            state = SM_Sensor;
            SM_timer = 0;
          }

          
    break;

    case SM_Button:
        Serial.print("Button num: ");
        Serial.println(button_press);
        if(button_press == 1){ //25%
          writeBuffer(0x01);
          clockwise(100);
          godlight = true;
          Serial.print("Fan speed: ");
          Serial.println(100);
        } 
        else if(button_press == 2){ //50%
          writeBuffer(0x03);
          clockwise(100);
          Serial.print("Fan speed: ");
          Serial.println(150);
        }
        else if(button_press == 3){ //75%
          writeBuffer(0x07);
          clockwise(140);
          Serial.print("Fan speed: ");
          Serial.println(200);
        }
        else if(button_press == 4){ //100%
           writeBuffer(0x0F);
           clockwise(200);
           Serial.print("Fan speed: ");
           Serial.println(250);
        }
        else{ //0%
          resetBuffer();
          clockwise(0);
          godlight = false;
          Serial.print("Fan speed: ");
          Serial.println(0);
        }
    break;
  }

  return state;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //irrecv.enableIRIn();//start the receiver
  IrReceiver.begin(receiver);
  pinMode(X, INPUT_PULLUP); //default low

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(speedPin, OUTPUT);
  pinMode(motorIn1,OUTPUT);  //initialize the motorIn1 pin as output 

  pinMode(motorIn2,OUTPUT);  //initialize the motorIn2 pin as output 

  for(int k = 0; k < b_size; k++)
    {
        pinMode(b[k], OUTPUT);
    }
    period = 1000;
    lastRan = 0;

      unsigned char i = 0;
  tasks[i].state = SM1_Tick;
  tasks[i].period = 500;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM1_Tick;
  delay_gcd = 500;
}

void loop() {
  // put your main code here, to run repeatedly:
  if(IrReceiver.decode()){
    Serial.print("What is in IR: ");
      Serial.println(IrReceiver.decode());
      //if(!check){check = true;}
      Serial.println(IrReceiver.decodedIRData.decodedRawData);
      if(IrReceiver.decodedIRData.decodedRawData > 0){Serial.println("Tesst");check = true;}
      IrReceiver.resume();
  }
  digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH);
    delay(25);
    distance = duration * 0.034/2;

  unsigned char i;
  for(i = 0; i < tasksNum; i++){
    if((millis() - tasks[i].elapsedTime) >= tasks[i].period ){
      tasks[i].state = tasks[i].TickFct(tasks[i].state);
      tasks[i].elapsedTime = millis();
    }
  }
    /*      
  if((millis() - lastRan) > period){
    SM1_state = (States)SM1_Tick(SM1_state);
    lastRan = millis();
  } */

  if(godlight){writeBuffer(0x01);}
}

void clockwise(int
Speed)

{
  digitalWrite(motorIn1, HIGH);
   digitalWrite(motorIn2, LOW);
   analogWrite(speedPin, Speed);
}
