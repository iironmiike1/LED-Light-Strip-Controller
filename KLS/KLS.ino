
///////////////////////////////////////////////////////////////////////////////////
// Light Strip Controller
// 
// October 29, 2018
//
// Red, Green, Blue, and White Light Strip Controller
// The hardware is based on a 2A 12V power supply, Arduino Nano microcontroller, four
// IRLB8721 MOSFET drivers. There are two light strips; one is white and the other is RGB.
// There is one MOSFET for each color, each controlled by a PWM pin from the Nano.
///////////////////////////////////////////////////////////////////////////////////

const int REVISION_NUMBER = 4;

const int OFF = LOW;
const int ON = HIGH;
const int NIL = -1;

const int DEBUG_MAJOR_IMPACT = 99;
const int DEBUG_MEDIUM_IMPACT = 50;
const int DEBUG_MINOR_IMPACT = 20;
const int DEBUG_MINIMAL_IMPACT = 10;

const int DEBUG_LEVEL = DEBUG_MEDIUM_IMPACT;

const int RedLEDs = 5; // controller hdw dependent PWM pin association
const int GreenLEDs = 6; // controller hdw dependent PWM pin association
const int BlueLEDs = 3; // controller hdw dependent PWM pin association
const int WhiteLEDs = 9; // controller hdw dependent PWM pin association

const int NanoLED = 13; // This is the Arduino Nano's onboard LED pre-defined pin
int NanoLEDState = OFF;

////////////////////////// RGBW Color Presets ////////////////////////////////////

const int FULL_INTENSITY = 255;
const int PALID_INTENSITY = 50;
const int NO_COLOR = 0;

struct color_def
{
  int red;
  int green;
  int blue;
  int white;
};

typedef struct color_def color_type;

color_type red = {FULL_INTENSITY, NO_COLOR, NO_COLOR, NO_COLOR};
color_type green = {NO_COLOR, FULL_INTENSITY, NO_COLOR, NO_COLOR};
color_type blue = {NO_COLOR, NO_COLOR, FULL_INTENSITY, NO_COLOR};
color_type purple = {FULL_INTENSITY, NO_COLOR, PALID_INTENSITY, NO_COLOR};
color_type yellow = {NO_COLOR, FULL_INTENSITY, FULL_INTENSITY, NO_COLOR};
color_type blank = {NO_COLOR, NO_COLOR, NO_COLOR, NO_COLOR};
color_type white = {NO_COLOR, NO_COLOR, NO_COLOR, FULL_INTENSITY};
color_type cool_white = {FULL_INTENSITY, FULL_INTENSITY, FULL_INTENSITY, FULL_INTENSITY};

color_type palid_red = {PALID_INTENSITY, NO_COLOR, NO_COLOR, NO_COLOR};
color_type palid_green = {NO_COLOR, PALID_INTENSITY, NO_COLOR, NO_COLOR};
color_type palid_blue = {NO_COLOR, NO_COLOR, PALID_INTENSITY, NO_COLOR};
color_type palid_blank = {NO_COLOR, NO_COLOR, NO_COLOR, NO_COLOR};
color_type palid_white = {NO_COLOR, NO_COLOR, NO_COLOR, PALID_INTENSITY};
color_type palid_purple = {PALID_INTENSITY, NO_COLOR, PALID_INTENSITY / 8, NO_COLOR};
color_type palid_yellow = {PALID_INTENSITY, PALID_INTENSITY, NO_COLOR, NO_COLOR};

////////////////////Heartbeat / Strobe Settings /////////////////////////////////////////////

const unsigned long MS_PER_MIN = 60000;
const unsigned int MS_PER_SEC = 1000;
const unsigned int SECS_PER_MIN = 60;
const int HEARTBEAT_MS = 200;
const int RANDOM_RANGE = 50;

///////////////////////////////////////////////////////////////////////////////////


void setup()
{
  pinMode(RedLEDs, OUTPUT);
  pinMode(GreenLEDs, OUTPUT);
  pinMode(BlueLEDs, OUTPUT);
  pinMode(WhiteLEDs, OUTPUT);

  redWrite(OFF);
  greenWrite(OFF);
  blueWrite(OFF);
  whiteWrite(OFF);

  pinMode(NanoLED, OUTPUT);

  randomSeed(analogRead(0)); //use noisy pin 0 as the wildcard

  if (DEBUG_LEVEL > OFF) {
    Serial.begin(9600);
    Serial.print("Kitchen Light Strip Revision ");
    Serial.println(REVISION_NUMBER);
  }
  
}

///////////////////////////////////////////////////////////////////////////////////

void loop()
{

  randomColor(2,50,600); // fast color changes (2 sec), 50% interval variability, for 5 minutes)
  
  strobe(palid_white, 5, 600, 5);
  
  for(int hb = 15; hb < 60; hb++){
    heartbeat(palid_red, hb);
  }
  for(int hb = 59; hb > 15; hb--){
    heartbeat(palid_red, hb);
  }
  heartbeat(palid_purple,15);
}

///////////////////////////////////////////////////////////////////////////////////

void showColor(color_type color_val, int duration) {

  analogWrite(RedLEDs, color_val.red);
  analogWrite(GreenLEDs, color_val.green);
  analogWrite(BlueLEDs, color_val.blue);
  analogWrite(WhiteLEDs, color_val.white);
  delay( duration);
}

///////////////////////////////////////////////////////////////////////////////////

void transitionColor(color_type start, color_type finish, int duration) {

  color_type current;
  float red_increment;
  float green_increment;
  float blue_increment;
  float white_increment;

  red_increment = (float)(finish.red - start.red) / (float)duration;
  green_increment = (float)(finish.green - start.green) / (float)duration;
  blue_increment = (float)(finish.blue - start.blue) / (float)duration;
  white_increment = (float)(finish.white - start.white) / (float)duration;
  float_debug(DEBUG_MAJOR_IMPACT, "transitionColor increments", red_increment, green_increment, blue_increment, white_increment);

  current = start;
  for (int i = 0; i < duration; i++) {
    current.red = start.red + (int)(red_increment * (float)i);
    current.green = start.green + (int) (green_increment * (float)i);
    current.blue = start.blue + (int) (blue_increment * (float)i);
    current.white = start.white + (int)(white_increment * (float)i);
    int_debug(DEBUG_MAJOR_IMPACT, "transitionColor current", current.red, current.green, current.blue, current.white);
    showColor(current, 1);
  }
  showColor(finish, 1); // in case it rounds off below the finish color

}

///////////////////////////////////////////////////////////////////////////////////

void fade_in (color_type color_val, int duration) {
  transitionColor(blank, color_val, duration);
}

///////////////////////////////////////////////////////////////////////////////////

void fade_out (color_type color_val, int duration) {
  transitionColor(color_val, blank, duration);
}

///////////////////////////////////////////////////////////////////////////////////

void heartbeat (color_type color_val, int heartrate) {

  unsigned long milliseconds_between_beats;
  unsigned long heartbeat_milliseconds;
  unsigned long total_heartbeat_milliseconds;
  
  heartbeat_milliseconds = HEARTBEAT_MS + random(-RANDOM_RANGE, RANDOM_RANGE);
  total_heartbeat_milliseconds = heartrate*heartbeat_milliseconds*5; // based on 5x heartbeat_milliseconds used in fade_in, fade_out sequence below
  if(total_heartbeat_milliseconds > MS_PER_MIN){
    total_heartbeat_milliseconds = MS_PER_MIN -random(0, RANDOM_RANGE);
  }
  
  milliseconds_between_beats = (MS_PER_MIN-total_heartbeat_milliseconds)/heartrate;
  int_debug(DEBUG_MEDIUM_IMPACT, "heartbeat", heartbeat_milliseconds, heartrate, total_heartbeat_milliseconds, milliseconds_between_beats);
  flipNanoLED();
  fade_in(color_val, heartbeat_milliseconds*1.5);
  fade_out(color_val, heartbeat_milliseconds);
  flipNanoLED();
  delay(heartbeat_milliseconds/2);
  flipNanoLED();
  fade_in(color_val, heartbeat_milliseconds*1.5);
  fade_out(color_val, heartbeat_milliseconds);
  flipNanoLED();
  delay(milliseconds_between_beats);
}


///////////////////////////////////////////////////////////////////////////////////
//
// strobe() -- 
//              flash_time is the duration, in milliseconds, of each flash
//              frequency is flashes / minute
//              duration is total # seconds

void strobe (color_type color, int flash_time, int frequency, int duration) {

int loop_delay;
float loops_per_second;
int loops;

  loop_delay = (MS_PER_MIN-(flash_time * frequency)) / frequency; 
  loops_per_second = (float) frequency / SECS_PER_MIN;
  loops = (int) (loops_per_second * (float) duration);
  int_debug(DEBUG_MEDIUM_IMPACT, "Strobe  ",loop_delay, frequency, duration, loops);
  
  for (int i = 0; i < loops; i++){
    flipNanoLED();
    showColor(color, flash_time);
    showColor(blank, loop_delay);
    flipNanoLED();
  }

}


/////////////////////////randomColor//////////////////////////////////////////////////
// Speed is the interval, in seconds, of the initial color change.
// chaos - is a percentage factor used to randomize the interval defined by 'speed'
// duration -- lengthy in seconds to run (approximate)

void randomColor(int change_speed, int chaos, int duration){

  color_type color1;
  color_type color2;
  int next_interval;
  int   variability_range;
  int random_val = 0;

  color1 = randomize_color();
  variability_range = (int)((float) change_speed * (float)((float)chaos / 100.0));
  next_interval = change_speed; 
  int_debug(DEBUG_MEDIUM_IMPACT, "Random_Color", chaos, variability_range, next_interval, duration); 

  while (duration >= 0) {
    color2 = randomize_color();
    flipNanoLED();
    transitionColor (color1, color2, next_interval*MS_PER_SEC);
    flipNanoLED();
    duration = duration - next_interval;
    random_val = random((-variability_range), (variability_range));
    next_interval = change_speed + random_val;
    color1 = color2;
    int_debug(DEBUG_MEDIUM_IMPACT, "Random_Color", variability_range, random_val, next_interval, duration); 
  }
  
}

///////////////////////////////////////////////////////////////////////////////////
color_type randomize_color(){

color_type color;
  color.red = random(0,255);  
  color.green = random(0,255);  
  color.blue = random(0,255);
  color.white = 0;
  
  return color; 
  
}
///////////////////////////////////////////////////////////////////////////////////

void whiteWrite (int value) {
  analogWrite(WhiteLEDs, value);
}

///////////////////////////////////////////////////////////////////////////////////

void greenWrite (int val) {
  analogWrite(GreenLEDs, val);
}

///////////////////////////////////////////////////////////////////////////////////

void redWrite (int value) {
  analogWrite(RedLEDs, value);
}
///////////////////////////////////////////////////////////////////////////////////

void blueWrite (int value) {
  analogWrite(BlueLEDs, value);
}

///////////////////////////////////////////////////////////////////////////////////



void flipNanoLED() {
  if (NanoLEDState == OFF) {
    digitalWrite(NanoLED, ON);
    NanoLEDState = ON;
  } else {
    digitalWrite(NanoLED, OFF);
    NanoLEDState = OFF;
  }
}
///////////////////////////////////////////////////////////////////////////////////

void int_debug(int level, char * str, int val1, int val2, int val3, int val4) {

 
  if (level <= DEBUG_LEVEL) {
    Serial.print(str);
    Serial.print("    ");
    if (val1 != NIL) {
      Serial.print(val1);
      Serial.print("    ");
    } else {
      Serial.print("--");
      Serial.print("    ");
    }
    if (val2 != NIL) {
      Serial.print(val2);
      Serial.print("    ");
    } else {
      Serial.print("--");
      Serial.print("    ");
    }
    if (val3 != NIL) {
      Serial.print(val3);
      Serial.print("    ");
    } else {
      Serial.print("--");
      Serial.print("    ");
    }
    if (val4 != NIL) {
      Serial.print(val4);
      Serial.print("    ");
    } else {
      Serial.print("--");
    }
    Serial.println("\n");
  }
  //  delay(10);
}

///////////////////////////////////////////////////////////////////////////////////

void float_debug(int level, char * str, float val1, float val2, float val3, float val4) {

  if (level <= DEBUG_LEVEL) {
    Serial.print(str);
    Serial.print("    ");
    if (val1 != NIL) {
      Serial.print(val1);
      Serial.print("    ");
    } else {
      Serial.print("--");
      Serial.print("    ");
    }
    if (val2 != NIL) {
      Serial.print(val2);
      Serial.print("    ");
    } else {
      Serial.print("--");
      Serial.print("    ");
    }
    if (val3 != NIL) {
      Serial.print(val3);
      Serial.print("    ");
    } else {
      Serial.print("--");
      Serial.print("    ");
    }
    if (val4 != NIL) {
      Serial.print(val4);
      Serial.print("    ");
    } else {
      Serial.print("--");
    }
    Serial.println("\n");
  }
  // delay(10);
}



