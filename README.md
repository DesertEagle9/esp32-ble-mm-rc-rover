# esp32-ble-mm-rc-rover
Arduino code to connect an ESP32 to a Micromelon Rover wirelessly via Bluetooth Low Energy. The code also implements functionality for a user to control the movement of a wirelessly connected Micromelon Rover via four pushbuttons connected to the GPIO pins (Pin 22, 21, 17, 16) of the ESP32. The four pushbuttons enable the Micromelon  Rover to move in four ways: 1. forwards, 2. backwards, 3. pivot on the spot in a clockwise direction, 4. pivot on the spot in a counter-clockwise direction

This code was modified from the original Rui Santos' tutorial which can be found here: 
 https://RandomNerdTutorials.com/esp32-ble-server-client/

The Micromelon Python API can be found here:
https://github.com/Micromelon-Robotics/mm-pymodule

The particular ESP32 model used for the project is found here:
https://www.aliexpress.com/item/32846710180.html?_randl_currency=AUD&_randl_shipto=AU&src=google&src=google&albch=rmkt&acnt=576-373-4425&albcp=16560254345&albag=&slnk=&trgt=&plac=&crea=&netw=x&device=c&mtctp=&albbt=Google_7_rmkt&gclid=CjwKCAjwy_aUBhACEiwA2IHHQCae2f6CnmNbD0QBvAhtqfIuzd_kipWZuzskXRm6EVVpVPYGZNCiKBoCiBgQAvD_BwE&aff_fcid=ed62039dbced430782711f9484dc9d71-1654506581935-06456-UneMJZVf&aff_fsk=UneMJZVf&aff_platform=aaf&sk=UneMJZVf&aff_trace_key=ed62039dbced430782711f9484dc9d71-1654506581935-06456-UneMJZVf&terminal_id=60c76bfabdf64f9db3a17ac94d1f5248&afSmartRedirect=y
