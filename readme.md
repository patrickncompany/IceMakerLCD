# IceMakerLCD
Create a timing circuit for ice maker using node mcu 8266 w/ ESP-12, rotary encoder for input and a 0.96" I2C IIC 12864 128X64 Pixel OLED LCD for display.  

>### ***Terms for instructions differ from those used in code and need to be refactored***    
> Run Time (code) or Fill Cycle (operation) refer to the time the relay is closed and water is filling the tray.  
> Wait Time (code) or Wait Interval (operation) refer to the count down timer between fill cycles.  
## 1. Menu  
  - Main Menu  
    List of all sub menus.
    - Run Time (5s) : Select how long the relay triggers solenoid and fills tray with water. Fill cycle duration.
    - Wait Time (2h) : Select how interval between run times. Run a fill cycle this often. 
    - Status : Mostly depracated. Still used to reset current wait time.
    - Quick Run : Trigger fill cycle based on current run time. Reset wait time countdown.
  - Run Time Menu 
    Due to timing issues the run time value is always set to one second less than actual run time.
    - 3s : Fill cycle of 3 seconds. Run time value of 2000ms.  
    - 4s : Fill cycle of 4 seconds. Run time value of 3000ms.  
    - 5s : Fill cycle of 5 seconds. Run time value of 4000ms.  
    - Back Home  
  - Wait Time Menu  
    Default wait time is 2 hours. Minimum wait time is 15 minutes.  
    - 2h : Set to 2 hour wait time. Fill cycle every 2 hours.  
    - +15m : Add 15 minutes to wait time and reset.  
    - -15m : Subtract 15 minutes from wait time and reset.  
    - Back Home 
  - Status Menu  
    - Remaing : Deprecated.  
    - Reset Time : Reset count down to next fill cycle to max wait time.
    - ***** : Place holder.  
    - Back Home  
  
## 2. Timing  
  - All time values are in milliseconds.  
  - An inifite non locking timer is run with 1 second heartbeat tick using <arduino-timer.h> library.
  - All other timing is done with millis() elapsed milliseconds and a target time.
  - Wait time between run times is handled by setting a target time (fixed) by adding the amount of time to wait to the current millis(). By calculating delta between target time and the elapsing millis() a count down clock can be displayed. Once millis() is larger than the target the fill cycle can be run.  
  - Run time is a simple locking timer using delay().  

