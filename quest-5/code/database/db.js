//db.js
const tingodb = require('tingodb')();

const timeUpQueue = [];

const connectToDatabase = (callback) => {
  const db = new tingodb.Db('./mydb', {});
  db.open((err) => {
    if (err) {
      console.error(err);
      callback(err);
    } else {
      callback(null, db);
    }
  });
};

const getCollection = (db, collectionName) => {
  return db.collection(collectionName);
};

const initializeMeters = (db, callback) => {
  const collection = getCollection(db, 'parking_table');

  // Check if the collection is empty
  collection.count({}, (err, count) => {
    if (err) {
      console.error(err);
      callback(err);
    }

    if (count === 0) {
      // Initialize meters only if the collection is empty
      const records = [
        { 'Meter_ID': 0, 'Status': 'Open', 'Fob_ID': null, 'Start_Time': null, 'End_Time': null },
        { 'Meter_ID': 1, 'Status': 'Open', 'Fob_ID': null, 'Start_Time': null, 'End_Time': null },
        { 'Meter_ID': 2, 'Status': 'Open', 'Fob_ID': null, 'Start_Time': null, 'End_Time': null }
      ];

      collection.insert(records, (insertErr, result) => {
        if (insertErr) {
          console.error(insertErr);
          callback(insertErr);
        }

        callback(null, result);
      });
    } else {
      callback(null, 'Collection is not empty');
    }
  });
};

const getAllData = (db, callback) => {
  const collection = getCollection(db, 'parking_table');
  collection.find().toArray((err, result) => {
    if (err) {
      console.error(err);
      callback(err);
    }

    callback(null, result);
  });
};

const parkCar = (db, meter_id, fob_id, callback) => {
  const collection = getCollection(db, 'parking_table');
  const query = {
    'Meter_ID': meter_id
  };
  const startTime = new Date();
  const endTime = new Date(startTime.getTime() + 2 * 60000);

  const updateData = { $set: { 'Fob_ID': fob_id, 'Status': 'Taken', 'Start_Time': startTime, 'End_Time': endTime } };

  collection.update(query, updateData, (updateErr, result) => {
    if (updateErr) {
      console.error(updateErr);
      callback(updateErr);
    }

    callback(null, result);
  });
};

// Park queue format = [{meter_id, fob_id}]
const handleParkQueue = (db, parkQueue) => {
  console.log("Current Park Queue: ", parkQueue);
  if (parkQueue.length === 0) {
    console.log("No cars waiting to be parked!");
  } else {
    // Create a copy of the parkQueue to avoid modifying it while iterating
    const queueCopy = [...parkQueue];

    queueCopy.forEach((entry, index) => {
      parkCar(db, entry['meter_id'], entry['fob_id'], (parkErr, result) => {
        if (parkErr) {
          console.error('Error parking car:', parkErr.message);
        } else {
          console.log('Fob ID ', entry['fob_id'], ' parked with Meter ID ', entry['meter_id']);
          // Remove the processed entry from the original parkQueue
          const entryIndex = parkQueue.findIndex((item) => item === entry);
          if (entryIndex !== -1) {
            parkQueue.splice(entryIndex, 1);
          } else {
            console.error('Processed entry not found in the original parkQueue');
          }
        }
      });
    });
  }
};

// function getMeterStatus (db, meter_id, callback) {
//   const collection = getCollection(db, 'parking_table');
//   const query = {
//     'Meter_ID': meter_id
//   };

//   collection.find(query).toArray((findErr, result) => {
//     if (findErr) {
//       console.error(findErr);
//       callback(findErr);
//     }

//     let status;
//     let timeLeft;

//     if (result.length > 0) {
//       status = result[0]['Status'];

//       if (status === "Taken") {
//         const now = new Date();
//         const timeDifferenceInMilliseconds = result[0]['End_Time'] - now;
//         timeLeft = timeDifferenceInMilliseconds / (1000 * 60);
//       } else {
//         timeLeft = 0;
//       }
//     } else {
//       status = "Not Found";
//       timeLeft = 0;
//     }

//     callback(null, [status, timeLeft]);
//   });
// };

function checkMeterStatus(db) {
  const collection = getCollection(db, 'parking_table');
  // Set the interval (e.g., every 10 sec)
  const intervalInMilliseconds = 10 * 1000;

  // Periodically check the status
  setInterval(() => {
    console.log("Timeup Queue: ", timeUpQueue);
    collection.find({ 'Status': 'Taken' }).toArray((err, results) => {
      if (err) {
        console.error(err);
        return;
      }

      // Iterate through the results and check end times
      results.forEach((meter) => {
        // console.log("Checking Time Left:");
        const currentTime = new Date();
        const endTime = new Date(meter['End_Time']);

        // console.log("MeterID: ", meter['Meter_ID']);
        // console.log("Current Time: ", currentTime);
        // console.log("End Time: ", endTime);
        // console.log("Status: ", meter['Status']);


        if (currentTime > endTime) {
          // End time has passed, update status to 'Open'
          const query = { 'Meter_ID': meter['Meter_ID'] };
          const updateData = { $set: { 'Status': 'Open', 'Fob_ID': null, 'Start_Time': null, 'End_Time': null } };
          timeUpQueue.push({'meter_id': meter['Meter_ID'], 'fob_id': meter['Fob_ID']})

          collection.update(query, updateData, (updateErr, updateResult) => {
            if (updateErr) {
              console.error(updateErr);
            } else {
              console.log(`Meter ${meter['Meter_ID']} status updated to 'Open'.`);
              // CALL SOMETHING THAT SENDS MESSAGE TO FOB AND METER changing status to G
            }
          });
        }
      });
    });
  }, intervalInMilliseconds);
}

// checkMeterStatus(db);

// Export the functions
module.exports = {
  connectToDatabase,
  getCollection,
  initializeMeters,
  getAllData,
  parkCar,
  checkMeterStatus,
  handleParkQueue,
  timeUpQueue
  // getMeterStatus,
};
