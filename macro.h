#define SO_NAVI getenv("SO_NAVI")
#define SO_PORTI getenv("SO_PORTI")
#define SO_MERCI getenv("SO_MERCI")
#define SO_MIN_VITA getenv("SO_MIN_VITA")
#define SO_MAX_VITA getenv("SO_MAX_VITA")
#define SO_SIZE getenv("SO_SIZE")
#define SO_LATO atoi(getenv("SO_LATO"))
#define SO_SPEED getenv("SO_SPEED")
#define SO_CAPACITY getenv("SO_CAPACITY")
#define SO_BANCHINE getenv("SO_BANCHINE")
#define SO_FILL getenv("SO_FILL")
#define SO_LOADSPEED getenv("SO_LOADSPEED")
#define SO_DAYS getenv("SO_DAYS")
#define SO_STORMDURATION getenv("SO_STORMDURATION")
#define SO_SWELLDURATION getenv("SSO_SWELLDURATIONO_NAVI")
#define SO_MALESTROM getenv("SO_MALESTROM")

#define PRINT_ERROR fprintf(stderr,				\
			    "%s Error number %3d at line %d:\n \"%s\"\n",	\
			    __FILE__, errno, __LINE__, strerror(errno));

