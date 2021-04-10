#include <Arduino.h>

/***************** Timer Start ********************/
#include <arduino-timer.h>
auto hbTimer = timer_create_default();
uint32_t hb_interval = 1000;
uint32_t wt_interval = 2*60*60*1000;
uint32_t rt_interval = 4*1000; // set interval to 1s less for accuracy.

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
String hmsTime;

/***************** Timer End ********************/

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
void rotate(ESPRotary&);  
void showDirection(ESPRotary&);  
void click(Button2&);  
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
bool manualSTOP = false;
/***************** Relay End ***********/

/***************** Menu Start ***********/
const int numPages=4;  //must be a const to use as array index
const int numOptions=8; //must be a const to use as array index
String page[numPages]={"Main Menu","Run Menu","Wait Menu","Status"};
String menu[numPages][numOptions]={
  {"Run Time","Wait Time","Status","Quick Run","!! STOP !!","Restart","",""},
  {"3s","4s","5s","Back","","","",""},
  {"2h","+15m","-15m","Back","","","",""},
  {"Reset Wait","*****","*****","Back","","","",""}
  };

int maxPage = numPages-1; // max page INDEX
int minPage = 0;  // min page INDEX
int curPage = minPage; // current page INDEX
int prePage = 0;
int maxOption = numOptions-1; // max option INDEX
int minOption = 0;  // min option index
int curOption = minOption;  // current menu index
int preOption = 0;
int selOption = 0; // selected option INDEX (set on click)
String currentTopTitle = page[curPage]; // glogal string for top title
String currentMenuTitle = menu[curPage][curOption]; // glogal string for menu title
String currentMenuInfo = "wait time";
String selectedOption; // global string for clicked option
/***************** Menu End ***********/

void updatePage();  
void updateOption();  
void showSelect();  
void updateTitle(String);  
void goHome();  
void showTopTitle();  
void showMenuTitle();  
void showMenuInfo();
void refreshDisplay();  

bool hb_callback(void *);
void wt_callback();
void rt_callback();

void updateTime();
void initTime();
void relayRun();
void relayStop();

void goMenu(int);

String mstohms(int32_t);

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

  //debug output
  Serial.println(menu[curPage][curOption]);
}

void loop() {
  //******************  Rotary Listen Loop Start ******************//
  r.loop();
  b.loop();
  //******************  Rotary Listen Loop End ******************//  

  hbTimer.tick();

}

void relayStop(){
  digitalWrite(RELAY_PIN,LOW);
  manualSTOP = true;
  currentMenuInfo="STOPPED";
  currentMenuTitle="Run = 0s";
  currentMenuInfo="00:00:00";
  refreshDisplay();
  wt_interval = 3*24*60*60*1000; // three days
  rt_interval = 0;
}

void relayRun(){
  int rInterval = (rt_interval/1000)+1; // run value is always 1s less than real world
  String rNotice = String(rInterval);
  rNotice = "Fill: " + rNotice + "s";
  Serial.print(mstohms(millis()));Serial.print(" - Start ");Serial.println(rNotice);
  currentMenuInfo=rNotice;
  refreshDisplay();
  digitalWrite(RELAY_PIN,HIGH);
  delay(rt_interval);
  digitalWrite(RELAY_PIN,LOW);
  Serial.print(mstohms(millis()));Serial.print(" - End ");Serial.println(rNotice);
  initTime();
  updateTime();
}

void initTime(){
  //eTime = millis();
  tTime = millis() + wt_interval;  // add wait millis to elapsed millis
}

String mstohms(int32_t msvalue){
  dTime = msvalue;
  dHours = dTime / hours;
  dMinutes = (dTime % hours) / minutes;
  dSeconds = ((dTime % hours) % minutes) / seconds;
  sHours = String(dHours);
  sMinutes = String(dMinutes);
  sSeconds = String(dSeconds);
  if (sHours.length()<2) sHours = "0" + sHours;
  if (sMinutes.length()<2) sMinutes = "0" + sMinutes;
  if (sSeconds.length()<2) sSeconds = "0" + sSeconds;
  hmsTime = sHours + ":" + sMinutes + ":" + sSeconds;
  return hmsTime;
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
  if (sHours.length()<2) sHours = "0" + sHours;
  if (sMinutes.length()<2) sMinutes = "0" + sMinutes;
  if (sSeconds.length()<2) sSeconds = "0" + sSeconds;  
  currentMenuInfo = sHours + ":" + sMinutes + ":" + sSeconds;
}

bool hb_callback(void *){
  //Serial.println("heartbeat");
  if (manualSTOP) {
    //Serial.println("manualSTOP");    
  } else {
    updateTime();
    // if wait time elapsed do something
    if (tTime<=millis()) { relayRun(); }
    refreshDisplay();
  }

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

void goMenu(int cp){
  curPage=cp; // new page
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
          currentMenuInfo="Run Now";
          refreshDisplay();
          delay(5000);
          relayRun();
          goHome();
          break;
        case 4:
          // Stop All
          relayStop();
          break; 
        case 5:
          // restart
          manualSTOP = false;
          wt_interval = 2*60*60*1000;
          rt_interval = 4*1000;
          initTime();  // prevents instant relay run bug.
          goHome();
          break; 
      }
      //Serial.print("Selected : ");Serial.println(selectedOption);
      break;
    case 1:
      // Run Time
      switch(selOption){
        // case for each option
        case 0:
          // 3 seconds 
          rt_interval=1000*2;// set interval to 1 less due to overhead
          currentMenuInfo = "Run 3s";
          refreshDisplay();          
          delay(500); //locking
          goHome();
          break;
        case 1:
         // 4 seconds
          rt_interval=1000*3;// set interval to 1 less due to overhead
          currentMenuInfo = "Run 4s";
          refreshDisplay();          
          delay(500); //locking
          goHome();       
          break;
        case 2:
         //5 seconds
          rt_interval=1000*4;// set interval to 1 less due to overhead
          currentMenuInfo = "Run 5s";
          refreshDisplay();
          delay(500); //locking
          goHome();
          break;
        case 3:
          // Go to Main Menu
          goHome();        
          break; 
      }
      //Serial.print("Selected : ");Serial.println(selectedOption);   
      break;
    case 2:
      // Wait Time
      switch(selOption){
        // case for each option
        case 0:
          // 2 hours
          wt_interval = 2*60*60*1000;
          currentMenuInfo = "Set 2h";
          refreshDisplay();
          delay(500); //locking
          initTime();
          updateTime();
          goHome();
          break;
        case 1:
          // +15 min
          wt_interval = wt_interval + (15*60*1000);
          currentMenuInfo = "Plus 15m";
          refreshDisplay();
          delay(500); //locking
          initTime();
          updateTime();
          //goHome();
          break;
        case 2:
          // -15 min
          wt_interval = wt_interval - (15*60*1000);
          if (wt_interval<=0) wt_interval=(1*60*60*1000);
          currentMenuInfo = "Minus 15m";
          refreshDisplay();
          delay(500); //locking
          initTime();
          updateTime();
          //goHome();
          break;
        case 3:
          // Go to Main Menu
          goHome();       
          break; 
      }
      //Serial.print("Selected : ");Serial.println(selectedOption);   
      break;
    case 3:
      // Status
      switch(selOption){
        // case for each option
        case 0:
          // Reset timer to max time
          currentMenuInfo = "0000000000";
          initTime();
          updateTime();
          goHome();
          break;
        case 1:
          // not assigned
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
      //Serial.print("Selected : ");Serial.println(selectedOption); 
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
  
  // limit menu position with min/max; filter blanks

  // maxpos is always upper index of array
  // NOT 
  // index of last valid option in array.

  // IE: {"option1","option2","",""}
  // Above menu has 2 options(index 0,1). maxpos = max index = 3.
  // maxpos references invalid option. can NOT use.  

  // Full menu array {"option1","option2","option3","option4"}
  // Above menu has 4 options(index 0,1,2,3). maxpos = max index = 3.
  // maxpos references valid option. can use.

  if (curpos>maxpos){
    // >maxpos only true for full array
    curpos=maxpos;  // maxpos only valid for full array
  } else if (curpos<minpos){
    // All menus have index 0 (minpos)
    curpos=minpos;
  } else if (menu[curPage][curpos]==""){
    // empty option decrement pos
    curpos--;
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