'use strict';
console.log('Loading function');
let response = require('cfn-response');
let doc = require('dynamodb-doc');
let dynamo = new doc.DynamoDB();



exports.handler = (event, context, callback) => {
   console.log('Received event:', JSON.stringify(event, null, 2));
   
   if (event.deviceId) {
      
      let timestampRange = (new Date()).getTime() - (300 * 1000);
      let params = {
          TableName: 'GreengrassDashboard-IoTGSDynamoTimeSeriesTable-WT6JJZXKXR0J',
          KeyConditionExpression: 'deviceId = :hkey and payloadTimestamp > :rkey',
          ExpressionAttributeValues: {
              ':hkey': event.deviceId,
              ':rkey': timestampRange
          }
      };
      
      dynamo.query(params, function(err, data) {
         if (err) {
            console.log(err, err.stack);
            callback(null, {});
         }
         else {   
            console.log(data);
            callback(null, data);
         }
      });
   }
   else {
      
      let params = {
         TableName: 'GreengrassDashboard-IoTGSDynamoDeviceStatusTable-1JGCAR33OAYSP',
      };
      
      dynamo.scan(params, function(err, data) {
         if (err) {
            console.log(err, err.stack);
            callback(null, {});
         }
         else {   
            console.log(data);
            callback(null, data);
         }
      });
   }
};
