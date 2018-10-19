////////////////////////////////////////////////////////////////////
//
// This javascript code queries a Google Cloud BigQuery database
//
// Prerequisites:
// A. NodeJS
//    1. npm install --save @google-cloud/bigquery
// B. Google Cloud
//    1. gcloud iam service-accounts create bigqueryclient
//    2. gcloud projects add-iam-policy-binding ft900iotproject --member "serviceAccount:bigqueryclient@ft900iotproject.iam.gserviceaccount.com" --role "roles/owner"
//    3. gcloud iam service-accounts keys create bigqueryclient.json --iam-account bigqueryclient@ft900iotproject.iam.gserviceaccount.com
//
// To run this application:
// 1. Set GOOGLE_APPLICATION_CREDENTIALS=C:\Users\richmond\Desktop\bigquery\bigqueryclient.json
// 2. Update the configurable parameters
// 3. Run FT90x for 1 minute to send data to IoTCore then save to BigQuery
// 4. Run this application "node bigqueryclient.js"
//
////////////////////////////////////////////////////////////////////

'use strict';



////////////////////////////////////////////////////////////////////
// Configure this parameters
////////////////////////////////////////////////////////////////////
const projectId = "ft900iotproject";
const dataSetId = "dataset";
const tableId   = "timeseriestable";
const timeSpan  = 1000 * 60 * 1; // 1 minute of data only
const maxItems  = 60;
////////////////////////////////////////////////////////////////////


const bigQueryId = projectId + "." + dataSetId + "." + tableId;

async function asyncQuery(sqlQuery, projectId) 
{
    const BigQuery = require('@google-cloud/bigquery');
    const bq = new BigQuery({projectId});
    const options = {query: sqlQuery, useLegacySql: false};

    const [job] = await bq.createQueryJob(options);
    console.log(`Job ${job.id} started.`);

    const metadata = await job.getMetadata();
    const errors = metadata[0].status.errors;
    if (errors && errors.length > 0) 
    {
        throw errors;
    }
    console.log(`Job ${job.id} completed.`);

    const [rows] = await job.getQueryResults();

    rows.forEach(row => console.log(row));
}

function getSQLQueryString(bigQueryId, timeRange, maxItems, deviceId)
{
    var sqlQuery = "SELECT * FROM `" + bigQueryId 
        + "` WHERE deviceId='"+ deviceId 
        + "' AND timeStampEpoch > " + timeRange 
        + " LIMIT " + maxItems;

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
asyncQuery(getSQLQueryString(bigQueryId, timeRange, maxItems, 'hopper'), projectId).catch(handleError);
//asyncQuery(getSQLQueryString(bigQueryId, timeRange, maxItems, 'turing'), projectId).catch(handleError);
//asyncQuery(getSQLQueryString(bigQueryId, timeRange, maxItems, 'knuth'), projectId).catch(handleError);
////////////////////////////////////////////////////////////////////


