#pragma once

// Heartbeat
#define VP_PING 0x40
#define VP_PONG 0x41
#define VP_ACK  0x42

// Telemetry request
#define VP_REQUEST_DATA 0x52

// Sensor values
#define VP_FLOW     0x61
#define VP_PRESSURE 0x62
#define VP_TDS      0x63
#define VP_TEMP     0x64
#define VP_LEVEL    0x72

// Commands
#define VP_HDP        0x65
#define VP_OSMOSE     0x66
#define VP_HEIZUNG    0x67
#define VP_REINIGER   0x55
#define VP_CHEM_PUMP  0x56
#define VP_TANK_FILL  0x45
#define VP_TANK_EMPTY 0x46
#define VP_PIPE_FLUSH 0x47
#define VP_WATER_SRC  0x68