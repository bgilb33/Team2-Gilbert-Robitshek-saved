# Code Readme

Author: Benjamin Gilbert and Noah Robitshek

Date: 2023-10-06


## Summary

This code base can be split into two main parts: ESP32 Code the Server code. The ESP32 code is responsible for reading the sensors and sending the data to the server. The server code is responsible for receiving the data from the ESP32 and write it to a csv file. The server code also provides a web interface for viewing the data. The server code parses the csv file and prints the data to a visual interface with canvas.js

## ESP32 Code
### Sample_project
This folder contains the code for the ESP32. The code is baised on the sample project provided by the ESP32. Below are some functions that are in the main file, main.c
1. **char_to_display_task()** This function will take call the current time that is set by the global timmer and will parse the time into 4 digits that can be printed onto the alphanumberic display. Addationally, the function will roll over to 0 after 99 seconds. This function is in a FreeRTOS task that is called continously running.
2. **button_task()** This function runs continously and will check the state of the button. The button is used to start and stop the activity. If the user is in an activity, the push of the button will pause the timer that is used for the timer interupts. 
3. **calcMagnitude()** This function will calculate the magnitude of the acceleration vector. The function will take the x, y, and z components of the acceleration vector and will return the magnitude of the vector. This vector is used in the peak detection algorithmn to calculate the number of steps a user takes.
4. **print_task()** This function will print the data to the serial port. On a set interval (in our case every 10 seconds), the data will be printed in the following format:
```
current_time, temp, step_count
```



## Server Code
### 1. data.csv
The data.csv file is the file that the ESP32 writes to. The server code reads from this file and parses the data. The data is in the following format:
```
Time,Temp,Steps
0.00, 24.411921, 0
```

### 2. server.js
The server.js file has a number of functions that will read and write to the csv file. One main funcion uses the "on.parser command" to read the serial port that the ESP writes to. In this function the server.js file is listening to the serial port for data or the string "Activtiy Started" or "Activity ended". Addationally, the server.js file wile read the data.csv file and parse the data into two arrays data1 and data2. Finally, the server.js file will send the data to the chart.html file via a web socket.

### 3. chart.html
The chart.html file is the file that the server.js file sends the data to. The chart.html file uses the canvas.js library to create a chart of the data. The chart will display the most recent 20 data points from the data.csv file.  

### AI Use

Artificial Intelligence (AI) was used a number of times throughout the project. In all cases below they are documented in the codebase with the attributes:

```
// (1) gpt start
code
// gpt end
```
The number of the comment indicates the prompt below that was used to generate the code.

#### Prompt 1
Prompt: XXX

#### Prompt 2
Prompt: XXX

#### Prompt 3
Prompt: XXX

