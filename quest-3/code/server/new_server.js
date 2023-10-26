// Required module
var dgram = require('dgram');
const readline = require('readline');


// Port and IP
var PORT = 49152;
var HOST = '192.168.1.23';

// Create socket
var server = dgram.createSocket('udp4');

// Create server that listens on a port
server.on('listening', function () {
    var address = server.address();
    console.log('UDP Server listening on ' + address.address + ":" + address.port);
});



// On connection, print out received message
server.on('message', async function (message, remote) {
    console.log(remote.address + ':' + remote.port +' - ' + message);

    server.send("all good", remote.port, remote.address, function (error) {
      if (error) {
        console.log('did not return number');
      } else {
        console.log('Package Recieved');
      }
    });

});

// Bind server to port and IP
server.bind(PORT, HOST);