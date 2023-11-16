const dbModule = require('./db.js');

// Example usage
dbModule.connectToDatabase((connectErr, db) => {
  if (connectErr) {
    console.error(connectErr);
    return;
  }

  // Example 1: Get all data
  dbModule.getAllData(db, (getDataErr, data) => {
    if (getDataErr) {
      console.error(getDataErr);
    } else {
      console.log('All Data:', data);
    }

    // Example 2: Park a car
    dbModule.parkCar(db, 0, '208.208.208.0', (parkCarErr, parkCarResult) => {
      if (parkCarErr) {
        console.error(parkCarErr);
      } else {
        console.log('Car parked successfully:', parkCarResult);
      }

      // Example 3: Get meter status
      dbModule.getMeterStatus(db, 0, (getMeterStatusErr, meterStatus) => {
        if (getMeterStatusErr) {
          console.error(getMeterStatusErr);
        } else {
          console.log('Meter Status:', meterStatus);
        }

        // Close the database connection when done
        db.close();
      });
    });
  });
});
