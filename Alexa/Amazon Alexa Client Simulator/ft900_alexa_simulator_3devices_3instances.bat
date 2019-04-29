SETLOCAL

set START_ID=2
set NUM_DEVICES=3
set SERVER_ADDR=192.168.100.12
set USE_DIFFERENT_ACCOUNT=1

set /a END_ID=%NUM_DEVICES%+1
for /l %%x in (%START_ID%, 1, %END_ID%) do (
   START python.exe ft900_alexa_simulator.py --serveraddr %SERVER_ADDR% --deviceid %%x --usediffacct %USE_DIFFERENT_ACCOUNT%
)

ENDLOCAL
