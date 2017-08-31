

typedef enum {
	A;
	B;
	Z;
	START;
	UP;  //up on dpad
	DOWN; //down on dpad
	LEFT; //left on dpad
	RIGHT;  //right on dpad
	L; //left bumper
	R; //right bumper
	C-UP;
	C-DOWN;
	C-LEFT;
	C-RIGHT;
} buttons_t;


typedef struct {
	bool a;
	bool b;
	bool z;
	bool start;
	bool up;
	bool down;
	bool left;
	bool right;
	bool l; 
	bool r;
	bool c_up;
	bool c_down;
	bool c_left;
	bool c_right;
	char analogz;
	char analogx;	
} controller_t;


// initialize GPIO pins for reading from shift register
int Controller_Init();

int Request();

// the Poll function to write shift register vals into the controller struct
int Poll(controller_t *controller);





