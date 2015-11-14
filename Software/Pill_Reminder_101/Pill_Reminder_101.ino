
/*************************************** PILL REMINDER **************************************/
//
//    The PILL REMINDER helps a user to remember to take their pills at the correct times of the
//    day.  The device has two, 7-day sets of pill dispensers: one for morning (AM) medications
//    and one for evening (PM) medications.  There is an LED just above each pill dispenser.
//    If the current time is within a time wiondow for the user to take their AM or PM pills, 
//    the LED above the proper dispenser will light.  Otherwise, no LEDs light (helping the user to
//    avoid taking a missed dose too close to the next dose).  
//
//    The device does not sense if the user has actually opened the dispenser and removed the pills.  
//    Therefore, the proper LED lights throughout a "proper" pill taking time interval.  These
//    "proper pill taking time intervals" are specified in defined constants for LED on times and
//    LED off times for the AM pills and the PM pills. The constants and their defaults are:
//
//      - AM_ON_TIME: default is 6:00 AM
//      - AM_OFF_TIME: default is 1159 AM (noon)
//      - PM_ON_TIME: default is 6:00 PM
//      - PM_OFF_TIME: default is 11:59 PM (midnight)
//
//    These defined constants must be changed, and the program re-compiled and uploaded to the
//    device, if it is desired to change the AM and PM pill taking intervals.
//
//    Although the Pill Reminder does not sense the user opening a dispenser and removing the pills,
//    it is provided with an infrared proximity sensor (PIR) device.  Whenever the PIR is triggered
//    for the first time in any pill taking interval, the appropriate lit LED will blink for 
//    approximately 4 seconds, as an extra aide in gaining the user's attention.
//
//    A user with short term memory loss or mild dementia must be taught to open the dispenser
//    and take the pills therein only whenever an LED is lit, regardless of whether it is lit blinking
//    or solid.  If the dispenser under the lit LED is empty, the user has already taken the pill,
//    even if they do not remember doing so.  Caregivers must train users to only take pills in the
//    dispenser under the LED that is lit (if any) and not take any other pills, regardless of their
//    memory of pill taking.
//
//    The Pill Reminder contains a battery backed realtime clock module that maintains the date and time
//    even if the power is off.  The battery is rated to last at least 5 years.  If the battery is 
//    removed or fails, the user (or caregiver) is forced to manually set the current date and
//    time and the unit will not operate until this is done.  This feature prevents users from
//    taking pills at incorrect times due to a realtime click battery failure or other hardware problem.
//    
//    The device contains a 2 line, 16 character per line LCD that displays the current date, day of the
//    week and current time.  In addition, this display is used to promt a cargiver when setting the date
//    and the time.  The hardware and software automatically adjusts for daylight savings time (if so set)
//    and corrects for leap year; therefore, time setting should be a very infrequent function.
//
//    Normally, there are no controls for the user to operate (and therefore none for the user to mis-set).
//    Time setting requires opening the enclosure cover behind the display panel and depressing buttons
//    on the printed circuit board inside the unit, while following prompts on the LCD.  The time may only
//    be set when the device is first powered on or the "reset" button inside the unit is depressed.  Either
//    way that the device is reset, it starts off with a (nominally) 10 second period for the PIR to 
//    calibrate.  During this period, the LEDs will light in succession though approximately 4 cycles as a
//    self-test.  If, at any time during this calibration, the user presses the internal "select" button,
//    the LEDs will stop lighting and the unit will enter time setting mode.  In time setting mode, the user
//    advances through a menu using "select" and changes values for the displayed menu item using "increment".
//    Since time setting is an infrequent activity, only two buttons are provided and the user must step
//    through all menu items in order to set the device's internal time.  The button functions "wrap around"  
//    and the user may cycle through the prompts as many times as they wish, in order to check their settings.  
//    The device exits the time setting mode and re-enters the calibration mode only when the user presses
//    the increment button in response to the prompt to do so to set the date and time.  At this point
//    the device completes any PIR calibration time remaining and then enters the normal user mode.  In
//    the normal user mode, the device operates automatically with no user controls or setting capability.
//    It is necessary (intentionally) to reset the device in order to make any changes to the date and time.
//
//    Note:  if the LED on and off time defaults are to be changed by the users, please note that 
//    at least one minute (minimum interval supported by the code) is required between AM_OFF_TIME 
//    and PM_ON_TIME, and also between PM_OFF_TIME and AM_ON_TIME.  This minimum time, when all LEDs are off, 
//    is needed to reset the PIR blink state machine back to the disarmed state, so that the PIR functionality 
//    can then be re-armed for triggering.
//	
//  version 1.01 (initial public release)
//  author: Bob Glicksman
//  date: 3/12/14
//
//  IMPORTANT NOTICE:  
//    The Pill Reminder is released under a Creative Commons Attribution-ShareAlike Open Source license.  
//    Use of this software, device hardware, and documentation is subject to the terms and conditions of this license 
//    and by using the hardware, software and/or documentation in any manner, you are agreeing to abide by all terms
//    and conditions of the license.  To view a copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/
//    or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
//    
//    The Pill Reminder is intended for hobbyist applications and for “do it yourself” assembly and use.  The user 
//    assumes all responsibility and liability for any damages, injuries or consequences resulting from the construction, 
//    implementation and use of this product or any part thereof. Complete terms and conditions of use can be found in the 
//    document "Terms_of_Use_License_and_Disclaimer" that is included in this release package.  This document can also
//    be found at http://www.shrimpware.com/pillreminder/TOU.html
//
//    Copyright © 2014 Shrimpware LLC. All rights reserved.


/****************************GLOBAL CONSTANTS AND VARIABLES ***********************************/

// INCLUDES (libraries)
#include <LiquidCrystal.h>      // library for LCD display
#include <Time.h>               // Arduino Time library
#include <Wire.h>               // Arduino I2C library
#include <DS1307RTC.h>          // Arduino library for DS1307 realtime clock
#include <Timezone.h>           // Arduino library for converting betweeen UTC and timezones, e.g. for DST
#include <EEPROM.h>             // Arduino library for writing to and reading from EEPROM

//  CONSTANTS

// debugging mode = advances time rapidly
// #define DEBUG    // comment this line out for normal operation

//    Patterns for lighting an LED
const byte LED_OFF = B00000000;
const byte SUN_AM =  B00000010;
const byte MON_AM =  B00000100;
const byte TUE_AM =  B00001000;
const byte WED_AM =  B00010000;
const byte THU_AM =  B00100000;
const byte FRI_AM =  B01000000;
const byte SAT_AM =  B10000000;

const byte SUN_PM =  B11111101;
const byte MON_PM =  B11111011;
const byte TUE_PM =  B11110111;
const byte WED_PM =  B11101111;
const byte THU_PM =  B11011111;
const byte FRI_PM =  B10111111;
const byte SAT_PM =  B01111111;

const unsigned long LED_TEST_DELAY = 150ul;    // time for LED to stay lighted in Calibrate self-test (150 ms)

//    PIR Calibration and self test time
const unsigned long PIR_CALIBRATE = 10000ul;  // 10 seconds (10,000 ms) per PIR spec sheet.

//    Time updating interval
const unsigned long TIME_UPDATE_INTERVAL = 250ul; // update the time display every 250 ms

//    Definition of Arduino pins used
//      Arduino pins used for LED shift register
const int latchPin = 12;
const int shiftPin = 11;
const int dataPin  = 13;

//      Arduino pins used for LCD display
const int RS     = 4;
const int ENABLE = 5;
const int D4     = 6;
const int D5     = 7;
const int D6     = 8;
const int D7     = 9;

//      Arduino pins used for push button switches
const int SELECT_PIN    = 3;
const int INCREMENT_PIN  = 2;

//      Arduino pin for PIR
const int pirPin    = 10;

//    Push button constants
const byte SELECT_ID = 0;         // SELECT is button number 0
const byte INCREMENT_ID = 1;      // INCREMENT is button number 1
const unsigned long DEBOUNCE_TIME = 20ul;  // 20 ms to debounce buttons.

//      LED lighting time periods (24 hour time for unambiguity)
const int AM_ON_TIME =   600;     // 6 am
const int AM_OFF_TIME =  1159;    // one minute before noon
const int PM_ON_TIME =   1800;    // 6 pm
const int PM_OFF_TIME =  2359;    // one minute before midnight 
const byte NUM_BLINKS = 16;    // blink selected LED for this number of quarter-seconds (16 = 4 seconds blink)

//      define states for PIR blink function
const byte PIR_DISARM    = 0x00;    // PIR blink function is disarmed - ignore PIR activity
const byte PIR_READY     = 0x01;    // PIR blink function is armed and ready to trigger LED blinking
const byte PIR_BLINKING  = 0x02;    // PIR has been triggered and is in the process of LED blinking
const byte PIR_COMPLETE  = 0x03;    // PIR blinking has completed and won't trigger again until next time cycle

//      Daylight savings time and timezone indications
const byte DST           = 0;      // Daylight savings time is to be observed and automated
const byte noDST         = 1;      //  Daylight savings time is not observed and only standard time is kept
const byte TZ_INDEX_MAX  = 1;      // Max - 1 number of entries in time zone possibilties -- currently 2 (DST, noDST)
const int TZ_ADDR        = 0;      // EEPROM address where user selected byte timeZoneIndex will be in non-volatile storage

//  GLOBAL VARIABLES

//    Arrays to index AM and PM LEDs
byte dayLightsAM[] = {SUN_AM, MON_AM, TUE_AM, WED_AM, THU_AM, FRI_AM, SAT_AM};
byte dayLightsPM[] = {SUN_PM, MON_PM, TUE_PM, WED_PM, THU_PM, FRI_PM, SAT_PM};

//    LCD display (lcd)
LiquidCrystal lcd(RS, ENABLE, D4, D5, D6, D7);

//    Rules for Daylight Savings Time using the Timezone library
TimeChangeRule dt = {"DT", Second, Sun, Mar, 2, 60};         // Daylight savings time is 60 minutes later than standard time
TimeChangeRule st = {"ST", First, Sun, Nov, 2, 0};           // Standard time, as set by the user
Timezone useDT(dt, st);                                      // Timezone object to use where daylight savings time is observed
Timezone useST(st, st);                                      // Timezone object to use where daylight savings time is not observed
     
//    The byte variable timeZoneIndex is used to determine if the user selected to observe daylight savings time
//    (timeZoneIndex = DST) or not to observe daylight savings time (timeZoneIndex = noDST).  Other values
//    may be added for regions of the world that observe different time zone change rules than the USA.
byte timeZoneIndex = DST;    // Default is to observe, and automate, daylight savings time conversions

//    Internal time
time_t currentTime;

/*********************END OF GLOBAL CONSTANTS AND VARIABLES ***********************************/

/**********************************BEGINNING OF setup()****************************************/
//  setup():  initialize Arduino pins for shift register control.  Turn off all LEDs,
//    set the time and check RTC and display "Calibrating ..." for 10 seconds 
//    (PIR_CALIBRATE) if RTC time is OK, or else print an RTC error message.  
//    During calibration, the LEDs flash in sequence as a self test.  At any
//    time during calibration, depressing the Select button will enter the date-time
//    setting mode in which the Select and Increment buttons are used to set the date
//    and time and store the new setting in the RTC module.  Upon completion of date-time
//    setting, PIR calibration and LED testing will be resumed for whatever duration of
//    the Calibration period remains.  After calibration, the software enters loop()
//    for normal operation of the Pill Reminder.

void setup()
{
  // initialize the hardware
  pinMode(SELECT_PIN, INPUT_PULLUP);
  pinMode(INCREMENT_PIN, INPUT_PULLUP);
  pinMode(pirPin, INPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(shiftPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  
  lightLed(LED_OFF);   // turn off all of the LEDs
  lcd.begin(16, 2);    // initialize the lcd display: 16 columns, 2 rows

  setSyncProvider(RTC.get);  // synchronize time to the DS1307 RTC chip
  // check to see if the time is synchronized to the RTC
  if(timeStatus() != timeSet)      // time is not synchronized
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("RTC time not set");
    lcd.setCursor(0,1);
    lcd.print("SEL to set time");
    
    // loop (forever) until the RTC time is set
    while(timeStatus() != timeSet)
    {
      if(readButton(SELECT_ID, SELECT_PIN) == true) // must press SELECT to enter setDateTime()
      {
        setDateTime();
      } 
    }
  }
  
  timeZoneIndex = EEPROM.read(TZ_ADDR);    // get the stored timeZoneIndex (DST or not) from EEPROM
  if(timeZoneIndex > TZ_INDEX_MAX)         // just in case EEPROM location was not initialized previously
  {
    timeZoneIndex = DST;                   // default to automatic DST
  }
  
  // time is synchronized, so proceed to calibrate the PIR for PIR_CALIBRATE amount of time.
  //
  //  Throughout the Calibrate loop, the LEDs are lit in sequence;  Sunday, AM, Sunday PM,
  //  Monday AM, etc. until PIR_CALIBRATE time has expired, at which time setup() is complete
  //  and loop() is entered for normal operation fo the Pill Reminder.
  //
  //  While in the Calibrate loop, the SELECT buttion may be pressed, in which case
  //  the code executes the setDateTime() function.  Upon returning from the setDateTime
  //  function, the Calibrate loop will complete its PIR_CALIBRATE time (if not already completed).
  //  setup() is then complete and loop() is entered for normal operation fo the Pill Reminder.
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CALIBRATING ...");
  unsigned long nowTime = millis();
  byte ledMode;
  byte weekDay;
  while ( diff(millis(), nowTime) < PIR_CALIBRATE)
  {
    weekDay = 0;
    ledMode = 0;
    while (weekDay < 7)    // run through 7 days of LED lighting
    {           
      //Check to see if SELECT is pressed and execute setDate() if it is
      if(readButton(SELECT_ID, SELECT_PIN) == true)
      {
        lightLed(LED_OFF);   // turn off all of the LEDs
        setDateTime();
      } 
      
      // self test the LEDs 
      switch(ledMode)
      {
        case 0:    // light an AM led
          lightLed(dayLightsAM[weekDay]);
          if (nbDelay(LED_TEST_DELAY) == false)
          {
            ledMode = 1;    // advance to PM leds after delay
          }
          break;
              
        case 1:    // light a PM led
          lightLed(dayLightsPM[weekDay]);
          if (nbDelay(LED_TEST_DELAY) == false)
          {
             ledMode = 2;    // advance to leds off after delay
          }
          break;            
            
        case 2:    // all leds off
          lightLed(LED_OFF);
          if (nbDelay(LED_TEST_DELAY) == false)
          {
            ledMode = 0;    // advance to AM leds after delay
            if (weekDay >=7)      // advance to next day
            {
              weekDay = 0;
            }
            else
            {
              weekDay++;
            }
          }
          break;     

         default:      // just in case :)
          ledMode = 0;  
                
        }  // end switch() statement
         
      } // end while() loop

  
  }  // loop for PIR_CALIBRATE time and then enter loop() to run
  currentTime = getCurrentTime(now());    // set the current time initially from the time library, converted by time zone
  
}  // end of setup()

/****************************************END OF setup()****************************************/

/****************************************BEGINNING OF loop()***********************************/
//  loop():  clears the LCD display and prints error/information message if RTC not OK.  Otherwise,
//    sets Arduino time to RTC time and displays the date and time on the LCD.  Determines if
//    the current time is within either the AM or PM LED lighting window and lights (or not)
//    the LEDs accordingly.
//
//    The first time that the PIR is tripped during any AM or PM LED lighting period, the
//    appropriate LED will flash for approximately 4 seconds (Light/unlight NUM_BLINKS times).
//    The software state must enter a period of no LEDs lit in order to enable the PIR blink
//    functionality to rearm for the next LED lighting period.

void loop()
{
  int timeValue;
  static byte pirState = PIR_DISARM;    // disarm the PIR until ready to arm it  
  
  //  display the current date and time on the LCD  
  if(timeStatus() != timeSet)
  {
    lcd.clear();    // Clear the display
    lcd.setCursor(0,0);
    lcd.print("RTC time not set");
    lcd.setCursor(0, 1);
    lcd.print("RESET sets time");
    delay(2000);    // wait a few seconds before retrying.  Blocking delay OK here - nothing else to do.
  }
  else    // get and display the time, light the LEDS, check and process PIR data
  {    
    if(nbDelay(TIME_UPDATE_INTERVAL) == false)
    {
      // set the time for debugging or normal     
      #ifdef DEBUG
        currentTime += 900;  // increment time by about one hour every second (does not test DST)
      #else
        currentTime = getCurrentTime(now());    // set the current time initially from the time library, converted by time zone
      #endif
      
      // display the date and time on the LCD
      displayDateTime(currentTime);
       
      //  Determine LED to be lit/blink (if any) and light/blink it
      //    AM?
      timeValue = hour(currentTime)*100 + minute(currentTime);     // get the hour in 24 hour format
      if( (timeValue >= AM_ON_TIME) && (timeValue <= AM_OFF_TIME) ) // AM on time
      {
         switch(pirState)
          {
            case(PIR_DISARM):
              pirState = PIR_READY;  // The PIR was previously disarmed and should be armed by the new LED light cycle 
              break;
              
            case(PIR_READY):
              if(digitalRead(pirPin) == LOW)  // PIR is armed and has been triggered
              {
                blinkLed(dayLightsAM[weekday(currentTime) - 1]);
                pirState = PIR_BLINKING;
              }
              else      // PIR is armed but not triggered -- just light the LED
              {
                lightLed(dayLightsAM[weekday(currentTime) - 1]);
              }
              break;
              
            case(PIR_BLINKING):    // the LED is blinking
              if( (blinkLed(dayLightsAM[weekday(currentTime) - 1])) == false) // LED blinkng done
              {
                pirState = PIR_COMPLETE;  // disarm until reset for next cycle
                lightLed(dayLightsAM[weekday(currentTime) - 1]);  // light the LED steady for duration of this cycle
              }
              break;
              
            case(PIR_COMPLETE):    // blinking has been done once this cycle - just light the LED
              lightLed(dayLightsAM[weekday(currentTime) - 1]);
              break;
              
            default:
              pirState = PIR_COMPLETE;    // just for safety :)
              break;
              
          }    // end of switch statement on pirState
      } // end of test for AM on time period
      
      else  // test for PM led on time period
      {                                       
        if( (timeValue >= PM_ON_TIME) && (timeValue <= PM_OFF_TIME) )  //PM on time      
        {

          switch(pirState)
          {
            case(PIR_DISARM):
              pirState = PIR_READY;  // The PIR was previously disarmed and should be armed by the new LED light cycle 
              break;
              
            case(PIR_READY):
              if(digitalRead(pirPin) == LOW)  // PIR is armed and has been triggered
              {
                blinkLed(dayLightsPM[weekday(currentTime) - 1]);
                pirState = PIR_BLINKING;
              }
              else      // PIR is armed but not triggered -- just light the LED
              {
                lightLed(dayLightsPM[weekday(currentTime) - 1]);
              }
              break;
              
            case(PIR_BLINKING):    // the LED is blinking
              if( (blinkLed(dayLightsPM[weekday(currentTime) - 1])) == false) // LED blinkng done
              {
                pirState = PIR_COMPLETE;  // disarm until reset for next cycle
                lightLed(dayLightsPM[weekday(currentTime) - 1]);  // light the LED steady for duration of this cycle
              }
              break;
              
            case(PIR_COMPLETE):    // blinking has been done once this cycle - just light the LED
              lightLed(dayLightsPM[weekday(currentTime) - 1]);
              break;
              
            default:
              pirState = PIR_COMPLETE;    // just for safety :)
              break;
              
          }    // end of switch statement on pirState
        }    // end of test for PM time period
        
        else  // not an led on time period
        {
          pirState = PIR_DISARM;  // disarm PIR blink function until next LED light time period
          lightLed(LED_OFF);
        }
      }
                     
    }  //  end of non-blocking delay
  }    // end of else - normal display  
  
}  // end of loop()

/****************************************END OF loop()******************************************/

/**********************************BEGINNING OF setDateTime()***********************************/
// setdateTime():  Function to allow the user to set the date and time of the RTC using the 
//   SELECT and INCREMENT buttons.  Once this function is entered, it can only be exited by advancing
//   the SELECT state to "INC to set time" and affirming by pressing INCREMENT.  
//
//   This function sets the global int variable timeZoneIndex to global constants DST or noDST, 
//   per user selection.  If the user selects DST, automatic daylight savings time adjustments
//   are made.  If not, the system maintains standard time throughout the year.  The rules for
//   daylight savings time are set in the global variables "dt" and "st" which are of type
//   TimeChangeRule that is defined in the <timezone.h> library.
//
//   This function accepts the current time (DST or not) from user selections and when the user
//   affirms the selected data, this function converts the user set time to standard (non-DST)
//   time and saves this time into the <time.h> library and into the RTC module.
//
//  arguments: 
//    none
//  return: 
//    none

void setDateTime()
{
  const String selectArray[] PROGMEM = {"Daylight Saving:", "Year:", "Month:", "Day:", "Hour:", "Minute:", 
                                         "AM/PM:", "INC to set time:"};
  const String timeZones[] PROGMEM =   {"automatic DST", "no DST"};
  const String theMonth[] PROGMEM = {"January", "February", "March", "April", "May", "June", "July", 
                                      "August", "September", "October", "November", "December"};
  static int theYear = 2014;
  static int monthIndex = 0;
  static int theDay = 1;
  static int theHour = 1;
  static int theMinute = 0;
  static boolean theAmPm = false;  // false = AM, true = PM
  static byte selectIndex = 0;
  boolean timeSetStatus = false;
  int theHour24;        // variable to hold the hour in military format
  time_t standardTime;  // variable to hold the standard (non-DST) time which is stored in the RTC
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(selectArray[selectIndex]);
  lcd.setCursor(0,1);
  lcd.print(timeZones[timeZoneIndex]); 

  do    // loop until time is set, then return
  {
    // first test for depression of select button and process it
    if(readButton(SELECT_ID, SELECT_PIN) == true)
    {
      if(selectIndex < 7)     // step to the next select element
      {
        selectIndex++;  // step to next choice
      }
      else
      {
        selectIndex = 0;
      }
      //  display the select element
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(selectArray[selectIndex]); 
      
      // display the second line per the current selection
      lcd.setCursor(0,1);
      switch (selectIndex)
      {
        // display the Timezone
        case 0:    
          lcd.print(timeZones[timeZoneIndex]);       
          break;
        
        // display the year
        case 1:    
          lcd.print(theYear);       
          break;
        
        //  display the month
        case 2:
         lcd.print(theMonth[monthIndex]);       
         break;
        
        //  display the day
        case 3:
          lcd.print(theDay);       
          break;
        
        //  display the hour
        case 4:
         lcd.print(theHour);      
         break;
        
        //  display the minute
        case 5:
          lcd.print(theMinute);        
          break;
        
        //  display the AM/PM
        case 6:
          if(theAmPm == false)
          { 
            lcd.print("AM");
          }
          else
          {
            lcd.print("PM");
          }
          break;
        
        //  display the RTC time setting
        case 7:
          lcd.print("INC=set,Sel=cont");       
          break;
        
        //  out of bounds or error
        default:
          lcd.print("error - Reset");  
          break;         
      }  // end of switch statement
    }  // end of loop until time is set
    
    //  now test for depression of the increment button and process it
    if(readButton(INCREMENT_ID, INCREMENT_PIN) == true)
    {      
      switch (selectIndex)
      {
        // setting the Timezone
        case 0:    
          if(timeZoneIndex < TZ_INDEX_MAX)
          {
            timeZoneIndex++;
          }
          else
          {
            timeZoneIndex = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(selectArray[selectIndex]);          
          lcd.setCursor(0,1);
          lcd.print(timeZones[timeZoneIndex]);         
          timeSetStatus = false;        
          break;
        
        // setting the year
        case 1:  
          theYear++;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(selectArray[selectIndex]);          
          lcd.setCursor(0,1);
          lcd.print(theYear);           
          timeSetStatus = false;        
          break;
        
        //  setting the month
        case 2:
          if(monthIndex < 11)
          {
            monthIndex++;
          }
          else{
            monthIndex = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(selectArray[selectIndex]);          
          lcd.setCursor(0,1);
          lcd.print(theMonth[monthIndex]);                 
          timeSetStatus = false;        
          break;
        
        //  setting the day -- NOTE: be careful of months less than 31 days (not automatic)
        case 3:
          if(theDay < 31)
          {
            theDay++;
          }
          else
          {
            theDay = 1;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(selectArray[selectIndex]);          
          lcd.setCursor(0,1);
          lcd.print(theDay);           
          timeSetStatus = false;        
          break;
        
        //  setting the hour
        case 4:
          if(theHour < 12)
          {
            theHour++;
          }
          else
          {
            theHour = 1;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(selectArray[selectIndex]);          
          lcd.setCursor(0,1);
          lcd.print(theHour);           
          timeSetStatus = false;       
          break;
        
        //  setting the minute
        case 5:
          if(theMinute < 59)
          {
            theMinute++;
          }
          else
          {
            theMinute = 0;
          }
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(selectArray[selectIndex]);          
          lcd.setCursor(0,1);
          lcd.print(theMinute);                   
          timeSetStatus = false;        
          break;
        
        //  setting the AM/PM
        case 6:
          theAmPm = !theAmPm;  // toggle AM/PM
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(selectArray[selectIndex]);          
          lcd.setCursor(0,1);
          if(theAmPm == false)
          { 
            lcd.print("AM");
          }
          else
          {
            lcd.print("PM");
          }           
          timeSetStatus = false;       
          break;
        
        //  break out of loop to set the time in the library and return
        case 7:      
          timeSetStatus = true;        
          break;
        
        //  out of bounds or error
        default:
         timeSetStatus = true;  
         break;         
      }  // end of switch statement
    }   // end of of readButton() for Increment button
  } while (timeSetStatus == false);
  
  //  set the time in the time library 
    // find the hour in 24 hour format
  theHour24 = theHour;
  if( (theAmPm == true) && theHour < 12)    // night
  {
    theHour24 += 12;
  }
  
  // fix up noon and midnight
  if(theHour == 12) 
  {
    if(theAmPm == true) 
    {
      theHour24 = theHour;
    }
    else
    {
      theHour24 = 0;
    }
  }
 
  // set the current time in the time library (temporary - to user specified time)
  setTime(theHour24, theMinute, 0, theDay, monthIndex + 1, theYear);  // January is month 1
  
  // establish the selected Timezone and use the timezone library to get the standard time
  switch (timeZoneIndex)
  {
    case DST:    // DST is observed and automated
      standardTime = useDT.toUTC(now());    
      break;
      
    case noDST:    // DT is not observed, standard time is used
      standardTime = useST.toUTC(now()); 
      break;   
    
    default:   // DST is observed
      standardTime = useDT.toUTC(now());    
      break; 
  }
  
  // set the time in the time library and in the RTC using the standard time
  setTime(standardTime);  // Time library is now set to UTC time
  RTC.set(now());  // store the time into the RTC module
  EEPROM.write(TZ_ADDR, timeZoneIndex);    // save the time zone index (DST or not) to EEPROM 
  
  // return to "Calibrating" mode after the time is set 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CALIBRATING ...");
  return;
  
}  // end of setDateTime()

/*************************************END OF setDateTime()**************************************/

/********************************BEGINNING of getCurrentTime() ********************************/
// getCurrentTime():  this function uses the proper timezone object (global) to convert standard 
//  time from the Time library and RTC into the proper local date and time.  The proper timezone 
//  object is defined by the global byte variable timeZoneIndex, as either DST or noDST.
//  
//  Arguments:
//      standard: a time_t variable containing the standard time
//  Return:
//      the local time, as a time_t variable

time_t getCurrentTime(time_t standard)
{  
  switch (timeZoneIndex)
  {
    case DST:    // Daylight savings time observed and automated
      return useDT.toLocal(standard);
      break;
      
     case noDST:    // Daylight savings time not observed, standard time used
       return useST.toLocal(standard);
       break;     
    
     default:      // default is to observe Daylight savings time
      return useDT.toLocal(standard);
      break;  
  }  // end of switch
  
}  // end of getCurrentTime()

/************************************END of getCurrentTime() ***********************************/

/********************************BEGINNING of displayDateTime() ********************************/
// displayDateTime():  function to display the date and time on the LCD.  The function accepts
//  the date-time to display as an argument of unix type time_t.  The function displays:
//    - The current date in month/day/year format on the first line, column 0 of the LCD
//    - The current day of the week on the first line, column 13 of the LCD
//    - The current time, in 12 hour:minute:second AM/PM format on the second line of 
//      the LCD, column 0.
//
//  arguments:
//    displayTime:  the time to display in unix time_t format.
//
//  return:
//    nothing

void displayDateTime(time_t displayTime)
{
  //      Strings for display of the day of week
  const String DOW[] PROGMEM = 
    {
     "---",      // invalid index of zero
     "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
    };
  int timeValue;
  
  lcd.clear();
  lcd.setCursor(0,0);
    
  //  First line is the date (month/day/year)
  lcd.print(month(displayTime));
  lcd.print("/");
  lcd.print(day(displayTime));
  lcd.print("/");
  lcd.print(year(displayTime));
      
  //  day of week in column 13
  lcd.setCursor(13,0);
  lcd.print(DOW[weekday(displayTime)]);
    
  //  Second line is the time (hour:minute:second; AM/PM)
  lcd.setCursor(0, 1);
    
  //    print the hour
  timeValue = hourFormat12(displayTime);
  if (timeValue < 10)
  {
    lcd.print("0");  // print the leading zero
  }
  lcd.print(timeValue);      
  lcd.print(":");
    
  //    print the minute    
  timeValue = minute(displayTime); 
  if (timeValue < 10)
  {
    lcd.print("0");  // print the leading zero
  }
  lcd.print(timeValue);      
  lcd.print(":");
    
  //    print the second
  timeValue = second(currentTime);
  if (timeValue < 10)
  {
    lcd.print("0");  // print the leading zero
  }
  lcd.print(timeValue);      
 
  //    print AM or PM
  lcd.setCursor(9, 1);
  if (isAM(currentTime) == true)
  {
    lcd.print("AM");
  } else
  {
    lcd.print("PM");
  }
  return;
  
}  // end of displayDateTime()

/***********************************END of displayDateTime() ***********************************/

/************************************BEGINNING OF nbDelay()*************************************/
// nbDelay():  non-blocking delay function.  This function maintains its internal state.  When not
//    in a delay (internal state is false), the internal state becomes true and the requested 
//    delay time is recorded.  When called with internal state of true, the function tests to see
//    if recorded delay time has expired.  If not, it returns true.  If the time has expired,
//    the the internal state retruns to false and the function returns false.
//
//  arguments:
//    delayTime: non-blocking delay time in milliseconds.
//
//  return:
//    internal state:  true if timing out the delay, false if delay time has expired.

boolean nbDelay(unsigned long delayTime)
{
  static boolean internalState = false;    // internal state is true only when timing out delay
  static unsigned long startTime;           // hold the starting time of the delay
  
  if (internalState == false)    // not delaying
  {
    startTime = millis();      // record the starting time of the delay
    internalState = true;      // set the flag to indicate delay in process
  }
  else                          // delay in process
  {
    if( diff(millis(), startTime) < delayTime)    // still timing out the delay
    {
      internalState = true;
    }
    else                      // delay has timed out
    {
      internalState = false;
    }
  }
  
  return internalState;
  
}  // end of nbDelay()

/************************************END OF nbDelay()*************************************/

/************************************BEGINNING OF lightLed()************************************/
//  lightLed():  Function to output an LED pattern to the LED hardware through a 74HC595
//    shift register.
//
//  arguments:
//    LedPattern:  the pattern to ourput to the shift register to light the proper LED
//
//  return:
//    none.

void lightLed(int LedPattern)
{
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, shiftPin, MSBFIRST, LedPattern);
    digitalWrite(latchPin, HIGH);
    
}  // end of lightLed()
/*************************************END OF lightLed()******************************************/

/***********************************BEGINNING OF blinkLed()**************************************/
//  blinkLed():  Function to blink the designated LED using lightLed().  Function is called
//    only after non-blocking delays, i.e. every 1/4 second which is also blink on or off time
//   
//  arguments:
//    LedPattern:  the pattern to output to the shift register to light the proper LED 
//
//  return:
//    true: if blinking the led
//    false: if not blinking the led

boolean blinkLed(int LedPattern)
{
  static boolean inBlink = false;
  static boolean toggle = false;  // toggle led on and off 
  static unsigned long blinkCount = 0; // the current count of blinks

  if(inBlink == false)    // a blink is to be started
  {
    inBlink = true;    // set the state to say blinking in progess
    blinkCount = 0;
    toggle = false;
  }
  else      // inBlink flag is true -- we are blinking
  {
    // test to see if blinking time is up
    if( blinkCount >= NUM_BLINKS)  // test if function completes
    {
      inBlink = false;    // time has expired, turn the flag off and  return
      blinkCount = 0;
    }
    else    // keep inBlink flag true and toggle the led    
    {
      inBlink = true;    // keep the flag set
      blinkCount++;
      
      // perform the blinking using the fact that blinkLed() is only called once every 1/4 second from loop()
      if(toggle == false)
      {
        toggle = true;  // turn on led next time through
        lightLed(LED_OFF);
      }
      else    // time to turn on the led
      {
        toggle = false;  // turn off led next time through
        lightLed(LedPattern);
      }
    }
 }    // end of processing the blink state
 
 return inBlink;    // return the current blink state 

} // end of blinkLed()
    
/*************************************END OF blinkLed()******************************************/

/*********************************BEGINNING OF readButton()*************************************/
// readButton(): Function to determine if a pushbutton switch is activated.  The function 
//  receives the ID and the Arduino pin number of the button.  If the button is LOW for longer 
//  than the debouncing time, "true" is returned.  If the button is not pressed for longer than 
//  the debouncing time, "false" is returned.
//
//  This function implements a non-blocking debouncing function.  Detection of a button depression
//  is flagged initially and the fact of button depression is stored, but "false" is returned.  This 
//  function must be repeatedly called in a loop and will return "true" only if the designated button
//  is still activated after the debouncing time (nominally 20 ms).
//
//  This function supports up to 256 buttons by keeping individual button state information
//  in arrays that are indexed by the "buttonID" ("byte" index).  To add more buttons, simply extend 
//  all the arrays to cover all necessary buttonID index values.
//
//  arguments:
//    buttonID:  the ID of a button.  The ID is expected to be an ordinal number beginning with zero for
//      for the first button, 1 for the seconds button, etc. up to n-1 for the "n"th button.
//    buttonPin:  the Arduino pin number for the button to be tested for activation
//
//  return:
//    true if the button indicated by the arguments is depressed for longer than the debouncing 
//      period; else false.

boolean readButton(byte buttonID, int buttonPin)
{
  static unsigned long changeDetectionTime[2];         // variables in array to hold the system time for debouncing
  static boolean debouncing[] = {false, false};        // set to true when in the process of debouncing
  static boolean buttonState[] = {false, false};       // hold the old (previous) value of the button
  boolean newButtonState[2];                        // new value of the buttons
  unsigned long timeInterval[2];                 // holds the time interval that a switch is depressed for debouncing
  
  if (debouncing[buttonID])      // code if in the process of debouncing a button
  {
    timeInterval[buttonID] = diff(millis(), changeDetectionTime[buttonID]);
    if (timeInterval[buttonID] < DEBOUNCE_TIME)      // still awaiting debounce
    {
      return false;
    }
    else      // debouncing time has expired; is a button still pressed?
    {
      debouncing[buttonID] = false;    // turn debouncing flag back off
      if ( digitalRead(buttonPin) == LOW)    // sample the button again
      {   
        newButtonState[buttonID] = true;  
      }
      else
      {
        newButtonState[buttonID] = false;
      }
      if (newButtonState[buttonID] == buttonState[buttonID])  // re-verify 
      {
        return newButtonState[buttonID];
      }
      else    // just noise
      {
        return false;
      }
 
    }
  }
  else      // code if not in the process of debouncing a button
  {
    // Test the button for activation
    if ( digitalRead(buttonPin) == LOW)    // detect if button is activated initially
    {   
      newButtonState[buttonID] = true;  
    }
    else
    {
      newButtonState[buttonID] = false;
    }

    // test for change of button state
    if (newButtonState[buttonID] == buttonState[buttonID])  // no change in button
    {
      debouncing[buttonID] = false;  
      return false;
    }
    else // buttons have changed
    {
      buttonState[buttonID] = newButtonState[buttonID];  // save the state of the button
      changeDetectionTime[buttonID] = millis();  // record the time for start of debouncing period
      debouncing[buttonID] = true;               // set the flag indicating time to debounce
      return false;                // no decision until after debouncing and re-verification
    }
  }

}  // end of readButtons()
/*************************************END OF readButtons()***************************************/

//************************************BEGINNING OF diff()****************************************/
// diff(): Function to subtract new and old millis() reading, correcting for millis() overflow
//
//    arguments:
//      newTime - the current, updated time from millis()
//      oldTime - the previous time, from millis(), for comparison
//    return:
//      the time difference (in milliseconds) corrected for millis() overflow

unsigned long diff(unsigned long newTime, unsigned long oldTime)
{
  const unsigned long MAX_TIME_VALUE = 0xFFFFFFFF; // max value of millis() before overflow to zero
  long timeInterval;
  
  if(newTime < oldTime)  // overflow has occurred, fix
  {
    timeInterval = newTime + (MAX_TIME_VALUE - oldTime) + 1L;
  }
  else
  {
    timeInterval = newTime - oldTime;
  }
  return timeInterval;
}  // end of diff()
//**************************************END OF diff()*******************************************/

