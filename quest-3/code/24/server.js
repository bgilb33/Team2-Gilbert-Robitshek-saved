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

// Port and IP
var PORT = 49152;
var HOST = '192.168.1.23';

// Create socket
var server = dgram.createSocket('udp4');

server.on('listening', function () {
    var address = server.address();
    console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

app.use(express.static(__dirname)); // Serve files from the current directory

var deviceList = [];
var deviceCount = 0;
var temperatureData = [];
var stepData = [];

var lastLine = ''
var secondToLastLine = ''

server.on('message', async function (message, remote) {
    var line = message.toString();

    server.send("All good!", remote.port, remote.address, function (error) {
        if (error) throw error;
    });

    var date = Date();
    var time = date.split(' ')[4];
    var espId, temperature, steps, status;
    var temp_color, step_color;
    if (line != lastLine && line != secondToLastLine) {
        dataCheck = line.split(',');
        dataCheckFront = dataCheck[0].split(' ');

        if (dataCheckFront[dataCheckFront.length - 2] == 'DATA:') {
            [espId, status,temperature, steps] = line.split(',')
            espId = espId.split(' ')[1];
            var deviceExists = false;
            var deviceId;

            if (deviceCount != 0) {
                for (let i = 0; i < deviceCount; i++) {
                    if (espId == deviceList[i].EspId) {
                        deviceExists = true;
                        break;
                    }
                }
            }

            // In Activity
            if (status == -1) {
                temp_color = "lightsteelblue";
                step_color = "lightseagreen";
            }
            // Not in Activity
            else if (status == 0) {
                temp_color = "#D62600";
                step_color = "#FF6F50";
            }

            // Add device if it has not been seen yet
            if (!deviceExists) {
                let header = `Device ID: ${deviceCount}\nESP32 ID: ${espId}\nTime, Temperature, Steps\nActivity Started\n${time}, ${temperature}, ${steps}\n`.trim();
                console.log("Header ", header);
                fs.writeFile(`./data/data_${deviceCount}.csv`, header, function (err) {
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
            else {
                for (let i = 0; i < deviceCount; i++) {
                    if (deviceList[i].EspId == espId) {
                        deviceId = deviceList[i].DeviceId;
                    }                   
                }
                for (let i = 0; i < deviceCount; i++) {
                    if (temperatureData[i].label == deviceId) {
                        temperatureData[i].y = parseFloat(temperature);
                        temperatureData[i].color = temp_color;
                        stepData[i].y = parseInt(steps);
                        stepData[i].color = step_color;
                        console.log("Write: ", `${time}, ${temperature}, ${steps}\n`)
                        fs.appendFile(`./data/data_${deviceId}.csv`, `${time}, ${temperature}, ${steps}\n`.trim(), function (err) {
                            if (err) throw err;
                        })
                    }
                }
            }
        }
        secondToLastLine = lastLine;
        lastLine = line;

        io.emit('updateData', [temperatureData, stepData]);
    }
});


// Set up a periodic task to read and update CSV data
// setInterval(readAndUpdateCSV, 1000); // Update data every 1 second (adjust as needed)


// Socket.io connection handling
io.on('connection', (socket) => {
    console.log('A user connected');

    // Send initial data to the connected client (empty array initially)
    socket.emit('updateData', []);

    socket.on('disconnect', () => {
        console.log('User disconnected');
    });
});

server.bind(PORT, HOST);

server2.listen(8080, () => {
    console.log('Server is running on port 8080');
});