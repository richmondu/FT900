////////////////////////////////////////////////////////////////////
//
// This javascript code queries a Google Cloud BigQuery database
//
// Prerequisites:
// A. Google Cloud
//    1. gcloud iam service-accounts create bigqueryclient
//    2. gcloud projects add-iam-policy-binding ft900iotproject --member "serviceAccount:bigqueryclient@ft900iotproject.iam.gserviceaccount.com" --role "roles/owner"
//    3. gcloud iam service-accounts keys create bigqueryclient.json --iam-account bigqueryclient@ft900iotproject.iam.gserviceaccount.com
// B. NodeJS
//    1. npm install --save @google-cloud/bigquery
//
// To run this application:
// 1. Run FT90x for 1 minute to send data to IoTCore then save to BigQuery
// 2. Run this application "node bigquery.js"
//
////////////////////////////////////////////////////////////////////

'use strict';



////////////////////////////////////////////////////////////////////
// Configure this parameters
////////////////////////////////////////////////////////////////////
const projectId = "ft900iotproject";
const dataSetId = "dataset";
const tableId   = "timeseriestable";
const accessKey = "C:\\Users\\richmond\\Desktop\\bigquery\\bigqueryclient2.json"
const timeSpan  = 1000 * 60 * 5; // 5 minutes
const maxItems  = 25;
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

function getTimeRange(timeSpan)
{
    var timeNow = Date.now();
    var timeRange = timeNow-timeSpan;

    console.log('getTimeRange(): ' + timeRange + '\n');
    return timeRange;
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


////////////////////////////////////////////////////////////////////
var timeRange = getTimeRange(timeSpan)
asyncQuery(getSQLQueryString(bigQueryId, timeRange, maxItems, 'hopper'), projectId)
//asyncQuery(getSQLQueryString(bigQueryId, timeRange, maxItems, 'turing'), projectId)
//asyncQuery(getSQLQueryString(bigQueryId, timeRange, maxItems, 'knuth'), projectId)
////////////////////////////////////////////////////////////////////


