# EnergyVore

Robotic oriented Arduino ESP8266 library and examples to control game robots and remote controls through a wifi server.

## typical use case

1 Both the robot and remote control connect to the wifi server. 
2 They send their identifier (set with jumpers) to the server. 
3 In response, the server accepts the devices into the network. 
4 Now the devices are able to send and receive data. 

* The remote control sends its joystick position and if it is clicked or not. The server forwards this information to the corresponding robot
* the robot sends a message to the server if it found an LED on the game board 
* when the server accepts the LED detection , it sends a message to the robot which turns on its own LED
* after a while, the server tells the robot to turn off its LED

Depending on the devices jumpers, the remote controls and the robots have a specific ID: a robot ID ranges from 0 up to 3, a remote control ID ranges from 4 up to 7. Any joystick state message sent from the remote control of ID x to the server is forwarded as-is to the robot of ID x - 4.

## Protocol

* Remote control and robot discovery message to server:
    * "'Z' {ID}" whith {ID} from '0' up to '7' (eg. Z6)
* Server to each device:
    * 'ZV' to accept the join request
* Remote control joystick position and button press state to server:
    * If the joystick is pressed
        * "'Y' {strength}" with {strength} ranging from '0' up to '9'. It is the position of the joystick from left to right (eg. Y5, the joystick is at the middle position)
    * Else
        * "{direction} {strenght}" with {direction} ranging from 'A' up to 'X', 'A' is straight ahead. {strenght} ranges from '0' up '9', '0' when the joystick is at the center position. (eg. F9, the joystick is at the most right position).
* Robot to the server :
    * D1 when a LED is detected on the game board
* Server to the robot
    * 'ZD': turn your LED on
    * 'ZO': turnb your LED off


