# Code Readme

In this readme is a discription of what is in our code folder and subfolder. 

## Folders
### Server
This folder contains all of the code that is hosted on the server. Throughout the testing process, we ran the server on our computers for testing but in the end, we places these files on the PI. 

**index.html** This is the html file that is the UI for the straba interface. Throuhgout the project we use templates from Canvas.js for alot of the graphs. Additionally, we use basic html and css for the custom UI elements including the download button and other features. 

**server.js** This is the main server file that will recieve the data through a socket on the UDP protocol. This file will parse the data and save the data to a csv file. 

**dataXX.cs** While running the program, all data will be sent to a csv file. The server file will identify a esp32 based on a static IP address and then make a custom a csv for each device. 


### code: 
This folder contains all of the code that is placed on the esp32. It contains all the header files, build files, and make files

**udpclient.c** This is the main file that runs on the esp32. This file is a combination of UDP client code written by Noah and Carmin code that was from a previous project. 


## Attribution
We used a number of different sources throghout our project. Below is a quick list:

- esp-idf UDP client
- esp-idf UDP server
- Canvas.js templates



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

