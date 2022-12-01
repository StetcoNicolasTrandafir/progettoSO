typedef struct goods_t {
	int type;
	int dim; /*in tonnellate, estratto casualmente tra 1 e SO_SIZE*/
	int life_time; /*in giorni, estratto casualmente tra SO_MIN_VITA e SO_MAX_VITA*/
}goods_t;

typedef struct ship_t {
	int speed; /*in km, uguale a SO_SPEED (uguale per tutte le navi)*/
	coordinates_t coord; /*una coppia di coordinate (x, y)*/
	int capacity; /*in tonnellate, uguale a SO_CAPACITY*/
}ship_t;

typedef struct coordinates_t {
	double x;
	double y;
}coordinates_t;

typedef struct port_t {
	coordinates_t coord; /*una coppia di coordinate (x, y)*/
	int docks; /*numero di banchine, estratto casualmente tra 1 e SO_BANCHINE*/
}port_t;