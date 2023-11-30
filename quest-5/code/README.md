# Code Readme

In this readme is a discription of what is in our code folder and subfolder. 

## Folders

### 📁 auth-server
This folder contains all of the code for the auth server. On the server is our database as well as a UDP server to intercept messages from the pi camera and send messages to the ESP32s. The server is written in Node.js and uses TingoDB as the database. Additionally, there is a user interface for that visually displays the capacity of the parking lot.

### 📁 PiCamera
This folder goes on the Raspberry Pi Zero. This contains all of the code to take a picture from the picamera, decode the picture, and send the decoded message to the auth server.

### 📁 Fob
This folder contains the code neccessary to run on the fob ESP32. This includes UDP code, IR sender code, LED change code,and state machine code.

### 📁 Meter
This folder contains the code necessary to run on the meter ESP32. This includes UDP code, IR reciever code, state machine code, and OLED display code.

### 📁 QRs
This folder contains the QR code images that we use to display on the OLED display. Additionally, this includes the QR codes for parked spot and unparked spot as well QR codes that we used for testing.

### 📁 Testing Files
In this folder contains a number of test files that we used to test our code. One helpful file we created was UDPsender.js which can send a packet to a specific IP address and port. This was helpful in testing our UDP server. Additinally we created the folder MeterGraphicTest that allowed us to test the state changes of the meter and the OLED display.

## Contribution

**Benji:** Developed the database schema and the auth server. This included creating a queue to store incomming UDP messages. Additionally, Benji developed all of the logic to check the database for a the status of a parking spot and to update the database when a parking spot is taken or released. Finally, Benji developed the user interface to display the capacity of the parking lot. 

**Noah:** Worked on the developing and configuring the devices. This included setting up the UDP communication and building state machines for the fob and the meter. Additionally, we worked on the pi camera system. Here he made sure the QR code would be displayed on the OLED display and configured the pi camera to take a picture and send it to the auth server.


