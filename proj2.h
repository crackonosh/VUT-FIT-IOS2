// ----- GLOBAL VARIABLES -----
FILE *oFile = NULL;
struct timespec ts;

int *outputCount = NULL;        // A
int *immRegistered = NULL;      // NE
int *immNotAllowed = NULL;      // NC
int *immInBuilding = NULL;      // NB
int *remainingImmigrants = NULL; // used for judge
// ----- END GLOBAL VARIABLES -----

// ----- SEMAPHORES -----
sem_t *registered = NULL; // immigrants can take out their confirmation
sem_t *judgeInBuilding = NULL; // if judge inside, immigrants cannot leave 
sem_t *fileWrite = NULL;
// ----- END SEMAPHORES -----

// ----- FUNCTION HEADERS -----
/**
 * Opens file for writing, initializes global variables and creates semaphores
 * 
 * @return int
 */
int init ();
/**
 * Cleans up everything init created
 * 
 * @return void
 */
void cleanup ();
/**
 * Function checks if parameter is within the allowed range.
 * Otherwise prints error msg to stderr.
 * 
 * @param int parameter
 * @param char *msg
 * @return void
 */
void checkParameter (int parameter, char *msg);
/**
 * Generates "count" number of immigrants every rand<0,maxTime>
 * 
 * @param int count
 * @param int maxTime
 * @return void
 */
void immigrantsGenerator (int count, int maxTime, int maxLeaveTime);
/**
 * Performs all expected actions for immigrant process with number
 * 
 * @param int number
 * @return void
 */
void processImmigrant (int number, int leaveTime);
/**
 * Performs Judge's tasks
 * 
 * @param approvalMaxTime
 * @return void
 */
void processJudge (int approvalMaxTime);
/**
 * Function writes formatted string with args to file
 * 
 * @param *s formatted string
 * @return void
 */
void writeToFile (const char *s, ...);
// ----- END FUNCTION HEADERS -----
