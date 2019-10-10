'use strict';
console.log('Loading function');
let aws = require('aws-sdk');
let doc = require('dynamodb-doc');
let dynamo = new doc.DynamoDB();

var location_lat = 37.26;
var location_lon = -119.62;
var location_lat_hopper = location_lat + (Math.random() % 4 -2);
var location_lon_hopper = location_lon + 3 + (Math.random() % 4 -2);
var location_lat_knuth = location_lat + 3 + (Math.random() % 4 -2);
var location_lon_knuth = location_lon + 40 + (Math.random() % 4 -2);
var location_lat_turing = location_lat + 10 + (Math.random() % 4 -2);
var location_lon_turing = location_lon + 20 + (Math.random() % 4 -2);



function addToTimeseriesDb(event, tableName, timeStampEpoch, timeStampIso, locationLat, locationLon) {
    
    let params = {
        TableName: tableName,
        Item: {
            'deviceId': event.deviceId,
            'payloadTimestamp': timeStampEpoch,
            'payload': {
                'deviceId': event.deviceId,
                'sensorReading': event.sensorReading,
                'batteryCharge': event.batteryCharge,
                'batteryDischargeRate': event.batteryDischargeRate,
                'timeStampEpoch': timeStampEpoch,
                'timeStampIso': timeStampIso,
                'location': {
                    'lat': locationLat,
                    'lon': locationLon
                }
            }
        }
    };

    dynamo.putItem(params, function(err, data) {
        if (err) {
            console.log("putItem Error", err);
        } else {
            console.log("putItem Success", data);
        }
    });    
}


function addToDevicestatusDb(event, tableName, timeStampEpoch, timeStampIso, locationLat, locationLon) {
    
    let params = {
        TableName: tableName,
        Item: {
            'deviceId': event.deviceId,
            'payload': {
                'deviceId': event.deviceId,
                'sensorReading': event.sensorReading,
                'batteryCharge': event.batteryCharge,
                'batteryDischargeRate': event.batteryDischargeRate,
                'timeStampEpoch': timeStampEpoch,
                'timeStampIso': timeStampIso,
                'location': {
                    'lat': locationLat,
                    'lon': locationLon
                }
            }
        }
    };

    dynamo.putItem(params, function(err, data) {
        if (err) {
            console.log("putItem Error", err);
        } else {
            console.log("putItem Success", data);
        }
    });    
}


function addToDb(event, timeStampEpoch, timeStampIso, locationLat, locationLon) {

    addToTimeseriesDb(event, 
        'GreengrassDashboard-IoTGSDynamoTimeSeriesTable-WT6JJZXKXR0J', 
        timeStampEpoch, timeStampIso, 
        locationLat, locationLon
        );
    
    addToDevicestatusDb(event, 
        'GreengrassDashboard-IoTGSDynamoDeviceStatusTable-1JGCAR33OAYSP', 
         timeStampEpoch, timeStampIso, 
        locationLat, locationLon
        );
}


function getLocation(event) {
    
    if (event.deviceId == "hopper") {
        location_lat = location_lat_hopper;
        location_lon = location_lon_hopper;
    }
    else if (event.deviceId == "knuth") {
        location_lat = location_lat_knuth;
        location_lon = location_lon_knuth;
    }
    else if (event.deviceId == "turing") {
        location_lat = location_lat_turing;
        location_lon = location_lon_turing;
    }
    return {location_lat, location_lon};
}


function getTimestamp() {
    
	var currDate = new Date();
	var timeStampEpoch = currDate.getTime();
	var timeStampIso = 
		currDate.getFullYear().toString() + "-" + 
		("0" + (currDate.getMonth() + 1)).slice(-2) + "-" + 
		("0" + (currDate.getDate())).slice(-2) + "T" +
		("0" + (currDate.getHours())).slice(-2) + ":" +
		("0" + (currDate.getMinutes())).slice(-2) + ":" +
		("0" + (currDate.getSeconds())).slice(-2) + "." +
		("0" + (currDate.getMilliseconds())).slice(-3);
    return {timeStampEpoch, timeStampIso};    
}


function processEvent(event) {
    try {
        if (event.timeStampEpoch == null || event.timeStampIso == null || event.location == null)  {
            console.log('Has no timestamp and location');
            throw 'myException';
        }
        else {
            console.log('Has timestamp and location');
            addToDb(event, event.timeStampEpoch, event.timeStampIso, event.location.lat, event.location.lon);
        }
    }
    catch (e) {
    	var timeStampEpoch = 0;
    	var timeStampIso = "";
    	var timeStamp;
    	var location;
    	
        timeStamp = getTimestamp();
        location = getLocation(event);
        
        console.log('timeStampEpoch:', timeStamp.timeStampEpoch);
        console.log('timeStampIso:',  timeStamp.timeStampIso);
        console.log('location_lat:', location.location_lat);
        console.log('location_lon:', location.location_lon);

        addToDb(event, timeStamp.timeStampEpoch, timeStamp.timeStampIso, location.location_lat, location.location_lon);
    }
}


function checkThresholdsReached(event) {
    
    console.log(event.batteryCharge);
    if (event.batteryCharge >= 20 || event.batteryCharge <= -10) {
        console.log('Temperature threshold reached!');
        return true;
    }

    console.log(event.batteryDischargeRate);
    if (event.batteryDischargeRate >= 5 || event.batteryDischargeRate <= 0) {
        console.log('Humidity threshold reached!');
        return true;
    }

    console.log('Values are ok!');
    return false;
} 


function sendNotification(event) {
    
    var params = {
        Message: 'FT900 AWS IoT Demo SNS Notification: Alert, threshold reached!',
        TopicArn: 'arn:aws:sns:us-east-1:773510983687:TemperatureMaxMinThreshold'
    };
    
    var xsns = new aws.SNS({apiVersion: '2010-03-31'}).publish(params).promise();
    xsns.then(function(data) {
        console.log('Notification published successfully!');
    }).catch(function(err) {
        console.log('Notification published failed!');
    });
}


exports.handler = (event, context, callback) => {
    console.log('Received event:', JSON.stringify(event, null, 2));

    try {
        if (event.records != null) {
            //console.log('event.records.length:', event.records.length);
            for (var i=0, notified=false; i<event.records.length; i++) {
                //console.log(JSON.stringify(event.records[i], null, 2));
                processEvent(event.records[i]);
                if (notified==false) {
                    if (checkThresholdsReached(event.records[i])) {
                        sendNotification(event.records[i]);
                        notified=true;
                    }
                }
            }
        }
        else {
            console.log('single record');
            processEvent(event);
            if (checkThresholdsReached(event)) {
                sendNotification(event);
            }
        }
    }
    catch (e) {
        processEvent(event);
        if (checkThresholdsReached(event)) {
            sendNotification(event);
        }
    }
};