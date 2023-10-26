# Code Readme

Author: Benjamin Gilbert and Noah Robitshek

Date: 2023-10-06


## Summary

This code base can be split into two main parts: 

The ESP32 code is responsible for reading the sensors and printing the data to the serial port

The server code is responsible for reading the the data from the serial port and writing it to a csv file. The server code also provides a web interface, with canvas.js, for viewing the data.

## Project Files:
### - ESP32_Code -> main.c
This file should be flashed onto the ESP32 and will interface with the accelerometer, thermistor, button, and alphanumeric display. Inside this file. We use eight different FreeRTOS tasks to read the sensors, print the data to the Alphanumeric display, and send the data to the serial port. The button connected to a hardware interupt that will start and stop the data collection. 

### - data.csv
The data.csv file is the file that the ESP32 writes to. The server code reads from this file and parses the data. The data is in the following format:
```
Time,Temp,Steps
0.00, 24.411921, 0
```

### - server.js
The server.js file has a number of functions that will read and write to the csv file. One main funcion uses the "on.parser command" to read the serial port that the ESP writes to. In this function the server.js file is listening to the serial port for data or the string "Activtiy Started" or "Activity ended". Addationally, the server.js file wile read the data.csv file and parse the data into two arrays data1 and data2. Finally, the server.js file will send the data to the chart.html file via a web socket.

### - chart.html
The chart.html file is the file that the server.js file sends the data to. The chart.html file uses the canvas.js library to create a chart of the data. The chart will display the most recent 20 data points from the data.csv file.  


## AI Use

Artificial Intelligence (AI) was used once throughout the project. In all cases below they are documented in the codebase with the attributes:

```
// (1) gpt start
code
// gpt end
```
The number of the comment indicates the prompt below that was used to generate the code.

#### Prompt 1
Prompt: Write a function in C to convert a double to a 4 digit integer array for an alphanumeric display?





