// Importing Functions
const { connectToDatabase, initializeMeters, getAllData, parkCar, checkMeterStatus, handleParkQueue, timeUpQueue } = require('./db');

// UDP Declarations
const dgram = require('dgram');
const SERVER_PORT = 12345;
const server = dgram.createSocket('udp4');

// Server Setup
const express = require('express');
const http = require('http');
const socketIo = require('socket.io');
const app = express();
const server_fe = http.createServer(app);
const io = socketIo(server_fe);
app.use(express.static(__dirname));


let isProcessing = false;

// SEND MESSAGE BACK WHEN TIME IS COMPLETE

const fobLookupTable = {
  0: { id: 0, ip: '192.168.1.3', name: 'Fob 1' },
  1: { id: 1, ip: '192.168.1.27', name: 'Fob 2' },
  2: { id: 2, ip: '192.168.1.4', name: 'Fob 3' },
};

const meterLookupTable = {
  0: { id: 0, ip: '192.168.1.3', name: 'Meter 1' },
  1: { id: 1, ip: '192.168.1.8', name: 'Meter 2' },
  2: { id: 2, ip: '192.168.1.4', name: 'Meter 3' },
};

let meterData;
let dbInstance;

let parkQueue = []

// Connect to db
connectToDatabase((err, db) => {
  if (err) throw err;

  // Save the database instance for later use
  dbInstance = db;

  // Initialize empty meters if not done so
  initializeMeters(db, (initErr, result) => {
    if (initErr) throw initErr;
    else {
      // console.log("Meters Initialized: ", result);

      // Continually check meter status, and update based on time left
      checkMeterStatus(dbInstance);
    }
  });

  // Get all data
  setInterval(() => {
    getAllData(dbInstance, (getDataErr, data) => {
      if (getDataErr) throw getDataErr;
      else {
        meterData = data;
        // console.log("Current Data: ", data);
        if (meterData[0] != null) {
          io.emit('updateData', [meterData]);
        }
      }
    });
  }, 1 * 1000);

  // Park Cars in queue every 10 seconds

  setInterval(() => {
    handleParkQueue(dbInstance, parkQueue, (handleParkErr, result) => {
      if (handleParkErr) throw handleParkErr;
      else {
        console.log("Emptied park queue.")
      }
    })
  }, 10 * 1000);

  setInterval(() => {
    handleTimeUpQueue();
  }, 10*1000);
});

// Event handler for when the server is listening
server.on('listening', () => {
  const address = server.address();
  console.log(`UDP server is listening on ${address.address}:${address.port}`);
});

// Bind the server to the specified port
server.bind(SERVER_PORT);

// Event handler for when the server is closed
server.on('close', () => {
  console.log('UDP server is closed');
});

server.on('message', async (message) => {
  if (isProcessing) {
    console.log('Server is currently processing. Ignoring the new request.');
    return;
  }

  isProcessing = true;

  const match = message.toString('utf-8').split(',');
  const meterID = parseInt(match[0], 10);
  const fobID = parseInt(match[1], 10);
  console.log("Received request to park fob ", fobID, " in meter ", meterID);

  parkQueue.push({ 'meter_id': meterID, 'fob_id': fobID });

  // Wait until parkQueue is empty and meterStatus is up to date
  await waitForProcessing();

  const meterStatus = meterData.find(meter => meter['Meter_ID'] === meterID)?.['Status'];

  const meterIP = fobLookupTable[meterID].ip;
  const fobIP = meterLookupTable[fobID].ip;

  console.log("Sending APPROVED to fob IP: " + fobIP.toString() + " and a meter IP: " + meterIP.toString());
  sendUdpMessage(fobIP.toString(), 12345, "Alert: Park");
  sendUdpMessage(meterIP.toString(), 12345, "Alert: Park");
  
  isProcessing = false;
});

function sendUdpMessage(clientIp, clientPort, message) {
  const clientSocket = dgram.createSocket('udp4');
  clientSocket.send(message, 0, message.length, clientPort, clientIp, (err) => {
      if (err) {
          console.error(`Error sending UDP message to ${clientIp}:${clientPort}: ${err.message}`);
      } else {
          console.log(`UDP message sent to ${clientIp}:${clientPort}: ${message}`);
      }
      clientSocket.close();
  });
}

function handleTimeUpQueue() {
  if (timeUpQueue.length == 0) {
    // console.log("Time up queue is empty!");
  }
  else {
    // console.log("TimeUp Queue in call: ",timeUpQueue);
    const queueCopy = [...timeUpQueue];

    queueCopy.forEach((entry, index) => {
      const meterIP = fobLookupTable[entry['meter_id']].ip;
      const fobIP = meterLookupTable[entry['fob_id']].ip;
      console.log("Sending TIME UP to fob IP: " + fobIP.toString() + " and a meter IP: " + meterIP.toString());
      sendUdpMessage(fobIP.toString(), 12345, "Alert: Timeout");
      sendUdpMessage(meterIP.toString(), 12345, "Alert: Timeout");

      const entryIndex = timeUpQueue.findIndex((item) => item === entry);
      if (entryIndex !== -1) {
        timeUpQueue.splice(entryIndex, 1);
      } else {
        console.error('Processed entry not found in the original timeUpQueue');
      }
    })
  }
}

function waitForProcessing() {
  return new Promise(resolve => {
    const intervalId = setInterval(() => {
      if (parkQueue.length === 0 && meterData && meterData.length > 0) {
        clearInterval(intervalId);
        resolve();
      }
    }, 1000); // Adjust the interval as needed
  });
}

io.on('connection', (socket) => {
  console.log('A user connected');

  socket.emit('updateData', [meterData]);

  socket.on('disconnect', () => {
      console.log('User disconnected');
  });
});

server_fe.listen(8080, () => {
  console.log('Server is running on port 8080');
});

