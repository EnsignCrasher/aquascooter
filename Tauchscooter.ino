#define speedButtonPin 2
#define voltPin A0
#define escOutPin 9
#define ledPin 13
//#define calibrateESC
//#define _debug 
#ifdef _debug
  #define dprintf(string) Serial.println(string)
#else
  #define dprintf(string)
#endif

#include <Servo.h>
Servo esc;
int i_speed = 0;
int esc_thr = 20;
bool ledState = false;

struct taskDefinition{
  long lastUpdate = 0;
  long period = 0;
};

enum taskID{
  updateButton,
  changeSpeed,
  updateESC,
  toggleLED
};

taskDefinition task[4];

void initTasks(){
  task[updateButton].period = 5;
  task[changeSpeed].period = 15; //this will increase speed from 0 to 255 in ca. 3,8s
  task[updateESC].period = 5;
  task[toggleLED].period = 0;
}

void setLastUpdate(taskID id){
  task[id].lastUpdate = millis();
}

bool isCalled(taskID id)
{
  if(millis()>(task[id].lastUpdate + task[id].period) && task[id].period > 0){
      setLastUpdate(id);
      return true;
  }else{
    return false;
  }
}

class Button{
  private:
    bool lastState;
    long t_lastStateChange;
    long t_debounceDelay;
    int buttonPin;
    bool invert = true;
  public:
    bool state;
    void initButton(int pin, long t_debounce){
      pinMode(pin, INPUT_PULLUP);
      buttonPin = pin;
      t_lastStateChange = millis();
      lastState = digitalRead(pin);
      t_debounceDelay = t_debounce;
    }
    void updateState(){ 
        bool currentState = digitalRead(buttonPin);
        if(invert)
          currentState = !currentState;
        //currentState ? dprintf(String(__func__) + " : button on pin " + String(buttonPin) + " pressed"):dprintf(String(__func__) + " : button on pin " + String(buttonPin) + " released");
        if(lastState != currentState){
          t_lastStateChange = millis();
          lastState = currentState;
        }

        if((millis()-t_lastStateChange) > t_debounceDelay){
          //seems like we got a stable state
          state = lastState;
        }
        
    }
};

Button speedSwitch;

void setLEDcycle(){
  if(i_speed){
    task[toggleLED].period = 256-i_speed;
  }
  else{
    task[toggleLED].period = 0;
    digitalWrite(ledPin, LOW);
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(voltPin, INPUT);
  pinMode(ledPin, OUTPUT);
  speedSwitch.initButton(speedButtonPin, 50);
  initTasks();
  esc.attach(escOutPin);
  #ifdef _debug
    Serial.begin(9600);
  #endif
}

void loop() {
  if(isCalled(updateButton))
    speedSwitch.updateState();
#ifdef calibrateESC
  if(!speedSwitch.state)
    esc.write(20);
  else
    esc.write(230);

#else
  
  

  if(isCalled(changeSpeed)){
    //speedSwitch.state? dprintf("button pressed") : dprintf("button released");
    
    if(speedSwitch.state && i_speed < 255){
      //dprintf("increase speed");
      i_speed++;
    }else if(!speedSwitch.state && i_speed > 0){
      //dprintf("decrease speed");
      if(i_speed > 125)
        i_speed = 125;
      i_speed--;
    }
  }


  if(isCalled(updateESC))
  {
    dprintf("set speed: " + String(i_speed));
    
    setLEDcycle();

    if(i_speed < esc_thr)
      esc.write(esc_thr);
    else
      esc.write(i_speed);
  }

  if(isCalled(toggleLED)){
    ledState = !ledState;
    ledState?digitalWrite(ledPin, HIGH):digitalWrite(ledPin, LOW);
  }
#endif
}
