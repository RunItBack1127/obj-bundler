#include <stdbool.h>

#define INPUT_FILE_EXT "obj"
#define OUTPUT_FILE_EXT "objb"
#define MTL_FILE_EXT "mtl"
#define MIN_MTL_SPACE 3
#define MAX_MTL_SPACE 10

#define LINE_BUFFER_LEN 1000
#define DEFAULT_DATA_LIMIT 10000000
#define MAX_DATA_LIMIT 99999999
#define VF_BUFFER_LEN 4
#define VTVPVN_BUFFER_LEN 3
#define VALUE_BUFFER_LEN 50
#define L_BUFFER_LEN 6
#define MTL_FILENAME_BUFFER_LEN 50
#define MTL_NAME_BUFFER_LEN 100
#define HEADER_COMMENT_BUFFER_LEN 500
#define OBJ_GROUP_BUFFER_LEN 1000
#define GROUP_NAME_BUFFER_LEN 100
#define SHADING_BUFFER_LEN 3

typedef struct OBJGroup {
	char *name;
	
	double **data_v;
	double **data_vt;
	double **data_vp;
	double **data_vn;
	char ***data_f;
	char *data_mtl;
	char *shading;
	int **data_l;
	
	int v_index;
	int vt_index;
	int vp_index;
	int vn_index;
	int f_index;
	int l_index;
} OBJGroup;

void _BUNDLER_HELP();
void _BUNDLER_ERROR( short error, char *payload );
bool _verify_file_extension( char *filename, char *ext );
OBJGroup *_construct_obj_group();