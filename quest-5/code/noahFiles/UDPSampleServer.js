const dgram = require('dgram');

const SERVER_PORT = 12345;  // Replace with the desired port number

const server = dgram.createSocket('udp4');

server.on('listening', () => {
    const address = server.address();
    console.log(`UDP server listening on ${address.address}:${address.port}`);
});

server.on('message', (msg, rinfo) => {
    console.log(`Received message from ${rinfo.address}:${rinfo.port}: ${msg}`);

    // Process the received data as needed

    // Respond to the client
    const responseMessage = 'Message received!';
    server.send(responseMessage, rinfo.port, rinfo.address, (err) => {
        if (err) {
            console.error(`Error sending response: ${err.message}`);
        } else {
            console.log(`Response sent to ${rinfo.address}:${rinfo.port}: ${responseMessage}`);
        }
    });
});

server.on('error', (err) => {
    console.error(`Server error: ${err.message}`);
});

server.bind(SERVER_PORT);
