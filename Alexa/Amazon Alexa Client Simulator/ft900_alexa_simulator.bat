:: deviceid should be in the range of [1, 16]
:: when usediffacct is 1,
::   deviceid 1 connects to port CONF_SERVER_PORT_DEFAULT + 0
::   deviceid 2 connects to port CONF_SERVER_PORT_DEFAULT + 1
::   deviceid X connects to port CONF_SERVER_PORT_DEFAULT + X-1

python.exe ft900_alexa_simulator.py --serveraddr 192.168.100.12 --deviceid 2 --usediffacct 0

pause

