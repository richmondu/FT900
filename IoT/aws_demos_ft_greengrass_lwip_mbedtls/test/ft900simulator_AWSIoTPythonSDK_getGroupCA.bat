:: AWS IoT broker
set AWS_IOT_CORE=amasgua12bmkv.iot.us-east-1.amazonaws.com
set AWS_IOT_CACERT=certificates/root-ca-cert.pem

:: Device certificates
set GREENGRASS_DEVICE_CERT=certificates/publisher.cert.pem
set GREENGRASS_DEVICE_KEY=certificates/publisher.private.key
set GREENGRASS_DEVICE_CLIENTID=HelloWorld_Publisher

:: AWS Greengrass broker
:: Do not modify; these are just for the output
:: Executing this batch file will produce group-ca-cert.pem in certificates/ folder
set GREENGRASS_CACERT_FILEPATH=certificates/
set GREENGRASS_CACERT_FILENAME=group-ca-cert.pem


python ft900simulator_AWSIoTPythonSDK_getGroupCA.py --endpoint %AWS_IOT_CORE% --rootCA %AWS_IOT_CACERT% --cert %GREENGRASS_DEVICE_CERT% --key %GREENGRASS_DEVICE_KEY% --thingName %GREENGRASS_DEVICE_CLIENTID% --filePath %GREENGRASS_CACERT_FILEPATH% --fileName %GREENGRASS_CACERT_FILENAME%
pause