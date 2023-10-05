# Quest Name

Authors: FirstName1 LastName1, FirstName2 LastName2, FirstName3 LastName 3

Date: YYYY-MM-DD

### Summary


### Self-Assessment 

| Objective Criterion | Rating | Max Value  | 
|---------------------------------------------|:-----------:|:---------:|
| Objective One |  |  1     | 
| Objective Two |  |  1     | 
| Objective Three |  |  1     | 
| Objective Four |  |  1     | 
| Objective Five |  |  1     | 
| Objective Six |  |  1     | 
| Objective Seven |  |  1     | 


## Solution Design

### Step counting
To count the steps we used the [Peak Detection algorithmn](https://saturncloud.io/blog/are-there-any-wellknown-algorithms-to-count-steps-based-on-the-accelerometer/#:~:text=1.,accelerometer%20data%20to%20detect%20steps.). We found the magnitude of the acceleration by taking the root of the sum of the squares of the acclerration in the X, Y, and Z axis. We then found the peaks in the data by finding the local maxima. We used a threshold of 20 to determine if a peak was a step or not. Addationally we implemented a debouncer to prevent multiple steps from being counted for a single step.

Below is a a graph of the magnitude of the acceleration over time. The peaks represent steps
![hello](./images/AccData.png)




### Sketches/Diagrams



### Supporting Artifacts
- [Link to video technical presentation](). Not to exceed 120s
- [Link to video demo](). Not to exceed 120s


### Modules, Tools, Source Used Including Attribution
[Step Counter Algorithm](https://saturncloud.io/blog/are-there-any-wellknown-algorithms-to-count-steps-based-on-the-accelerometer/#:~:text=1.,accelerometer%20data%20to%20detect%20steps.)


### AI Use


