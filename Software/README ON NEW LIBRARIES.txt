NOTES ON THE updatelibs BRANCH OF PILL REMINDER.

THIS BRANCH IS UNTESTED.

It was reported that the code does not compile with Arduino IDE 1.8.0. We have replaced three libraries and the code not compiles. We have not tested the code with these new libraries. The new versions are from:

Time:

https://github.com/PaulStoffregen/Time

01083f838c037d50d03e2dd228c56ef35eeb064b committed on Mar 20, 2017

Timezone: 

https://github.com/JChristensen/Timezone

8e96486b2c139ea5b6e656e17f43839e98218f8a committed on Mar 27, 2017


DS1307RTC

https://github.com/PaulStoffregen/DS1307RTC

160fa0dbca29f6fb3c1465954f89ad7f1f0fbf67 committed on Mar 10, 2016


These repositories were downloaded as zips, unzipped and placed in a PillReminder project folder named "Software/libraries" (note that libraries uses a lower case L). The Arduino IDE preferences were modified to point to the PillReminder project folder "Software" as the source of libraries.

This work was all done on a OS X 10.11.6. 

Your mileage may vary.