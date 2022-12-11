const byte PR_SUCCESS								=1;
const byte PR_STARTED								=3;
const byte PR_SUCCESSFUL_LOOP_ITERATION				=0;
const byte PR_FORCED_REBOOT							=-3;
const byte PR_INFLUXDB_COULDNT_SEND_METRIC_TO_SERVER=-2;
const byte PR_ERROR									=-1;
const int HUMIDITY_DELTA_FOR_FINISH=1;
int HUMIDITY_THRESHOLD = 50;

const String HOSTNAME="kitchen-humidity";

const char* LOCATION = "home2";
// const String HOSTNAME="bath-humidity";
// const String HOSTNAME="test-humidity";

