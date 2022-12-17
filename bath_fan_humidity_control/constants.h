const byte PROGMEM PR_SUCCESS								=1;
const byte PROGMEM PR_STARTED								=3;
const byte PROGMEM PR_SUCCESSFUL_LOOP_ITERATION				=0;
const byte PROGMEM PR_FORCED_REBOOT							=-3;
const byte PROGMEM PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER=-2;
const byte PROGMEM PR_ERROR									=-1;
const int PROGMEM HUMIDITY_DELTA_FOR_FINISH=1;
const int PROGMEM HUMIDITY_THRESHOLD_DEFAULT_VALUE = 50;

const char*  PROGMEM LOCATION = "home2";

// const String HOSTNAME="kitchen-humidity";
// const String HOSTNAME="bath-humidity";
const char*  PROGMEM HOSTNAME="test-humidity";

