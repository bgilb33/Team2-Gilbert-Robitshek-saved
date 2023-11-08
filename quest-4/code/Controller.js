const dgram = require('dgram');

// Create a UDP client
const client = dgram.createSocket('udp4');

// Define the server address and port to send the datagram
const serverAddress = '192.168.1.27'; // Replace with the server's IP address
const serverPort = 12345; // Replace with the server's port number

const readline = require('readline').createInterface({
    input: process.stdin,
    output: process.stdout
});

function sendUserInputToServer() {
    readline.question('Enter command (or "exit" to quit): ', userInput => {
        if (userInput.toLowerCase() === 'exit') {
            // If the user enters "exit," close the client and exit the program
            client.close(() => {
                console.log('UDP client closed. Exiting...');
                process.exit(0);
            });
        } else {
            const buffer = Buffer.from(userInput);
            client.send(buffer, serverPort, serverAddress, (err) => {
                if (err) {
                    console.error(`Error sending message: ${err}`);
                } else {
                    console.log(`Message sent to ${serverAddress}:${serverPort}`);
                }
                sendUserInputToServer(); // Continue the loop
            });
        }
    });
}

// Start the loop
sendUserInputToServer();
