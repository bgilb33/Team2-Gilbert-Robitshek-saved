const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const fs = require('fs');
const path = require('path');

const app = express();
const server2 = http.createServer(app);
const io = socketIo(server2);

var dgram = require('dgram');
const readline = require('readline');

// Define the UDP server's PORT and HOST
var PORT = 49152;
var HOST = '192.168.1.23';

// Create UDP socket
var server = dgram.createSocket('udp4');

server.on('listening', function () {
    var address = server.address();
    console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

app.use(express.static(__dirname));

// Initialize emptdy data variables
var deviceList = [];
var deviceCount = 0;
var temperatureData = [];
var stepData = [];

var leader = 0;
var lastLine = ''
var secondToLastLine = ''
var last_status = 0;

// Handle data from UDP server
server.on('message', async function (message, remote) {
    var line = message.toString();

    // Send current leader back to ESP32
    if (Object.keys(leaderboard).length > 0) {
        var dataToSend = `LEADER: ${leader}`;
    }
    else {
        dataToSend = "LEADER: 0";
    }

    server.send(dataToSend, remote.port, remote.address, function (error) {
        if (error) throw error;
    });

    // Parse and process incoming data
    var date = Date();
    var time = date.split(' ')[4]
    var espId, temperature, steps, status;
    var temp_color, step_color;
    var activity_ended = 0;
    var deviceId;

    if (line != lastLine && line != secondToLastLine) {
        dataCheck = line.split(',');
        dataCheckFront = dataCheck[0].split(' ');

        // Check if incoming message is data we want
        if (dataCheckFront[dataCheckFront.length - 2] == 'DATA:') {
            [espId, status,temperature, steps] = line.split(',')
            espId = espId.split(' ')[1];
            var deviceExists = false;

            // Check if device has been seen yet
            if (deviceCount != 0) {
                for (let i = 0; i < deviceCount; i++) {
                    if (espId == deviceList[i].EspId) {
                        deviceExists = true;
                        break;
                    }
                }
            }

            // Manage data color
            if (status == -1) {
                temp_color = "lightsteelblue";
                step_color = "lightseagreen";
            }
            else if (status == 0) {
                temp_color = "#D62600";
                step_color = "#FF6F50";
            }

            if (status == 0 && last_status == -1) {
                activity_ended = 1;
            }
            else {
                activity_ended = 0;
            }
            
            // Get device ID
            if (deviceCount > 0) {
                for (let i = 0; i < deviceCount; i++) {
                    if (deviceList[i].EspId == espId) {
                        deviceId = deviceList[i].DeviceId;
                    }                   
                }
            }

            // Add device if it has not been seen yet
            if (!deviceExists) {
                let header = `Device ID: ${deviceCount}\nESP32 ID: ${espId.replace(/\0/g, '')}\nTime, Temperature, Steps\nActivity Started\n${time.replace(/\0/g, '')}, ${temperature.replace(/\0/g, '')}, ${steps.replace(/\0/g, '')}`;
                fs.writeFile(`./data/data_${deviceCount}.csv`, header.replace(/\0/g, ''), function (err) {
                    if (err) throw err;
                });
                deviceList.push({
                    EspId: espId,
                    DeviceId: deviceCount
                });
                temperatureData.push({
                    label: deviceCount.toString(),
                    y: parseFloat(temperature),
                    color: temp_color
                });
                stepData.push({
                    label: deviceCount.toString(),
                    y: parseInt(steps),
                    color: step_color
                })
                deviceCount++;
            }
            // In Activity: log data
            else if (status == -1) {
                for (let i = 0; i < deviceCount; i++) {
                    if (temperatureData[i].label == deviceId) {
                        temperatureData[i].y = parseFloat(temperature);
                        temperatureData[i].color = temp_color;
                        stepData[i].y = parseInt(steps);
                        stepData[i].color = step_color;
                        if (deviceId != undefined) {
                            fs.appendFile(`./data/data_${deviceId}.csv`, `${time}, ${temperature.replace(/\0/g, '')}, ${steps.replace(/\0/g, '')}`.replace(/\0/g, ''), function (err) {
                                if (err) throw err;
                            })
                        }
                    }
                }
            }
            // Update data color
            else {
                for (let i = 0; i < deviceCount; i++) {
                    if (temperatureData[i].label == deviceId) {
                        temperatureData[i].color = temp_color;
                        stepData[i].color = step_color;
                    }
                }
            }
            // Write activity status to CSV
            if (activity_ended && deviceCount > 0 && lastLine != "Activity Ended") {
                if (deviceId != undefined) {
                    fs.appendFile(`./data/data_${deviceId}.csv`, "Activity Ended\n", function (err) {
                        if (err) throw err;
                    })
                }
            }
        }
        last_status = status;
        secondToLastLine = lastLine;
        lastLine = line;
        updateLeaderBoard();
        io.emit('updateData', [temperatureData, stepData]);
    }
});

// Function to update the leaderboad
var leaderboard = {};
function updateLeaderBoard() {
    leaderboard = {};

    // Read all device's data and update the csv
    fs.readdir('./data', (err, files) => {
        if (err) throw err;
        files.forEach(file => {
            const filePath = path.join('./data', file);
            var lastRow;
            var deviceId_lb;
            const rl = readline.createInterface({
                input: fs.createReadStream(filePath)
            });

            rl.on('line', (row) => {
                if (row.startsWith("Device ID:")) {
                    deviceId_lb = row.split(' ')[2];
                    leaderboard[deviceId_lb] = 0;
                }
                if (row == "Activity Ended") {
                    let steps = lastRow.split(', ')[2];
                    leaderboard[deviceId_lb] += parseInt(steps);
                }
                lastRow = row;
            })
            rl.on('close', () => {
                const keyValueArray = Object.entries(leaderboard);
                keyValueArray.sort((a, b) => a[1] - b[1]);
                const transformedArray = keyValueArray.map(item => ({ label: item[0], y: item[1] }));
                io.emit('updateDataLb', [transformedArray]);
            })
        })
        leader = Object.keys(leaderboard)[0];
        console.log(leaderboard);
    })
}

io.on('connection', (socket) => {
    console.log('A user connected');

    socket.emit('updateData', []);
    socket.emit('updateDataLb', []);

    socket.on('disconnect', () => {
        console.log('User disconnected');
    });
});

server.bind(PORT, HOST);

server2.listen(8080, () => {
    console.log('Server is running on port 8080');
});