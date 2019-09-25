import greengrasssdk
import platform
import logging
import json
import datetime
from ctypes import c_longlong as ll



timeout_sec = 10

client = greengrasssdk.client('iot-data')
my_platform = platform.platform()
logger = logging.getLogger()

records_id = "records"
records = dict()
start_time = dict()

location_lat = 37.26;
location_lon = -119.62;
location_lat_hopper = location_lat;
location_lon_hopper = location_lon + 3;
location_lat_knuth = location_lat + 3;
location_lon_knuth = location_lon + 40;
location_lat_turing = location_lat + 10;
location_lon_turing = location_lon + 20;



def compute_timeiso_value():
	
	str = datetime.datetime.now().strftime("%Y-%m-%dT%H:%M:%S.000")
	logger.info(str)
	return str
	
	
def compute_timeepoch_value(iso):
	
	epoch = datetime.datetime(1970, 1, 1, 0, 0, 0)
	
	end = iso[0:].index("-")
	year = int(iso[0:end])
	logger.info("year")
	logger.info(year)
	
	iso = iso[end+1:]
	end = iso[0:].index("-")
	month = int(iso[0:end])
	logger.info("month")
	logger.info(month)
	
	iso = iso[end+1:]
	end = iso[0:].index("T")
	day = int(iso[0:end])
	logger.info("day")
	logger.info(day)
	
	iso = iso[end+1:]
	end = iso[0:].index(":")
	hour = int(iso[0:end])
	logger.info("hour")
	logger.info(hour)
	
	iso = iso[end+1:]
	end = iso[0:].index(":")
	minute = int(iso[0:end])
	logger.info("minute")
	logger.info(minute)
	
	iso = iso[end+1:]
	end = iso[0:].index(".")
	second = int(iso[0:end])
	logger.info("second")
	logger.info(second)
	
	curr  = datetime.datetime(year, month, day, hour, minute, second)
	delta = int((curr-epoch).total_seconds() * 1000)
	logger.info(delta)
	return delta
	
	
def enrich_data(event):
	
    # add the geo location if not set
	if event.get("location") == None:
		if event["deviceId"] == "hopper":
			event.update({"location": {"lat": location_lat_hopper, "lon": location_lon_hopper}})
		elif event["deviceId"] == "turing":
			event.update({"location": {"lat": location_lat_turing, "lon": location_lon_turing}})
		elif event["deviceId"] == "knuth":
			event.update({"location": {"lat": location_lat_knuth, "lon": location_lon_knuth}})

	# compute the timestamp iso if not set
	if event.get("timeStampIso") == None:
		event["timeStampIso"] = compute_timeiso_value()

	# correct the timeStampEpoch based on the timeStampIso
	if event.get("timeStampEpoch") == None:
		event["timeStampEpoch"] = compute_timeepoch_value(event["timeStampIso"])


def enrich_and_publish(event):

	# enrich the data with additional information:
	# timestamps, geolocation
	enrich_data(event)

	# publish the message to IoT cloud
	client.publish(
		topic="device/" + event["deviceId"] + "/devicePayload", 
		payload=json.dumps(event)
	)
	logger.info("Published done:\n" + json.dumps(event) + "\n")


def enrich_and_process(event):
	
	# enrich the data with additional information:
	# timestamps, geolocation
	enrich_data(event)

	if (records.get(event["deviceId"]) == None):
		records[event["deviceId"]] = dict()
		records[event["deviceId"]]["records"] = list()
		
	if (len(records[event["deviceId"]]["records"]) == 0):
		start_time[event["deviceId"]] = datetime.datetime.now()

	# append packet
	records[event["deviceId"]]["records"].append(event)
	logger.info(len(records[event["deviceId"]]["records"]))
	logger.info("New record:\n" + json.dumps(records[event["deviceId"]]) + "\n")

	# send packet only after timeout_sec seconds
	diff_time = datetime.datetime.now() - start_time[event["deviceId"]]
	if (diff_time.seconds > timeout_sec):
		# publish the message to IoT cloud
		client.publish(
			topic="device/" + event["deviceId"] + "/devicePayload", 
			payload=json.dumps(records[event["deviceId"]])
		)
		logger.info("Published done:\n" + json.dumps(records[event["deviceId"]]) + "\n")
		logger.info(len(records[event["deviceId"]]["records"]))
		# reset the list of packets
		records[event["deviceId"]]["records"] = []
		
		
def function_handler(event, context):

	logger.setLevel(logging.INFO)	
	logger.info(json.dumps(event))

	# enrich_and_publish(event)
	enrich_and_process(event)

	return


