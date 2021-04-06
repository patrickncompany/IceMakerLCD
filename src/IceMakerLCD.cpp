#include <Arduino.h>

void updatePage();  // forward declares
void updateOption();  // forward declares
void showSelect();  // forward declares
void updateTitle(String);  // forward declares
void checkTime();  // forward declares
void goHome();  // forward declares
void showTopTitle();  // forward declares
void showMenuTitle();  // forward declares
void refreshDisplay();  // forward declares

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

/***************** Menu Start ***********/
const int numPages=4;  //must be a const to use as array index
const int numOptions=4; //must be a const to use as array index
String page[numPages]={"Main Menu","Run Time","Timer","Status"};
String menu[numPages][numOptions]={{"Run Time","Timer","Status","*****"},{"3 seconds","4 seconds","5 seconds","Back Home"},{"2 hours","+1 hour","-1 hour","Back Home"},{"Run Now","Next Run","Last Now","Back Home"}};
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
String selectedOption; //global string for clicked option
/***************** Menu End ***********/

/***************** Timer Start ********************/
#include <elapsedMillis.h>
elapsedMillis sinceStart;
int sinceFlag=0;
unsigned long runTime=1000*60*10;// multiply ms*s*m (1000ms*60s*1m= 1 minute in milliseconds)
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

  // show lcd library splash
  display.display();
  delay(1000);
  display.clearDisplay();  

}

void loop() {
  //******************  Rotary Listen Loop Start ******************//
  r.loop();
  b.loop();
  //******************  Rotary Listen Loop End ******************//

  checkTime();
}

void checkTime(){
  if (sinceStart>runTime && sinceFlag){
    // timed event occurs.
    Serial.println("Time Elapsed");
    // reset elapsed time.
    sinceStart=0;
    sinceFlag=0;
  }
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
  // top 24 pixels (yellow)
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(currentTopTitle);
}

void showMenuTitle(){
  // first 12 pixels after top yellow bar
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,24);
  display.println(currentMenuTitle);
}

void refreshDisplay(){
  display.clearDisplay();
  showTopTitle();
  showMenuTitle();
  display.display();   
}

// single click
void click(Button2& btn) { 
  selOption = curpos;
  selectedOption = menu[curPage][selOption];
  switch(curPage){
    // case for each page
    case 0:
      switch(selOption){
        // case for each option
        case 0:
          if(firstRun){refreshDisplay();break;} //consume boot click event
          curPage=1; // new page
          curOption = minpos;  //option array index 
          curpos = minpos; //rotary position
          currentTopTitle = page[curPage];
          currentMenuTitle = menu[curPage][curOption];
          refreshDisplay();
          break;
        case 1:
          curPage=2; // new page
          curOption = minpos;  //option array index 
          curpos = minpos; //rotary position
          currentTopTitle = page[curPage];
          currentMenuTitle = menu[curPage][curOption];
          refreshDisplay();
          break;
        case 2:
          curPage=3; // new page
          curOption = minpos;  //option array index 
          curpos = minpos; //rotary position
          currentTopTitle = page[curPage];
          currentMenuTitle = menu[curPage][curOption];
          refreshDisplay();
          break;
        case 3:
          break; 
      }
      Serial.print("Selected : ");Serial.println(selectedOption);
      break;
    case 1:
      switch(selOption){
        // case for each option
        case 0:
          // 3 seconds
          //sinceFlag=1;
          //sinceStart=0;
          runTime=1000*2;
          goHome();
          break;
        case 1:
         // 4 seconds
          //sinceFlag=1;
          //sinceStart=0;
          runTime=1000*3;
          goHome();       
          break;
        case 2:
         //5 seconds
          //sinceFlag=1;
          //sinceStart=0;
          runTime=1000*4;
          goHome();
          break;
        case 3:
          goHome();        
          break; 
      }
      Serial.print("Selected : ");Serial.println(selectedOption);   
      break;
    case 2:
      switch(selOption){
        // case for each option
        case 0:
          goHome();
          break;
        case 1:
          goHome();
          break;
        case 2:
          goHome();
          break;
        case 3:
          goHome();       
          break; 
      }
      Serial.print("Selected : ");Serial.println(selectedOption);   
      break;
    case 3:
      switch(selOption){
        // case for each option
        case 0:
          // start timer
          sinceFlag=1;
          sinceStart=0;        
          break;
        case 1:
          goHome();
          break;
        case 2:
          goHome();
          break;
        case 3:
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