# Code Readme

In this readme is a discription of what is in our code folder and subfolder. 

## Folders

### 📁 frontend
The front end was build using Angular JS. The front end has mutiple pages which can be accessed through the login page. After inserting the correct user and pasword into the login page, the user is able to access the main page to see devices, set alarms, and configure devices.


### 📁 WebAPI2
In this folder we have the main node server that allows for communication between the front end and the back end. We call this our API becase with specific endpoits for the uri, we can get, and configure, data from the database.


### 📁 localNode
This folder contains two main node servers that subscribe and publish to the MQTT broker. These files are run on the raspberry pi and are used to communicate with the ESP32s.


## Contribution

**Benji:** Benji developed the entire user interface. This icnluded setting up a login feature to validate the user. Additionally it included setting up the database and finishing to build out the API. In the end, the team decided to use a cloud based mongoDB database and Benji took the lead in setting up the schema and writing the database calls. 

**Noah:** From the start, Noah designed the system architecture and the communication protocol. He drafted building the api which allowed communication between the ESP32s and the server. After Benji finished the front end, Noah transitioned to the IoT devices and build out the MQTT communication protocol. He topics for setup, configuration, data sending. Additionally, he built out the state machine for the ESP32s, alowwing for easy provisioning.
