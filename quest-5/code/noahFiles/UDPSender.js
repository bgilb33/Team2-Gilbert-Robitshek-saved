// This is a testing program to send and recieve udp packets

const dgram = require('dgram');
const readline = require('readline');

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

const client = dgram.createSocket('udp4');


// Set the server address and port
const serverAddress = '192.168.1.23'; // Change this to your server's IP address
const serverPort = 12345; // Change this to your server's port

client.bind(serverPort);


// Function to send UDP messages
function sendMessage(message) {
  client.send(message, serverPort, serverAddress, (err) => {
    if (err) throw err;
    console.log(`Message sent to ${serverAddress}:${serverPort}: ${message}`);
  });
}

// Read user input and send messages
rl.setPrompt('Enter message to send (or type "exit" to quit): ');
rl.prompt();

rl.on('line', (input) => {
  if (input.toLowerCase() === 'exit') {
    rl.close();
    client.close();
  } else {
    sendMessage(input);
    rl.prompt();
  }
});

// Handle user input stream end
rl.on('close', () => {
  console.log('UDP sender closed.');
  process.exit(0);
});

// Handle socket errors
client.on('error', (err) => {
  console.error(`UDP socket error:\n${err.stack}`);
  client.close();
});

client.on('message', (message, remote) => {
    console.log(`Received message from ${remote.address}:${remote.port}: ${message}`);
  });
  

// Handle socket closure
client.on('close', () => {
  console.log('UDP socket closed.');
});
