# Automated_Cat_Feeder
An automated cat feeder project based on an ESP32!

## Project Components
* DRV8874 motor driver
* TPS65261 3-channel voltage regulator
* ESP32-S3-WROOM-1 with 8MB RAM
* OV2640 camera
* I2C OLED Display

## Project Overview
* Feeds cats using an auger design
* Has a food hopper for long term dispension
* Automated system with schedule and facial recognition
* Manual option for feeding immediately
* Utilizes WiFi for UI and obtaining the current time
* Smooth WiFi setup interface for new networks

## Project Status (as of 5/8/25)
* Partially working auger system for dispensing
* Working motor driver / motor system with current feedback
* Working jam detection using current feedback from motor
* Easy to use web interface containing settings; feed time length, a schedule (4x a day max), a change password page, a page to reset device or erase memory, and a page to update software, etc.
* Smooth WiFi setup for first time network connection with multiple fallbacks
* Working OLED display displaying feeder status

## How to Use
Upload the code from the respository to your cat feeder board.  Power up the device and then refer to the 'WiFi Setup' section for next steps.

## Project Description

### WiFi Setup
When first turning the system on, the board will check its flash for a saved network.  If there is a saved network, it will attempt to connect to it. If it can't connect after 30 sec, it will erase the saved network from flash, and restart itself.  If there is no saved network in flash, it will start an access point network called 'catfeeder_AP'.  After connecting to this network with your smartphone (laptop/dekstop works as well, but you must go to '192.168.4.1' in a browser), a captive portal will come up requiring you to put in a network name and password.  After you have entered these, the device will reboot, and attempt to connect to that network.

### User Interface (website)
If the WiFi connection is successful, go to 'catfeeder.local' in a browser, and a webpage will open requiring you to signin.  Trying to visit any of the feeder's webpages without signing in will kick you back to the signin page.  After successfully signing in, the home page will load which contains a menu bar, two main settings, and a 'feed now' button.  There is also an LED indicator that will turn green when the motor is running, and stay red when not.

All webpages that you open on this webpage will be loaded next to the homepage dynamically.  The page will never reload, nor should you need to reload it for any reason.  All data and pages are delivered dynamically and updated automatically.

### Siri Integration (for iPhone/Mac)
By adding a shortcut to your device, you can use "Hey Siri" to turn the feeder on*.  It utilizes the cookie authentication to be able to turn the device on with the '/feed' endpoint.  To call this shortcut, say "Hey Siri" and then the shortcut name.  I named my shortcut 'Feed my Cats', so I would say "Hey Siri, feed my cats".
Visit this link to retrieve the shortcut: https://www.icloud.com/shortcuts/d3358b3d90564f18b419d0107f0d7751

*Please note this only works when on the same WiFi network as the feeder itself.

## Unfinished Components / Issues (as of 5/8/25)
* I am still working on getting the camera to work, so no facial recognition or camera code is added yet
* Auger currently jams due to bad screw design
* OLED Display displays "feeding" even when not feeding
* Food measurement is currently in seconds the auger turns, instead of grams of food
* A small box appears next to the homepage after going to another webpage and going back

