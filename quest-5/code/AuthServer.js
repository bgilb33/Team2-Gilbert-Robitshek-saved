const dgram = require('dgram');

// Constants
const SERVER_PORT = 12345;  // Replace with the same port used in the Python client

// Create a UDP server
const server = dgram.createSocket('udp4');

// Event handler for when a message is received
server.on('message', (message) => {
    const match = message.toString('utf-8').match(/Meter: (\d+), Fob: (\d+)/);
    const meterID = parseInt(match[1], 10);
    const fobID = parseInt(match[2], 10);

    const meterStatus = getStatus(meterID);

    const meterIP = fobLookupTable[meterID].ip;
    const fobIP = meterLookupTable[fobID].ip;

    if(meterStatus){
        console.log("Sending APPROVED to fob IP: " + fobIP.toString() + " and a meter IP: " + meterIP.toString());
    }
    else{
        console.log("Sending NOT APPROVED to fob IP: " + fobIP.toString() + " and a meter IP: " + meterIP.toString());
    }

});

// fake get status function: if EVEN return 0, ODD return 1
function getStatus(meterID){
    if(meterID % 2 == 0){return 0}
    else{return 1}
}

const fobLookupTable = {
    1: { id: 1, ip: '192.168.1.2', name: 'Meter 1' },
    2: { id: 2, ip: '192.168.1.3', name: 'Meter 2' },
    3: { id: 3, ip: '192.168.1.4', name: 'Meter 3' },
    // Add more devices as needed
};

const meterLookupTable = {
    1: { id: 1, ip: '192.168.1.2', name: 'Fob 1' },
    2: { id: 2, ip: '192.168.1.3', name: 'Fob 2' },
    3: { id: 3, ip: '192.168.1.4', name: 'Fob 3' },
    // Add more devices as needed
};

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

// Handle server shutdown gracefully
process.on('SIGINT', () => {
    server.close(() => {
        console.log('UDP server is gracefully shutting down');
        process.exit();
    });
});
