////////////////////////////////////////////////////////////////////
//
// This javascript code queries a Microsoft Azure CosmosDB database
//
// Prerequisites:
// A. NodeJS
//    1. npm install async –g
//    2. npm install @azure/cosmos -g
//
// To run this application:
// 1. Get the access in key CosmosDB
// 2. Update the configurable parameters
// 3. Run FT90x for 1 minute to send data to IoTCore then save to CosmosDB
// 4. Run this application "node cosmosdbclient.js"
//
////////////////////////////////////////////////////////////////////

'use strict';



////////////////////////////////////////////////////////////////////
// Configure this parameters
////////////////////////////////////////////////////////////////////
const endpoint = "https://ft900iotcosmos.documents.azure.com:443/";
const masterKey = "9eAPFh6cZ4geusIykcyvz1Epy2y95A7e1hqSk13XFdgvEeHoUF92g5YjvJHFhC3EIVBjG0QQnFwfJ5FfsoHcbA==";
const databaseId = "timeseriestable";
const containerId = "mycollections";
const timeSpan  = 1000 * 60 * 1; // 1 minute of data only
////////////////////////////////////////////////////////////////////


const cosmos = require('@azure/cosmos');
const client = new cosmos.CosmosClient({ endpoint, auth: { masterKey } });

async function asyncQuery(timeRange, deviceId) 
{
    const { database } = await client.databases.createIfNotExists({ id: databaseId });
    const { container } = await database.containers.createIfNotExists({ id: containerId });
    const querySQL = getSQLQueryString(container.id, timeRange, deviceId);

    const querySpec = { query: querySQL };
    const { result: results } = await container.items.query(querySpec).toArray();
    console.log("items=" + results.length);

    for (var i=0; i<results.length; i++) {
        console.log("{ " + i);
        console.log("  deviceId="             + results[i].deviceId);
        console.log("  timeStampEpoch="       + results[i].timeStampEpoch);
        console.log("  timeStampIso="         + results[i].timeStampIso);
        console.log("  sensorReading="        + results[i].sensorReading);
        console.log("  batteryCharge="        + results[i].batteryCharge);
        console.log("  batteryDischargeRate=" + results[i].batteryDischargeRate);
        console.log("}");
    }
}

function getSQLQueryString(cosmosDBId, timeRange, deviceId)
{
    var sqlQuery = "SELECT * FROM " + cosmosDBId 
        + " WHERE " + cosmosDBId +".deviceId='"+ deviceId 
        + "' AND " + cosmosDBId +".timeStampEpoch>" + timeRange;

    console.log('getSQLQueryString(): ' + sqlQuery + '\n');
    return sqlQuery;
}

function getTimeRange(timeSpan)
{
    var timeNow = Date.now();
    var timeRange = timeNow-timeSpan;

    console.log('getTimeRange(): ' + timeRange + '\n');
    return timeRange;
}

async function handleError(error) 
{
    console.log("\nAn error with code '" + error.code + "' has occurred:");
    console.log("\t" + error.body || error);
}


////////////////////////////////////////////////////////////////////
var timeRange = getTimeRange(timeSpan);
asyncQuery(timeRange, 'hopper').catch(handleError);
//asyncQuery(timeRange, 'turing').catch(handleError);
//asyncQuery(timeRange, 'knuth').catch(handleError);
////////////////////////////////////////////////////////////////////
