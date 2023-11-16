const tingodb = require('tingodb')();

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
        { 'Meter_ID': 0, 'Meter_IP': '123.456.789.0', 'Status': 'Open', 'Fob_IP': null, 'Start_Time': null, 'End_Time': null },
        { 'Meter_ID': 1, 'Meter_IP': '123.456.789.1', 'Status': 'Open', 'Fob_IP': null, 'Start_Time': null, 'End_Time': null },
        { 'Meter_ID': 2, 'Meter_IP': '123.456.789.2', 'Status': 'Open', 'Fob_IP': null, 'Start_Time': null, 'End_Time': null },
        { 'Meter_ID': 3, 'Meter_IP': '123.456.789.3', 'Status': 'Open', 'Fob_IP': null, 'Start_Time': null, 'End_Time': null },
        { 'Meter_ID': 4, 'Meter_IP': '123.456.789.4', 'Status': 'Open', 'Fob_IP': null, 'Start_Time': null, 'End_Time': null }
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

const parkCar = (db, meter_id, fob_ip, callback) => {
  const collection = getCollection(db, 'parking_table');
  const query = {
    'Meter_ID': meter_id
  };
  const startTime = new Date();
  const endTime = new Date(startTime.getTime() + 40 * 60000);

  const updateData = { $set: { 'Fob_IP': fob_ip, 'Status': 'Taken', 'Start_Time': startTime, 'End_Time': endTime } };

  collection.update(query, updateData, (updateErr, result) => {
    if (updateErr) {
      console.error(updateErr);
      callback(updateErr);
    }

    callback(null, result);
  });
};

function getMeterStatus (db, meter_id, callback) {
  const collection = getCollection(db, 'parking_table');
  const query = {
    'Meter_ID': meter_id
  };

  collection.find(query).toArray((findErr, result) => {
    if (findErr) {
      console.error(findErr);
      callback(findErr);
    }

    let status;
    let timeLeft;

    if (result.length > 0) {
      status = result[0]['Status'];

      if (status === "Taken") {
        const now = new Date();
        const timeDifferenceInMilliseconds = result[0]['End_Time'] - now;
        timeLeft = timeDifferenceInMilliseconds / (1000 * 60);
      } else {
        timeLeft = 0;
      }
    } else {
      status = "Not Found";
      timeLeft = 0;
    }

    callback(null, [status, timeLeft]);
  });
};

// Export the functions
module.exports = {
  connectToDatabase,
  getCollection,
  initializeMeters,
  getAllData,
  parkCar,
  getMeterStatus,
};
