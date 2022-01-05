#include <stdbool.h>

typedef struct OBJGroup {
	char *name;
	
	double **data_v;
	double **data_vt;
	double **data_vp;
	double **data_vn;
	char ***data_f;
	char **data_mtllib;
	char **data_mtls;
	char *shading;
	int **data_l;
	
	int v_index;
	int vt_index;
	int vp_index;
	int vn_index;
	int f_index;
	int mtllib_index;
	int mtls_index;
	int l_index;
} OBJGroup;

void _BUNDLER_HELP();
void _BUNDLER_ERROR( short error, char *payload );
bool _verify_file_extension( char *filename, char *ext );
OBJGroup *_construct_obj_group();