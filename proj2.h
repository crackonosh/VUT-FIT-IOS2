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
int init ();
void cleanup ();
void checkParameter (int parameter, char *msg);
void immigrantsGenerator (int count, int maxTime, int maxLeaveTime);
void processImmigrant (int number, int leaveTime);
void processJudge (int approvalMaxTime);
void writeToFile (const char *s, ...);
// ----- END FUNCTION HEADERS -----
