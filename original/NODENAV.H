
struct HEADERTYPE {
	char type[4];
	unsigned long num_dir_entries;
	unsigned long dirpointer;
};

struct DIRENTRYTYPE {
	unsigned long startaddr;
	unsigned long length;
	char name[8];
};

struct LINEDEFTYPE {
	short int from_vertex;
	short int to_vertex;
	short int attributes;
	short int type;
	short int sector_trigger;
	short int right_sidedef;
	short int left_sidedef;
};

struct SIDEDEFTYPE {
	short int	u_offset;	
	short int	v_offset;
	char	uppertxt[8];
	char	lowertxt[8];
	char	walltxt[8];
	short int	sector;
};

struct VERTEXTYPE {
	short int x;
	short int y;
};

struct SEGMENTTYPE {
	short int from_vertex;
	short int to_vertex;
	short int angle;
	short int linedef;
	short int side;
	short int distance;
};

struct SSECTORTYPE {
	short int startseg;
	short int numsegs;
};

struct NODETYPE {
	short int x;
	short int y;
	short int dx;
	short int dy;

	short int left_y_upper;
	short int left_y_lower;
	short int left_x_upper;
	short int left_x_lower;

	short int right_y_upper;
	short int right_y_lower;
	short int right_x_upper;
	short int right_x_lower;

	unsigned short int left_child;
	unsigned short int right_child;
};

struct SECTORTYPE {
	short int floor_alt;
	short int ceiling_alt;
	char floortxt[8];
	char ceiltxt[8];
	short int brightness;
	short int special;
	short int trigger;
};



