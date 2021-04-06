# LCDMenu
## This code is Arduino code renamed and refactored to work with PlatformIO.  
## ArduinoIDE is designed to be easy to use for a beginner. It handles a few hurdles for you on configuration, syntax and structure and just compiles. Now we have to clean all that sloppy code up and configure stuff ourselves.
----
### **1. Add arduino header at very top.**  

#include <Arduino.h> 

### **2. Reformat any Arduino includes.**  

Use format **#include <Arduino.h>**  

Not format **#include "Arduino.h";**  

### **3. PlatformIO requires Forward-Declares.**  

All functions other than setup() and loop() require declares at top. Make a list of all functions and add to global declaration.   
void checkTime();  
void goHome();  
void showTopTitle();  
void showMenuTitle();  
void refreshDisplay();

### **4. Must add library with CLI manager.**  
   See [Setup Notes](#setupnotes)

### **5. Add library to C/C++ include path.**  
   See [Setup Notes](#setupnotes)  

.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
. .  
.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
. .  
.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
.  
. .  

### <a id="setupnotes"></a>Setup Notes
Had a bitch of a time looking in documents for include path.  
Turns out I was looking in PlatformIO settings and it is a C/C++ setting in VSCode.  
F1 > **C/C++: Edit configurations (UI)**  

To install library requires CLI:  

**pio lib search Button2**  
  
> Button2  
> \=======  
> #ID: 6059  
> Arduino/ESP library to simplify working with buttons. It allows you to use  
> callback functions to track single, double, triple and long clicks. It takes   
> care of debouncing. It will reduce and simplify your souce code significantly.  
> Tested with Arduino, ESP8266 and ESP32.  
> . . . etc . . .  
> Authors: Lennart Hennigs  

**Install to local project.**  
> pio lib install 6059
> .pio/libdeps/ . . . 

**Install global.**  
> pio lib -g install 6059
> C:\Users\ken\.platformio  

Once library is installed it must be added to the C/C++ Configuration.  
EX: C:/Users/ken/.platformio/lib/Button2/src  
Path must be to .h header file.



