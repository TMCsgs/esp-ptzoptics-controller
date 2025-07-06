#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Display: SCL/SCK => (GPIO) 22, SDA => (GPIO) 21 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// define pins for joystick one
int ONE_PIN_X = 12;
int ONE_PIN_Y = 13;
int ONE_PIN_BUTTON = 32;

// define pins for joystick two
int TWO_PIN_X = 34;
int TWO_PIN_Y = 35;
int TWO_PIN_BUTTON = 33;

// define pins for Camera LEDs
int PIN_CAM_ONE_LED = 25;
int PIN_CAM_TWO_LED = 26;

// define variables for read functions
int x;
int y;
int buttonState;
String command = "zoomstop";
String directionx = "";
String directiony = "";
int speedx;
int speedy;

// define Variables for Joystick
int MAX = 4095;
int MIN = 0;
int OFFSET = 0; // tolerance for zero

// zero points of joystick one
int ONE_ZERO_X = 1903;
int ONE_ZERO_Y = 1890;

// zero points of joystick two
int TWO_ZERO_X = 1904;
int TWO_ZERO_Y = 1891;

// struct to return Joystickdata
struct JoystickOut {
  int speedx;
  int speedy;
  String command;
  int buttonPressed;
};

// Variables to save Joystickdata
JoystickOut JoystickOne;
JoystickOut JoystickOneLast = {0, 0, "ptzstop", 0};
JoystickOut JoystickTwo;
JoystickOut JoystickTwoLast = {0, 0, "zoomstop", 0};

// Variables for speed function
float factor;
int returnspeed;

// Variables for url
String selectedcam = "1";
String IP = "192.168.0.1";
//String IP_CAM_ONE = "192.168.0.11";
//String IP_CAM_TWO = "192.168.0.12";
//String currentip = IP_CAM_ONE;
String returnurl;


void draw_Display() {
    display.clearDisplay();
    display.setTextSize(1);     
    display.setTextColor(SSD1306_WHITE); 
    display.setCursor(0, 0);
    display.println("  Joystickbelegung");
    display.println("--------------------");
    display.print("   ");
    display.setTextSize(2);
  if (ONE_PIN_X == 34) {
    display.println("Z <" + selectedcam + "> K"); 
  } else {
    display.println("K <" + selectedcam + "> Z"); 
  }
  display.display();
}

void setup() {
 // Serial.begin(9600);
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Joystick one setup pinmode
  pinMode(ONE_PIN_X, INPUT);
  pinMode(ONE_PIN_Y, INPUT);
  pinMode(ONE_PIN_BUTTON, INPUT_PULLUP);

  // Joystick two setup pinmode
  pinMode(TWO_PIN_X, INPUT);
  pinMode(TWO_PIN_Y, INPUT);
  pinMode(TWO_PIN_BUTTON, INPUT_PULLUP);

  // Camera LEDs setup pinmode
  pinMode(PIN_CAM_ONE_LED, OUTPUT);
  pinMode(PIN_CAM_TWO_LED, OUTPUT);

  // Set CAmera One LED to active
  digitalWrite(PIN_CAM_ONE_LED, HIGH);
  
  /*display.display();
  delay(2000);
  display.clearDisplay();
  delay(200);*/
  draw_Display();
  delay(3000);
  /*display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.println("  Joystickbelegung");
  display.println("--------------------");
  display.print("   ");
  display.setTextSize(2);
  display.println("Z <1> K"); 
  display.display();*/
}




int speed(int zero, int value, int steps) {
  if (value < zero) {
    factor = (zero - OFFSET) / steps;
    returnspeed = (zero - value) / factor;
  } else {
    factor = (MAX - zero + OFFSET) / steps;
    returnspeed = (value - zero) / factor;
  }
  return returnspeed;
}

//Get Joystickdata from Joystick one
JoystickOut read_one() {
  x = analogRead(ONE_PIN_X);
  y = analogRead(ONE_PIN_Y);
  buttonState = digitalRead(ONE_PIN_BUTTON);

  speedy = speed(ONE_ZERO_Y, y, 10 + 1);
  speedx = speed(ONE_ZERO_X, x, 15 + 1);

  if (speedy == 0 && speedx == 0) {
    return {0, 0, "ptzstop", 1 - buttonState};
  }   

  if (speedx == 0) {
    directionx = "";
    speedx = 1;
  } else if (x < (ONE_ZERO_X - OFFSET)) {
    directionx = "left";
  } else if (x > (ONE_ZERO_X + OFFSET)) {
    directionx = "right";
  }

  if (speedy == 0) {
      directiony = "";
      speedy = 1;
  } else if (y < (ONE_ZERO_Y - OFFSET)) {
    directiony = "up";
  } else if (y > (ONE_ZERO_Y + OFFSET)) {
    directiony = "down";
  }

  return {speedx - 1, speedy -1, directionx + directiony, 1 - buttonState};
}


//Get Joystickdata from Joystick two
JoystickOut read_two() {
  //x = analogRead(TWO_PIN_X);
  y = analogRead(TWO_PIN_Y);
  buttonState = digitalRead(TWO_PIN_BUTTON);
  
  //speedx = speed(TWO_ZERO_X, x, 20 + 1);
  speedy = speed(TWO_ZERO_Y, y, 7 + 1);

  if (speedy == 0) {
    return {0, 0, "zoomstop", 1 - buttonState};
  }   

  if (speedx == 0) {
    command = "";
  } else if (x < (TWO_ZERO_X - OFFSET)) {
    command = "left";
  } else if (x > (TWO_ZERO_X + OFFSET)) {
    command = "right";
  }

  if (speedy == 0) {
    command = "zoomstop";
  } else if (y < (TWO_ZERO_Y - OFFSET)) {
    command = "zoomin";
  } else if (y > (TWO_ZERO_Y + OFFSET)) {
    command = "zoomout";
  }

  return {speedx - 1, speedy - 1, command, 1 - buttonState};
}



// create & send url to Serial output
void output_url(JoystickOut JoystickOutput, bool Zoom = false) {
  if (Zoom) {
    //"http://{camera ip}/cgi-bin/ptzctrl.cgi?ptzcmd&{command}&{speedy}";
    returnurl = "http://" + IP + selectedcam + "/cgi-bin/ptzctrl.cgi?ptzcmd&" + JoystickOutput.command + "&" + JoystickOutput.speedy;
  } else {
    //"http://{currentip}/cgi-bin/ptzctrl.cgi?ptzcmd&{command}&{speedx}&{speedy}";
    returnurl = "http://" + IP + selectedcam + "/cgi-bin/ptzctrl.cgi?ptzcmd&" + JoystickOutput.command + "&" + JoystickOutput.speedx + "&" + JoystickOutput.speedy;
  }

  Serial.println(returnurl);
}

void switchJoysticks() {
  if (ONE_PIN_X == 12) {
    ONE_PIN_X = 34;
    ONE_PIN_Y = 35;

    TWO_PIN_X = 12;
    TWO_PIN_Y = 13;

    draw_Display();
  } else {
    ONE_PIN_X = 12;
    ONE_PIN_Y = 13;

    TWO_PIN_X = 34;
    TWO_PIN_Y = 35;

    draw_Display();
  }
  return;
}



void loop() {
  // read Joystick outputs
  JoystickOne = read_one();
  JoystickTwo = read_two();

  // toggle Camera LEDs
  if (!(JoystickOne.buttonPressed == JoystickOneLast.buttonPressed)) {
    if (JoystickOne.buttonPressed == 1) {
      if (selectedcam == "1") {
        selectedcam = "2";
        digitalWrite(PIN_CAM_ONE_LED, LOW);
        digitalWrite(PIN_CAM_TWO_LED, HIGH);
        draw_Display();
      } else {
        selectedcam = "1";
        digitalWrite(PIN_CAM_ONE_LED, HIGH);
        digitalWrite(PIN_CAM_TWO_LED, LOW);
        draw_Display();
      }
    }

    JoystickOneLast.buttonPressed = JoystickOne.buttonPressed;
  }

  if (!(JoystickOne.speedx == JoystickOneLast.speedx && JoystickOne.speedy == JoystickOneLast.speedy && JoystickOne.command == JoystickOneLast.command)) {
    output_url(JoystickOne);
    JoystickOneLast = JoystickOne;
  }

  if (!(JoystickTwo.buttonPressed == JoystickTwoLast.buttonPressed)) {
    if (JoystickTwo.buttonPressed == 1) {
      switchJoysticks();
    }
    JoystickTwoLast.buttonPressed = JoystickTwo.buttonPressed;
  }

  if (!(JoystickTwo.speedy == JoystickTwoLast.speedy && JoystickTwo.command == JoystickTwoLast.command)) {
    // true means that the url for zooming is used
    output_url(JoystickTwo, true);
    JoystickTwoLast = JoystickTwo;
  }

  delay(200);
}
