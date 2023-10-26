const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const fs = require('fs');
const path = require('path');
const csv = require('csv-parser'); // Require the csv-parser module
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline')
const readline = require('readline');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);
const port = new SerialPort({ path: 'COM8', baudRate: 115200 })
const parser = port.pipe(new ReadlineParser({ delimiter: '\r\n'}))


app.use(express.static(__dirname)); // Serve files from the current directory

var deviceList = [];
var deviceCount = 0;
var temperatureData = [];
var stepData = [];
var leaderboardData = [];

var lastLine = ''
var secondToLastLine = ''

function readAndUpdateCSV() {
    parser.on('data', (line) => {
        var date = Date();
        var time = date.split(' ')[4];
        var espId, temperature, steps, status;
        if (line != lastLine && line != secondToLastLine) {
            // console.log(line);
            dataCheck = line.split(',');
            dataCheckFront = dataCheck[0].split(' ');
            if (dataCheckFront[dataCheckFront.length - 2] == 'INFO:') {
                [espId, temperature, steps] = line.split(',')
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

                if (!deviceExists) {
                    let header = `Device ID: ${deviceCount}\nESP32 ID: ${espId}\nTime, Temperature, Steps\nActivity Started\n${time}, ${temperature}, ${steps}\n`;
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
                        color: "#b0c4de"
                    });
                    stepData.push({
                        label: deviceCount.toString(),
                        y: parseInt(steps),
                        color: "#20b2aa"
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
                            stepData[i].y = parseInt(steps);
                            fs.appendFile(`./data/data_${deviceId}.csv`, `${time}, ${temperature}, ${steps}\n`, function (err) {
                                if (err) throw err;
                            })
                        }
                    }
                }
            }
            else if (dataCheckFront[dataCheckFront.length - 2] == 'STATUS:') {
                var [espId, status] = line.split(',')
                espId = espId.split(' ')[1];
                temp_color = ""
                step_color = ""
                activity_text = ""
                if (status == "In Activity") {
                    activity_text = "Activity Started\n";
                    temp_color = "lightsteelblue";
                    step_color = "lightseagreen";
                }
                else if (status == "Activity Ended") {
                    activity_text = "Activity Ended\n";
                    temp_color = "#D62600";
                    step_color = "#FF6F50";
                }
                for (let i = 0; i < deviceCount; i++) {
                    if (deviceList[i].EspId == espId) {
                        var deviceId = deviceList[i].DeviceId;
                        fs.appendFile(`./data/data_${deviceId}.csv`, activity_text, function (err) {
                            if (err) throw err;
                        })
                        temperatureData[i].color = temp_color;
                        stepData[i].color = step_color;
                    }
                }
            }
            secondToLastLine = lastLine;
            lastLine = line;

            io.emit('updateData', [temperatureData, stepData]);
        }
    });
}

// Set up a periodic task to read and update CSV data
//setInterval(readAndUpdateCSV, 1000); // Update data every 1 second (adjust as needed)

function updateLeaderBoard() {

    var leaderboard = {};

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
                    // console.log(row.split(' '));
                    deviceId_lb = row.split(' ')[2];
                    leaderboard[deviceId_lb] = 0;
                }
                if (row == "Activity Ended") {
                    let steps = lastRow.split(', ')[2];
                    leaderboard[deviceId_lb] += parseInt(steps);
                }
                lastRow = row;
                // console.log(leaderboard);
            })
            rl.on('close', () => {
                const keyValueArray = Object.entries(leaderboard);
                keyValueArray.sort((a, b) => a[1] - b[1]);
                const transformedArray = keyValueArray.map(item => ({ label: item[0], y: item[1] }));
                io.emit('updateDataLb', [transformedArray]);
            })
        })
    })
}
setInterval(updateLeaderBoard, 5000);

// Socket.io connection handling
io.on('connection', (socket) => {
    console.log('A user connected');

    // Send initial data to the connected client (empty array initially)
    socket.emit('updateData', []);
    socket.emit('updateDataLb', []);

    socket.on('disconnect', () => {
        console.log('User disconnected');
    });
});


server.listen(8080, () => {
    console.log('Server is running on port 8080');
});