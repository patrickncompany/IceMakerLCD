#include <Arduino.h>

void updatePage();  // forward declares
void updateOption();  // forward declares
void showSelect();  // forward declares
void updateTitle(String);  // forward declares
void goHome();  // forward declares
void showTopTitle();  // forward declares
void showMenuTitle();  // forward declares
void showMenuInfo();
void refreshDisplay();  // forward declares

bool hb_callback(void *);
void wt_callback();
void rt_callback();

void updateTime();
void initTime();

/***************** Rotary Control Start ***********/
#include <Button2.h>
#include <ESPRotary.h>
#define ROTARY_PIN1 D5
#define ROTARY_PIN2 D6
#define BUTTON_PIN  D7
#define CLICKS_PER_STEP   4   // this number depends on your rotary encoder 
ESPRotary r = ESPRotary(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP);
Button2 b = Button2(BUTTON_PIN);
int curpos;
int minpos;
int maxpos;
void rotate(ESPRotary&);  // forward declares
void showDirection(ESPRotary&);  // forward declares
void click(Button2&);  // forward declares
// debounce mcu boot single click event. use as flag to ignore first click event on boot.
int firstRun=1;
/***************** Rotary Control End ***************/

/***************** LCD Start ***********/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
/***************** LCD End ***********/

/***************** Relay Start ***********/
#define RELAY_PIN D8
/***************** Relay End ***********/

/***************** Menu Start ***********/
const int numPages=4;  //must be a const to use as array index
const int numOptions=4; //must be a const to use as array index
String page[numPages]={"Main Menu","Run Time","Wait Time","Status"};
String menu[numPages][numOptions]={{"Run Time","Wait Time","Status","Quick Run"},{"3 seconds","4 seconds","5 seconds","Back Home"},{"2 hours","+1 hour","-1 hour","Back Home"},{"Remaing","Reset Time","*****","Back Home"}};
int maxPage = numPages-1; //max page INDEX
int minPage = 0;  //min page INDEX
int curPage = minPage; // current page INDEX
int prePage = 0;
int maxOption = numOptions-1; //max option INDEX
int minOption = 0;  //min option index
int curOption = minOption;  //current menu index
int preOption = 0;
int selOption = 0; //selected option INDEX (set on click)
String currentTopTitle = page[curPage]; //glogal string for top title
String currentMenuTitle = menu[curPage][curOption]; //glogal string for menu title
String currentMenuInfo = "wait time";
String selectedOption; //global string for clicked option
/***************** Menu End ***********/

/***************** Timer Start ********************/
#include <arduino-timer.h>
auto hbTimer = timer_create_default();
uint32_t hb_interval = 1000;
uint32_t wt_interval = 2*60*60*1000;
uint32_t rt_interval = 5*1000;

uint32_t milliseconds  = 1;
uint32_t seconds  = 1000 * milliseconds;
uint32_t minutes  = 60 * seconds;
uint32_t hours = 60 * minutes;

uint32_t eTime;
uint32_t tTime;
uint32_t dTime;
uint32_t dHours;
uint32_t dMinutes;
uint32_t dSeconds;
String sHours;
String sMinutes;
String sSeconds;
/***************** Timer End ********************/

void setup() {
  Serial.begin(115200);

  //******************  LCD Setup Start ******************//
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  //******************  LCD Setup END ******************// 

  //******************  Rotary Setup Start  ******************//
  r.setChangedHandler(rotate);
  r.setLeftRotationHandler(showDirection);
  r.setRightRotationHandler(showDirection);
  b.setTapHandler(click);
  minpos=0;
  maxpos=numOptions-1;
  curpos=0;
  //******************  Rotary Setup End  ******************//

  //******************  Relay Setup Start  ******************//
  pinMode(RELAY_PIN,OUTPUT);
  digitalWrite(RELAY_PIN,LOW);
  //******************  Relay Setup End  ******************//

  // show lcd library splash
  display.display();
  delay(1000);
  display.clearDisplay();


  // init timer values before starting heartbeat timer.
  // heartbeat timer callback updates display ever second.
  // heartbeat timer callback updates wait timer values.
  initTime();
  hbTimer.every(hb_interval,hb_callback);

}

void loop() {
  //******************  Rotary Listen Loop Start ******************//
  r.loop();
  b.loop();
  //******************  Rotary Listen Loop End ******************//  

  hbTimer.tick();

}

void initTime(){
  //eTime = millis();
  tTime = millis() + wt_interval;  // add wait millis to elapsed millis
}

void updateTime(){
  //eTime = millis();
  dTime = tTime - millis();
  dHours = dTime / hours;
  dMinutes = (dTime % hours) / minutes;
  dSeconds = ((dTime % hours) % minutes) / seconds;
  sHours = String(dHours);
  sMinutes = String(dMinutes);
  sSeconds = String(dSeconds);
  currentMenuInfo = sHours + ":" + sMinutes + ":" + sSeconds;
}

bool hb_callback(void *){
  Serial.println("heartbeat");
  updateTime();
    // if delta time (dTime) >= elapsed time (millis()) do something
    if (tTime<=millis()) {
      Serial.println("DING!");
      currentMenuInfo="Filling.";
      refreshDisplay();
      digitalWrite(RELAY_PIN,HIGH);
      delay(rt_interval);
      digitalWrite(RELAY_PIN,LOW);
      initTime();
      updateTime();
    }
  refreshDisplay();
  return true;
}

void wt_callback(){
  Serial.println("waittime");
  refreshDisplay();
}

void rt_callback(){
  Serial.println("runtime");
  refreshDisplay();
}

void goHome(){
  curPage=0; // new page
  curOption = minpos;  //option array index 
  curpos = minpos; //rotary position
  currentTopTitle = page[curPage];
  currentMenuTitle = menu[curPage][curOption];
  refreshDisplay();   
}

void showTopTitle(){
  // top display (Yellow ?21 pixels)
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(currentTopTitle);
}

void showMenuTitle(){
  // first line after top yellow bar
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,20);
  display.println(currentMenuTitle);
}

void showMenuInfo(){
  // second line after top yello bar
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,40);
  display.println(currentMenuInfo);
}

void refreshDisplay(){
  display.clearDisplay();
  showTopTitle();
  showMenuTitle();
  showMenuInfo();
  display.display();   
}

// single click
void click(Button2& btn) { 
  selOption = curpos;
  selectedOption = menu[curPage][selOption];
  switch(curPage){
    // case for each page
    case 0:
      // Main Menu
      switch(selOption){
        // case for each option
        case 0:
          // Go to Run Time
          if(firstRun){refreshDisplay();break;} //consume boot click event
          curPage=1; // new page
          curOption = minpos;  //option array index 
          curpos = minpos; //rotary position
          currentTopTitle = page[curPage];
          currentMenuTitle = menu[curPage][curOption];
          refreshDisplay();
          break;
        case 1:
          // Go to Wait Time
          curPage=2; // new page
          curOption = minpos;  //option array index 
          curpos = minpos; //rotary position
          currentTopTitle = page[curPage];
          currentMenuTitle = menu[curPage][curOption];
          refreshDisplay();
          break;
        case 2:
          // Go to Status
          curPage=3; // new page
          curOption = minpos;  //option array index 
          curpos = minpos; //rotary position
          currentTopTitle = page[curPage];
          currentMenuTitle = menu[curPage][curOption];
          refreshDisplay();
          break;
        case 3:
          // Quick Run
          goHome();
          break; 
      }
      Serial.print("Selected : ");Serial.println(selectedOption);
      break;
    case 1:
      // Run Time
      switch(selOption){
        // case for each option
        case 0:
          // 3 seconds
          //runTime=1000*3;
          goHome();
          break;
        case 1:
         // 4 seconds
          //runTime=1000*4;
          goHome();       
          break;
        case 2:
         //5 seconds
          //runTime=1000*5;
          goHome();
          break;
        case 3:
          // Go to Main Menu
          goHome();        
          break; 
      }
      Serial.print("Selected : ");Serial.println(selectedOption);   
      break;
    case 2:
      // Wait Time
      switch(selOption){
        // case for each option
        case 0:
          // 2 hours
          goHome();
          break;
        case 1:
          // +1 hour
          goHome();
          break;
        case 2:
          // -1 hour
          goHome();
          break;
        case 3:
          // Go to Main Menu
          goHome();       
          break; 
      }
      Serial.print("Selected : ");Serial.println(selectedOption);   
      break;
    case 3:
      // Status
      switch(selOption){
        // case for each option
        case 0:
          // Show time until next run
          break;
        case 1:
          // Reset timer to max time
          currentMenuInfo = "0000000000";
          initTime();
          updateTime();
          goHome();
          break;
        case 2:
          // not assigned
          goHome();
          break;
        case 3:
          // Go to Main Menu
          goHome();        
          break; 
      }
      Serial.print("Selected : ");Serial.println(selectedOption); 
      break;
  }

}

// left/right rotation
void showDirection(ESPRotary& r) {
  //Serial.println(r.directionToString(r.getDirection()));
  if (r.directionToString(r.getDirection())=="RIGHT"){
    curpos++;
  } else {
    curpos--;
  }

  if (curpos>maxpos){
    curpos=minpos; // wrap to start
  } else if (curpos<minpos){
    curpos=maxpos; // wrap to end
  }

  curOption = curpos; //current option index
  currentTopTitle = page[curPage];
  currentMenuTitle = menu[curPage][curOption];

  refreshDisplay(); 
  
  if (firstRun){firstRun=0;} // consume first click event.
  Serial.print("Menu Item : ");Serial.println(curpos);
}

// all rotation
void rotate(ESPRotary& r) {
   //Serial.println(r.getPosition());
   /*
    * with no call to r.getPosition() here
    * intermitent drop out of rotary polling
    * after click or change direction.
    * 
    */
}