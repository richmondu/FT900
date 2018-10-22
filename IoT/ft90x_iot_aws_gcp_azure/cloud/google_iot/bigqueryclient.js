////////////////////////////////////////////////////////////////////
//
// This javascript code queries a Google Cloud BigQuery database
//
// Prerequisites:
// A. NodeJS
//    1. npm install --save @google-cloud/bigquery
// B. Google Cloud
//    1. gcloud iam service-accounts create bigqueryclient
//    2. gcloud projects add-iam-policy-binding ft900iotproject --member "serviceAccount:bigqueryclient@ft900iotproject.iam.gserviceaccount.com" --role "roles/bigquery.admin"
//    3. gcloud iam service-accounts keys create bigqueryclient.json --iam-account bigqueryclient@ft900iotproject.iam.gserviceaccount.com
// C. Save credentials in Cloud Storage 
//    1. Upload bigqueryclient.json to your Google Storage bucket ft900iotbucket
//    2. Make it public "gsutil acl ch -u AllUsers:R gs://ft900iotbucket/bigqueryclient.json"
//    3. Verify you can access it via https://storage.googleapis.com/ft900iotbucket/bigqueryclient.json
// To run this application:
// 1. Update the configurable parameters
// 2. Run FT90x for 1 minute to send data to IoTCore then save to BigQuery
// 3. Run this application "node bigqueryclient.js"
//
////////////////////////////////////////////////////////////////////

'use strict';



////////////////////////////////////////////////////////////////////
// Configure this parameters
////////////////////////////////////////////////////////////////////
const projectId = "ft900iotproject";
const dataSetId = "dataset";
const tableId   = "timeseriestable";
//const credentialPath = "C:\\Users\\richmond\\Desktop\\bigquery\\bigqueryclient.json";
//const credentialPath = "gs://ft900iotbucket/bigqueryclient.json";
//const credentialPath = "https://storage.googleapis.com/ft900iotbucket/bigqueryclient.json";
const credentialPrivateKey = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCcY0NPOBLjNpTT\nW41Gzp/duhq017XCMv0GqgKlewnDUd5KCwNBpxn6XPBncT3EPDiwaccnf9jP108w\nqgoWNPGmYt0+6iz4ozTvEsActjaWbWkshm2fVSZ36lr0pj1VZQf/buVrAwtxq8Um\nByrh+GLiG4iUcpIIE4rEi8FYasEB4v8zLYRNxoucPXSfsJx3qh8XDkg3SZeNgIRJ\ntVNmzb6hIl7ne0KfuOF0d8yazCOPZArVdDaXcrl2Ww04weyTcpuJLd0b294SrKSa\nGk4NHOwooyIA2bYQ391Un7cgbtXirKpJ+3wbM6JOh5IbqKxYTX4htnRdYRnrW4+H\n7+KhbwXpAgMBAAECggEABKFCsaJ3Gq93vf8ooldFL98N4ds+CBMS3yLi46vOGPCA\nTIBhdwzfrcQZIPLfEYN36S8khISaMmMNNuNd25TzL4ofQzjTHoRUcZyEQAVb/d2c\n3sLXFX/DnQq86R/P0oqkbzlFBQNI0DN16X+OUzTTlp6aj0yKCr+rXNDHZ1r3vlrC\neuCKvMO4lHeMD+pP33BfdLnkH6AgO9JiCUmBWC+p44jMGc5CxGWQlE7Quwvc/Vtw\nadtDf4THCL65dm4kGmdfZL7dWKFrf5qxqzaqQPWwCVzPyDYa6LjFPUtCmPOEn1iM\nnXUwM7wVP7c223YJECsy2VQEmPmG8TwJePQOa0w2JQKBgQDJekD7yiQhqMI2GIjD\nGpED3lTH+KlIxb3nUbj7cT40bBLSyZDXFkWzlGEOmkeIAgA4YOPal2ZCWA7dwaT7\nGBLeIgl6yfLHUYpYKBl+Rp+68JO5u1dyunh92+exGiP8mCOaMMO28LbXpz3RNeQI\ncPx1tR5/rlbARcPD9HL4CDQ6jQKBgQDGtVNd47n8aiVx/X20hNAoCjPMPatxQcp7\nf7+RKehTp/2NbDVadnCCsp2lxqYumvTSx3dG0bvXiCWQpyi/Z3Q3soNFZHI5rs05\n4wAOqYCn/jDV4QzRqNf9h38SbqSgHMMOcsQrCQCraOrgtJFpISvjk29cSr+M1rYD\n+4Ut+V1vzQKBgQDBnd69GXDHd+nLGqCy/eDGGu7WjTwBVyY8GejumdDxh7o/Hr02\nNY0eGkV7Rv/e0C1uAI53Dm+S6hS/PsxREL1xAryhZ1kL+mw8c6ysESqG95K/Ni2/\nbztDmAR7yWcz4vCzyJEMSR8s9ucNGgDdq9axNzDr+pobg++xbETf3uJyMQKBgB7B\nskW2GfOX+nPKaMqoRA/uhFX4tbCyHkU/YMd3VR6dwgFB3oBCFOgl/YDLXq2YDo0R\nksTDce/KgwYA2a3GtZZpaygyg/upstpi6H9cfS2DisuQKgvJtqTndCPGPHUL/qUd\n3n9KxW/CrZhaWqeLGt/+0h3W9A84rLOxGswi7POBAoGBAIpCeumGPkjfZbLGgRnq\nqrENE7XLxmV0xVGm78Zd2bhDSge79L/MgdM4rACXageIgL5Cnr2yLfWGN7AyxGUI\n6eci6BXoEVn3NWdxLbA5bS62+uzev2BLOAofMG7LhSw0Goxqx6UtfY7NIsibNRQi\nT+kn75qPaHsw7+gsvUECnRuY\n-----END PRIVATE KEY-----\n";
const credentialEmail = "bigqueryclient4@ft900iotproject.iam.gserviceaccount.com";
const timeSpan  = 1000 * 60 * 1; // 1 minute of data only
const maxItems  = 60;
////////////////////////////////////////////////////////////////////


const bigQueryId = projectId + "." + dataSetId + "." + tableId;

async function asyncQuery(sqlQuery, projectId) 
{
    const BigQuery = require('@google-cloud/bigquery');
    const bq = new BigQuery({
        projectId: projectId, 
        // Notes:
        // keyFilename seem to work only when file is located locally; does not seem to work when save in cloud storage
        // so use credentials instead. credentials data can be found inside the keyFilename
        // keyFilename: credentialPath 
        credentials: {
            private_key: credentialPrivateKey,
            client_email: credentialEmail
        }
    });
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


