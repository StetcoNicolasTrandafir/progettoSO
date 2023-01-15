#ifndef _MACRO_H
#define _MACRO_H

#define SO_NAVI atoi(getenv("SO_NAVI"))
#define SO_PORTI atoi(getenv("SO_PORTI"))
#define SO_MERCI atoi(getenv("SO_MERCI"))
#define SO_MIN_VITA atoi(getenv("SO_MIN_VITA"))
#define SO_MAX_VITA atoi(getenv("SO_MAX_VITA"))
#define SO_SIZE atoi(getenv("SO_SIZE"))
#define SO_LATO atoi(getenv("SO_LATO"))
#define SO_SPEED (double)atoi(getenv("SO_SPEED"))
#define SO_CAPACITY atoi(getenv("SO_CAPACITY"))
#define SO_BANCHINE atoi(getenv("SO_BANCHINE"))
#define SO_FILL atoi(getenv("SO_FILL"))
#define SO_LOADSPEED atoi(getenv("SO_LOADSPEED"))
#define SO_DAYS atoi(getenv("SO_DAYS"))
#define SO_DISTANZA_PORTI 3
#define SO_STORM_DURATION atoi(getenv("SO_STORM_DURATION"))
#define SO_SWELL_DURATION atoi(getenv("SO_SWELL_DURATION"))
#define SO_MEALSTROM atoi(getenv("SO_MEALSTROM"))


#define TEST_ERROR    if (errno && errno != EINTR) {fprintf(stderr, \
					  "\x1b[31m%s at line %d: PID=%5d, Error %d: %s\n\x1b[37m", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno)); \
					  errno=0;\
					  }
#define SWELL 3
#define REQUEST 2
#define OFFER 1
#define DOCK 0


#define GOODS 0
#define COORDS 1
#define INDOCK 2 
#define STORM 3


#endif /*_MACRO_H*/


