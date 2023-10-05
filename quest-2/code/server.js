const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const fs = require('fs');
const csv = require('csv-parser'); // Require the csv-parser module
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('@serialport/parser-readline')

const app = express();
const server = http.createServer(app);
const io = socketIo(server);
const port = new SerialPort({ path: 'COM7', baudRate: 115200 })
const parser = port.pipe(new ReadlineParser({ delimiter: '\r\n' }))

var activityStatus = "";

parser.on('data', function (data) {
    if (data == 'Activity Started') {
        fs.writeFile('./data.csv', 'Time,Temp,Steps\n', function (err) {
            if (err) throw err;
        });
        activityStatus = ": Activity In Progress";
    }
    else if (data == "Activity Ended") {
        activityStatus = ": Activity Ended";
    }
    else {
        fs.appendFile('./data.csv', `${data}\n`, function (err) {
            if (err) throw err;
        })
    }

});


// Serve the HTML and client-side JavaScript on port 8080
app.use(express.static(__dirname)); // Serve files from the current directory

// Function to read and update CSV data
function readAndUpdateCSV() {
    const data1 = []; // To store parsed CSV data
    const data2 = []

    // Read the CSV file and parse it
    fs.createReadStream('./data.csv')
        .pipe(csv())
        .on('data', (row) => {
            data1.push({
                label: row.Time,
                y: parseInt(row.Temp), // Assuming temperature data is in column 'Temp'
            });

            data2.push({
              label: row.Time,
              y: parseInt(row.Steps), // Assuming temperature data is in column 'Temp'
            });            
        })
        .on('end', () => {
            // Emit the parsed data to all connected clients
            io.emit('updateData', [data1, data2, activityStatus]);
        });
}

// Set up a periodic task to read and update CSV data
setInterval(readAndUpdateCSV, 1000); // Update data every 1 second (adjust as needed)

// Socket.io connection handling
io.on('connection', (socket) => {
    console.log('A user connected');

    // Send initial data to the connected client (empty array initially)
    socket.emit('updateData', []);

    socket.on('disconnect', () => {
        console.log('User disconnected');
    });
});

server.listen(8080, () => {
    console.log('Server is running on port 8080');
});
