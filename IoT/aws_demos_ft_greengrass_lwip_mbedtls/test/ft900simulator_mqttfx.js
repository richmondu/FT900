////////////////////////////////////////////////////////////////////////////////////////////////////////
// This test application simulates the FT900 device for the FT900 AWS IoT demo
// It can be used to test and troubleshoot the backend cloud service
// This javascript code can be tested using MQTT.FX application
// MQTT.FX can be downloaded at http://mqttfx.jensd.de/
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Below are the necessary settings for the MQTT.FX
// AWS Greengrass scenario:
//   Broker Address: 192.168.22.16
//   Broker Port: 8883
//   Client ID: HelloWorld_Publisher
//   SSL/TLS settings:
//     CA File: Greengrass group certificate (expires every 7 days, configurable to 30 days max)
//     Client Certificate File: Greengrass device certificate corresponding to HelloWorld_Publisher
//     Client Key File: Greengrass device certificate private key corresponding to HelloWorld_Publisher
// AWS IoT scenario:
//   Broker Address: amasgua12bmkv.iot.us-east-1.amazonaws.com
//   Broker Port: 8883
//   Client ID: HelloWorld_Publisher
//   SSL/TLS settings:
//     CA File: AWS IoT CA certificate
//     Client Certificate File: Greengrass device certificate corresponding to HelloWorld_Publisher
//     Client Key File: Greengrass device certificate private key corresponding to HelloWorld_Publisher
////////////////////////////////////////////////////////////////////////////////////////////////////////

var Thread = Java.type("java.lang.Thread");

function execute(action) 
{
    var devices = ["hopper", "knuth", "turing"];
    
    for (;;) 
    {
        for each(var device in devices) 
        {
            publish(device);
            Thread.sleep(1000);
        }
    }
    
    action.setExitCode(0);
    action.setResultText("done.");
    
    return action;
}

function publish(deviceId) 
{
    var topic = generateTopic(deviceId);
    var message = generateMessage(deviceId);
    //var message = generateMessageEx(deviceId);
    
    mqttManager.publish(topic, message);
    
    output.print("MQTT app published to " + topic + "\n" + message + "\n");
}

function generateTopic(deviceId)
{
    return "device/" + deviceId + "/devicePayload";
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generate message without timestamp and geolocation
// Note that if device does not add timestamp and geolocation,
// the Greengrass core will add it via a lambda function
// If there are no Greengrass, AWS IoT will add it via a lambda function too.
////////////////////////////////////////////////////////////////////////////////////////////////////////
function generateMessage(deviceId) 
{
    var message = "{\n";    
    message += "\t" + "\"deviceId\": \""           + deviceId + "\",\n";
    message += "\t" + "\"sensorReading\": "        + (Math.random() % 10 + 30).toFixed(2) + ",\n";
    message += "\t" + "\"batteryCharge\": "        + (Math.random() % 30 - 10).toFixed(2) + ",\n";
    message += "\t" + "\"batteryDischargeRate\": " + (Math.random() % 5).toFixed(2) + "\n";
    message += "}";
    
    return message;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generate message with timestamp and geolocation
////////////////////////////////////////////////////////////////////////////////////////////////////////
function generateMessageEx(deviceId) 
{
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
    
    var message = "{\n";    
    message += "\t" + "\"deviceId\": \""           + deviceId + "\",\n";
    message += "\t" + "\"sensorReading\": "        + (Math.random() % 10 + 30).toFixed(2) + ",\n";
    message += "\t" + "\"batteryCharge\": "        + (Math.random() % 30 - 10).toFixed(2) + ",\n";
    message += "\t" + "\"batteryDischargeRate\": " + (Math.random() % 5).toFixed(2) + ",\n";
    message += "\t" + "\"timeStampIso\": \""       + timeStampIso + "\",\n";
    message += "\t" + "\"timeStampEpoch\": "       + timeStampEpoch + ",\n";
    message += "\t" + "\"location\": {\n";
    message += "\t" + "  \"lat\": "                + 37.032685262365355 + ",\n";
    message += "\t" + "  \"lon\": "                + -98.61302669373025 + "\n";    
    message += "\t" + "}\n";
    message += "}";
    
    return message;
}
