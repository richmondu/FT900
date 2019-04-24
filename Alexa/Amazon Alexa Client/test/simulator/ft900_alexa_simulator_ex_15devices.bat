SETLOCAL
set START_ID=2
set NUM_DEVICES=15


for /l %%x in (%START_ID%, 1, %NUM_DEVICES%) do (
   START python.exe ft900_alexa_simulator_ex.py --deviceid %%x
)

ENDLOCAL
