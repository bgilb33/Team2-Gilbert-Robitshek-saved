# Code Readme

Author: Benjamin Gilbert and Noah Robitshek

Date: 2023-09-19


### Summary

This code folder consists of a subfolder, mission_impossible. Themission_impossible project is based on the esp-idf example-project, but we changed the main.c file to contain the code for the quest 1 assignment. This main.c file is within the main folder that is in the mission_impossible folder. This program uses everything we learned in the tasks to create a program and a circuit that detects the light in a room, the temperature in a room, whether a pressure plate is pressed, and display all of that information via the console and LEDs.

### Alarms
We communicated whether the circuit detected ambient light, body temperature, or a pressed pressure plate through LEDs on the board and the console. To do this, we created three global variables, light_alarm_state, thermo_alarm_state, and button_alarm, that, if set to 1, indicate that the event occurred, or if set to 0, indicate that the event did not occur. To communicate these values to the user, we created a FreeRTOS task called sound_alarm_task, that continually checks the values of these alarm states. If light_alarm_state is 1, it will light up the yellow LED. If thermo_alarm_state is 1, it will light up the red LED. If button_alarm is 1, it will light up the blue LED. Finally, if all of the alarm states are 0, it will light up the green LED.

### Photoresistor
To measure the light in the room, we used a photoresistor. On the software side, we created a FreeRTOS task called photoresistor_task. This task is based on the code used in the ADC1 example from skill assignment 07. This function continually samples the ADC reading between the photoresistor and a 10k Ohm resistor. It then converts this raw value to voltage and stores it in a global variable, photo_voltage. The function finally checks if the value is above our set threshold of 500 mV, and if it is, it sets the value of light_alarm_state to 1, and sets it to 0 otherwise.

### Thermistor
To measure the temperature in the room, we used a thermistor. Once again, we created a FreeRTOS task called photoresistor_task. This task is based on the code used in the ADC1 example from skill assignment 07. This function continually samples the ADC reading between the thermistor and a 10k Ohm resistor. It then converts this raw value to voltage which is then converted to the resistance in the thermistor. Using this resistance and the Steinhart Equation, we found the temperature of the thermistor and stored it in the global variable, temperature. The function finally checks if the value is above our set threshold of 27 degrees Celcius, and if it is, it sets the value of thermo_alarm_state to 1, and sets it to 0 otherwise.

### Pressure Plate
To measure whether someone stepped on the "pressure plate in the room", we used a button. Once again, we created a FreeRTOS task called photoresistor_task. This task is based on the code used in the hardware interrupt design pattern example in skill assignment 12. We declared a global flag variable that is initially set to 0. We then set up the hardware interrupt in the init function. Next, we created a GPIO ISR Handler that sets the global variable button_alarm to 1 and lights the when the flag is 1, which happens when the button is pressed. Finally, the flag is reset to 0 inside of the sound_alarm task after the sound_alarm task turns on the blue LED.

### Reset Button
Since the pressure plate button can only turn the blue LED on, we had to make a "reset switch" to turn it off. To do so, we used another button. However, to take full advantage of all the skills we learned, we did so with the polling design pattern. We  made a FreeRTOS task, reset_task that continually checks if the reset button has been pressed. If the reset button is pressed and the pressure plate button alarm has been tripped (button_alarm == 1), then button_alarm is set to 0.

### Printing
Finally, we wanted to display the current values and statuses of the photoresistor, thermistor, and pressure plate to the user through the console. We created a FreeRTOS task, print_reading_task, that continually checks the values of the global variables button_alarm, light_alarm_state, and thermo_alarm state. The task then prints out whether the pressure plate button has been pressed, the voltage of the photoresistor in mV and whether its threshold has been passed, and the temperature of the thermistor in degrees Celcius and whether its threshold has been passed. This is all formatted to be user friendly and is printed every 2 seconds.

