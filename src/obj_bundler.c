#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define ARG__INPUT_FILENAME "--in"
#define ARG__OUTPUT_FILENAME "--out"
#define ARG__PRESERVE_HEADER_COMMENTS "--preserve-header-comments"
#define ARG__PRESERVE_GROUP_NAMES "--preserve-group-names"
#define ARG__INCLUDE_W_COORDS "--include-w-coords"
#define ARG__MTL_TAB_SPACE "--mtl-tab-space="
#define ARG__SET_DATA_LIMIT "--set-data-limit="
#define ARG__HELP "--help"

#define _ERROR_NO_INPUT_FILE_PROVIDED 100
#define _ERROR_NO_OUTPUT_FILE_PROVIDED 101
#define _ERROR_NO_MTL_SPACE_PROVIDED 102
#define _ERROR_NO_DATA_LIMIT_PROVIDED 103
#define _ERROR_INVALID_INPUT_FILE_PROVIDED 104
#define _ERROR_INVALID_OUTPUT_FILE_PROVIDED 105
#define _ERROR_INVALID_MTL_SPACE_PROVIDED 106
#define _ERROR_INVALID_DATA_LIMIT_PROVIDED 107
#define _ERROR_EXCESS_DATA_LIMIT_PROVIDED 108
#define _ERROR_IO_ERROR_OCCURRED 109
#define _ERROR_INVALID_COMMAND_PROVIDED 110

#define INPUT_FILE_EXT "obj"
#define OUTPUT_FILE_EXT "objb"
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

char *_input_filename;
char *_output_filename;

bool _input_file_provided = false;
bool _preserve_header_comments = false;
bool _include_w_coords = false;
bool _preserve_group_names = false;

int _mtl_spacing = MIN_MTL_SPACE;
unsigned int _data_limit = DEFAULT_DATA_LIMIT;

OBJGroup **obj_groups;
char **data_header_comments;

void _BUNDLER_HELP() {
	printf( "--preserve-header-comments\n--preserve-group-names\n--include-w-coords\n--mtl-tab-space\n--set-data-limit\n" );
	exit( EXIT_SUCCESS );
}

void _BUNDLER_ERROR( short error, char *payload ) {
	fprintf( stderr, "OBJ_BUNDLER encountered error during process (%d)\n", error );
	switch( error ) {
		case _ERROR_NO_INPUT_FILE_PROVIDED:
			fprintf( stderr, "No input filename provided with arguments.\n" );
			break;
		case _ERROR_NO_OUTPUT_FILE_PROVIDED:
			fprintf( stderr, "No output filename provided with arguments.\n" );
			break;
		case _ERROR_NO_MTL_SPACE_PROVIDED:
			fprintf( stderr, "No explicit spacing value provided with command.\nValid spacing values range from %d-%d\n", MIN_MTL_SPACE, MAX_MTL_SPACE );
			break;
		case _ERROR_INVALID_INPUT_FILE_PROVIDED:
			fprintf( stderr, "Invalid input filename provided [%s]\nValid input filename must end with .%s extension\n", payload, INPUT_FILE_EXT );
			break;
		case _ERROR_INVALID_OUTPUT_FILE_PROVIDED:
			fprintf( stderr, "Invalid output filename provided [%s]\nValid output filename must end with .%s extension\n", payload, OUTPUT_FILE_EXT );
			break;
		case _ERROR_INVALID_MTL_SPACE_PROVIDED:
			fprintf( stderr, "Invalid spacing value provided [%s]\nValid spacing values range from %d-%d\n", payload, MIN_MTL_SPACE, MAX_MTL_SPACE );
			break;
		case _ERROR_INVALID_COMMAND_PROVIDED:
			fprintf( stderr, "Invalid command provided [%s]\nUse --help flag for list of valid commands\n", payload );
			break;
		case _ERROR_IO_ERROR_OCCURRED:
			fprintf( stderr, "Internal I/O error occurred\nCannot open either provided input or output file\n" );
			break;
	}
}

bool _verify_file_extension( char *filename, char *ext ) {
	int ext_len = strlen( ext );
	int filename_len = strlen( filename );
	
	char *ext_buf = (char *) malloc( ext_len + 1 );
	memcpy( ext_buf, &filename[filename_len - ext_len], ext_len );
	ext_buf[ ext_len ] = '\0';
	
	return strcmp( ext_buf, INPUT_FILE_EXT ) == 0 ||
		strcmp( ext_buf, OUTPUT_FILE_EXT ) == 0;
}

OBJGroup *_construct_obj_group() {
	OBJGroup *objg = (OBJGroup *) malloc( sizeof( OBJGroup ) );
	
	objg->name = (char *) malloc( GROUP_NAME_BUFFER_LEN );
	
	objg->data_v = (double **) malloc( _data_limit );
	objg->data_vt = (double **) malloc( _data_limit );
	objg->data_vp = (double **) malloc( _data_limit );
	objg->data_vn = (double **) malloc( _data_limit );
	objg->data_f = (char ***) malloc( _data_limit );
	objg->data_mtllib = (char **) malloc( _data_limit );
	objg->data_mtls = (char **) malloc( _data_limit );
	objg->data_l = (int **) malloc( _data_limit );
	
	objg->shading = (char *) malloc( SHADING_BUFFER_LEN );
	strncpy( objg->shading, "off", SHADING_BUFFER_LEN );
	
	objg->v_index = 0;
	objg->vt_index = 0;
	objg->vp_index = 0;
	objg->vn_index = 0;
	objg->f_index = 0;
	objg->mtllib_index = 0;
	objg->mtls_index = 0;
	objg->l_index = 0;
	
	return objg;
}

int main( int argc, char* argv[] ) {
	
	// Process user input arguments
	
	bool ignore_next_arg = false;
	for( int i = 1; i < argc; i++ ) {
		if( ignore_next_arg ) {
			ignore_next_arg = false;
		}
		else if( strcmp( argv[i], ARG__INPUT_FILENAME ) == 0 ) {
			
			// IF no input filename provided, error out
			if(argc == i + 1) {
				// No more input arguments, error out
				_BUNDLER_ERROR( _ERROR_NO_INPUT_FILE_PROVIDED, NULL );
				exit( EXIT_FAILURE );
			}
			else {
				// If valid file extension, save input filename
				// and toggle input boolean
				if( _verify_file_extension( argv[i + 1], INPUT_FILE_EXT ) ) {
					_input_filename = argv[i + 1];
					_input_file_provided = true;
					ignore_next_arg = true;
				}
				else {
					_BUNDLER_ERROR( _ERROR_INVALID_INPUT_FILE_PROVIDED, argv[i + 1] );
					exit( EXIT_FAILURE );
				}
			}
		}
		else if( strcmp( argv[i], ARG__PRESERVE_HEADER_COMMENTS ) == 0 ) {
			_preserve_header_comments = true;
		}
		else if( strcmp( argv[i], ARG__PRESERVE_GROUP_NAMES ) == 0 ) {
			_preserve_group_names = true;
		}
		else if( strcmp( argv[i], ARG__INCLUDE_W_COORDS ) == 0 ) {
			_include_w_coords = true;
		}
		else if( strcmp( argv[i], ARG__OUTPUT_FILENAME ) == 0 ) {
			if(argc == i + 1) {
				_BUNDLER_ERROR( _ERROR_NO_OUTPUT_FILE_PROVIDED, NULL );
				exit( EXIT_FAILURE );
			}
			else {
				// If valid file extension, save input filename
				// and toggle input boolean
				if( _verify_file_extension( argv[i + 1], OUTPUT_FILE_EXT ) ) {
					_output_filename = argv[i + 1];
					ignore_next_arg = true;
				}
				else {
					_BUNDLER_ERROR( _ERROR_INVALID_OUTPUT_FILE_PROVIDED, argv[i + 1] );
					exit( EXIT_FAILURE );
				}
			}
		}
		else if( strcmp( argv[i], ARG__HELP ) == 0 ) {
			_BUNDLER_HELP();
		}
		else {
			char *mtl_cmd_found = strstr( argv[i], ARG__MTL_TAB_SPACE );
			
			if( mtl_cmd_found != NULL ) {
				int mtl_index = mtl_cmd_found - argv[i];
				
				// IF the command is the MTL command
				if( mtl_index == 0 ) {
					
					// Parse the spacing provided with command
					int mtl_cmd_len = strlen( ARG__MTL_TAB_SPACE );
					int user_mtl_cmd_len = strlen( argv[ i ] );
					
					if( user_mtl_cmd_len == mtl_cmd_len ) {
						_BUNDLER_ERROR( _ERROR_NO_MTL_SPACE_PROVIDED, NULL );
						exit( EXIT_FAILURE );
					}
					
					int spacing_len = user_mtl_cmd_len - mtl_cmd_len;
					char *space_buffer = (char *) malloc( spacing_len );
					memcpy( space_buffer, &( argv[ i ][ user_mtl_cmd_len - spacing_len ] ), spacing_len );
					space_buffer[ spacing_len ] = '\0';
					
					_mtl_spacing = atoi( space_buffer );
					if( _mtl_spacing > MAX_MTL_SPACE ) {
						_BUNDLER_ERROR( _ERROR_INVALID_MTL_SPACE_PROVIDED, space_buffer );
						exit( EXIT_FAILURE );
					}
				}
				else {
					_BUNDLER_ERROR( _ERROR_INVALID_MTL_SPACE_PROVIDED, NULL );
					exit( EXIT_FAILURE );
				}
			}
			else {
				char *sdl_cmd_found = strstr( argv[ i ], ARG__SET_DATA_LIMIT );
				
				if( sdl_cmd_found != NULL ) {
					int sdl_index = sdl_cmd_found - argv[i];
					
					if( sdl_index == 0 ) {
						int sdl_cmd_len = strlen( ARG__SET_DATA_LIMIT );
						int user_sdl_cmd_len = strlen( argv[ i ] );
						
						if( user_sdl_cmd_len == sdl_cmd_len ) {
							_BUNDLER_ERROR( _ERROR_NO_DATA_LIMIT_PROVIDED, NULL );
							exit( EXIT_FAILURE );
						}
						
						int limit_len = user_sdl_cmd_len - sdl_cmd_len;
						
						char *limit_buffer = (char *) malloc( limit_len );
						memcpy( limit_buffer, &( argv[ i ][ user_sdl_cmd_len - limit_len ] ), limit_len );
						limit_buffer[ limit_len ] = '\0';
						
						// IF number of digits provided is greater
						// than the number of digits in the max data
						// limit, error out
						if( limit_len > log2( MAX_DATA_LIMIT ) + 1 ) {
							_BUNDLER_ERROR( _ERROR_EXCESS_DATA_LIMIT_PROVIDED, limit_buffer );
							exit( EXIT_FAILURE );
						}
						
						_data_limit = atoi( limit_buffer );
						if( _data_limit > MAX_DATA_LIMIT || _data_limit < 0 ) {
							_BUNDLER_ERROR( _ERROR_INVALID_DATA_LIMIT_PROVIDED, limit_buffer );
							exit( EXIT_FAILURE );
						}
					}
					else {
						_BUNDLER_ERROR( _ERROR_INVALID_MTL_SPACE_PROVIDED, NULL );
						exit( EXIT_FAILURE );
					}
				}
				else {
					_BUNDLER_ERROR( _ERROR_INVALID_COMMAND_PROVIDED, argv[ i ] );
					exit( EXIT_FAILURE );
				}
			}
		}
	}
	
	// IF input file provided and no output file
	// specified, default to input file with
	// OBJB extension
	if( !_input_file_provided ) {
		_BUNDLER_ERROR( _ERROR_NO_INPUT_FILE_PROVIDED, NULL );
		exit( EXIT_FAILURE );
	}
	
	int inf_len = strlen( _input_filename );
	int pf_len = inf_len - strlen( INPUT_FILE_EXT ) - 1;
	int out_ext_len = strlen( OUTPUT_FILE_EXT );
	
	char *provided_filename = (char *) malloc( pf_len );
	memcpy( provided_filename, _input_filename, pf_len );
	provided_filename[ pf_len ] = '\0';
	
	if( _output_filename == NULL ) {
		fprintf( stdout, "No explicit output file provided\nOBJ_BUNDLER will default to %s.%s\n", provided_filename, OUTPUT_FILE_EXT );
		_output_filename = (char *) malloc( inf_len + 1 );
		
		strncpy( _output_filename, provided_filename, pf_len );
		_output_filename[ pf_len ] = '.';
		for( int i = 1; i < out_ext_len + 1; i++ ) {
			_output_filename[ pf_len + i ] = OUTPUT_FILE_EXT[ i - 1 ];
		}
		_output_filename[ pf_len + out_ext_len + 1 ] = '\0';
	}
	
	FILE *in_handle = fopen( _input_filename, "r" );
	FILE *out_handle = fopen( _output_filename, "w" );
	
	if( in_handle == NULL || out_handle == NULL ) {
		_BUNDLER_ERROR( _ERROR_IO_ERROR_OCCURRED, NULL );
		exit( EXIT_FAILURE );
	}
	
	obj_groups = (OBJGroup **) malloc( OBJ_GROUP_BUFFER_LEN );
	obj_groups[ 0 ] = _construct_obj_group();
	int obj_index = 0;
	
	data_header_comments = (char **) malloc( _data_limit );
	int hc_index = 0;
	
	char *line_buffer = (char *) malloc( LINE_BUFFER_LEN );
	bool header_parsed = false;
	
	while( fgets( line_buffer, LINE_BUFFER_LEN, in_handle ) != NULL )  {
		
		// TODO: Check for mtllib and g
		OBJGroup *cobjg = obj_groups[ obj_index ];
		
		if( line_buffer[ 0 ] == 'v' ) {
			
			header_parsed = true;
			
			if( line_buffer[ 1 ] == 't' ) {
				cobjg->data_vt[ cobjg->vt_index ] = (double *) malloc( sizeof( double ) * VTVPVN_BUFFER_LEN );
				char *ignore_item = (char *) malloc(2);
				
				if( sscanf( line_buffer, "%s %lf %lf %lf", ignore_item, &( cobjg->data_vt[ cobjg->vt_index ][ 0 ] ),
					&( cobjg->data_vt[ cobjg->vt_index ][ 1 ] ), &( cobjg->data_vt[ cobjg->vt_index ][ 2 ] ) ) == 3 ) {
					
					cobjg->data_vt[ cobjg->vt_index ][ 2 ] = 0.0f;
				}
				++cobjg->vt_index;
			}
			else if( line_buffer[ 1 ] == 'p' ) {
				cobjg->data_vp[ cobjg->vp_index ] = (double *) malloc( sizeof( double ) * VTVPVN_BUFFER_LEN );
				char *ignore_item = (char *) malloc(2);
				
				int vp_values_scanned = sscanf( line_buffer, "%s %lf %lf %lf", ignore_item, &( cobjg->data_vp[ cobjg->vp_index ][ 0 ] ),
					&( cobjg->data_vp[ cobjg->vp_index ][ 1 ] ), &( cobjg->data_vp[ cobjg->vp_index ][ 2 ] ) );
					
				if( vp_values_scanned == 2 ) {
					cobjg->data_vp[ cobjg->vp_index ][ 2 ] = 0.0f;
					cobjg->data_vp[ cobjg->vp_index ][ 3 ] = 0.0f;
				}
				else if( vp_values_scanned == 3 ) {
					cobjg->data_vp[ cobjg->vp_index ][ 3 ] = 0.0f;
				}
				++cobjg->vp_index;
			}
			else if( line_buffer[ 1 ] == 'n' ) {
				cobjg->data_vn[ cobjg->vn_index ] = (double *) malloc( sizeof( double ) * VTVPVN_BUFFER_LEN );
				char *ignore_item = (char *) malloc(2);
				
				sscanf( line_buffer, "%s %lf %lf %lf", ignore_item, &( cobjg->data_vn[ cobjg->vn_index ][ 0 ] ),
					&( cobjg->data_vn[ cobjg->vn_index ][ 1 ] ), &( cobjg->data_vn[ cobjg->vn_index ][ 2 ] ) );
				++cobjg->vn_index;
			}
			else {
				cobjg->data_v[ cobjg->v_index ] = (double *) malloc( sizeof( double ) * VF_BUFFER_LEN );
				char *ignore_item = (char *) malloc(1);
				
				if( sscanf( line_buffer, "%s %lf %lf %lf %lf", ignore_item, &( cobjg->data_v[ cobjg->v_index ][ 0 ] ),
					&( cobjg->data_v[ cobjg->v_index ][ 1 ] ), &( cobjg->data_v[ cobjg->v_index ][ 2 ] ),
					&( cobjg->data_v[ cobjg->v_index ][ 3 ] ) ) == 4 ) {

					cobjg->data_v[ cobjg->v_index ][ 3 ] = 1.0f;
				}
				++cobjg->v_index;
			}
		}
		else if( line_buffer[ 0 ] == 'f' ) {
			
			header_parsed = true;
			
			cobjg->data_f[ cobjg->f_index ] = (char **) malloc( sizeof( char * ) * VF_BUFFER_LEN );
			for( int i = 0; i < VF_BUFFER_LEN; i++ ) {
				cobjg->data_f[ cobjg->f_index ][ i ] = (char *) malloc( VALUE_BUFFER_LEN );
			}
			char *ignore_item = (char *) malloc(1);
			
			if( sscanf( line_buffer, "%s %s %s %s %s", ignore_item, cobjg->data_f[ cobjg->f_index ][ 0 ],
				cobjg->data_f[ cobjg->f_index ][ 1 ], cobjg->data_f[ cobjg->f_index ][ 2 ],
				cobjg->data_f[ cobjg->f_index ][ 3 ] ) == 4 ) {

				cobjg->data_f[ cobjg->f_index ][ 3 ][ 0 ] = '\0';
			}
			++cobjg->f_index;
		}
		else if( line_buffer[ 0 ] == 'l' ) {
			
			header_parsed = true;
			
			cobjg->data_l[ cobjg->l_index ] = (int *) malloc( sizeof( int ) * L_BUFFER_LEN );
			char *ignore_item = (char *) malloc(1);
			
			sscanf( line_buffer, "%s %d %d %d %d %d %d", ignore_item, &( cobjg->data_l[ cobjg->l_index ][ 0 ] ),
				&( cobjg->data_l[ cobjg->l_index ][ 1 ] ), &( cobjg->data_l[ cobjg->l_index ][ 2 ] ),
				&( cobjg->data_l[ cobjg->l_index ][ 3 ] ), &( cobjg->data_l[ cobjg->l_index ][ 4 ] ),
				&( cobjg->data_l[ cobjg->l_index ][ 5 ] ) );
			++cobjg->l_index;
		}
		else if( line_buffer[ 0 ] == '#' && !header_parsed ) {
			int line_buf_len = strlen( line_buffer );
			data_header_comments[ hc_index ] = (char *) malloc( line_buf_len );
			strncpy( data_header_comments[ hc_index ], line_buffer, line_buf_len );
			data_header_comments[ hc_index++ ][ line_buf_len ] = '\0';
		}
		else if( line_buffer[ 0 ] == 'g' ) {
			if( header_parsed ) {
				obj_groups[ ++obj_index ] = _construct_obj_group();
				cobjg = obj_groups[ obj_index ];
			}
			char *scanned_name = (char *) malloc( GROUP_NAME_BUFFER_LEN );
			char *ignore_item = (char *) malloc(1);
			
			sscanf( line_buffer, "%s %s", ignore_item, scanned_name );
			
			int scan_name_len = strlen( scanned_name );
			strncpy( cobjg->name, scanned_name, scan_name_len );
			cobjg->name[ scan_name_len ] = '\0';
			
			header_parsed = true;
		}
		else if( line_buffer[ 0 ] == 's' ) {
			char *shading = (char *) malloc( SHADING_BUFFER_LEN );
			char *ignore_item = (char *) malloc(1);
			
			sscanf( line_buffer, "%s %s", ignore_item, shading );
			strncpy( cobjg->shading, shading, SHADING_BUFFER_LEN );
		}
		else {
			char *mtllib_found = strstr( line_buffer, "mtllib" );
			
			// IF line starts with 'mtllib'
			if( line_buffer - mtllib_found == 0 ) {
				
				// Two name stores in case material
				// name is listed before material
				// filename
				// EX: mtllib asteroid asteroid.obj
				char *initial_name_store = (char *) malloc( MTL_FILENAME_BUFFER_LEN );
				char *fallback_name_store = (char *) malloc( MTL_FILENAME_BUFFER_LEN );
				
				char *ignore_item = (char *) malloc( sizeof( "mtllib" ) );
				sscanf( line_buffer, "%s %s %s", ignore_item, initial_name_store,
					fallback_name_store );
					
				char *mtl_filename = fallback_name_store == NULL ?
					initial_name_store : fallback_name_store;
				cobjg->data_mtllib[ cobjg->mtllib_index ] = mtl_filename;
				
				++cobjg->mtllib_index;
			}
			else {
				char *mtls_found = strstr( line_buffer, "usemtl" );
				
				if( line_buffer - mtls_found == 0 ) {
					
					header_parsed = true;
					
					char *mtl_name = (char *) malloc( MTL_NAME_BUFFER_LEN );
					
					char *ignore_item = (char *) malloc( sizeof( "usemtl" ) );
					sscanf( line_buffer, "%s %s", ignore_item, mtl_name );
					
					cobjg->data_mtls[ cobjg->mtls_index ] = mtl_name;
					
					++cobjg->mtls_index;
				}
			}
		}
	}
	
	// Account for first object
	++obj_index;
	
	// Print the header comments
	if( _preserve_header_comments ) {
		for( int i = 0; i < hc_index; i++ ) {
			fprintf( out_handle, "%s", data_header_comments[ i ] );
		}
	}
	if( _include_w_coords ) {
		fprintf( out_handle, "%s\n", "INCLUDE_W_COORDS" );
	}
	fprintf( out_handle, "MTL_TAB_SPACING=%u\n", _mtl_spacing );
	
	for( int i = 0; i < obj_index; i++ ) {
		// Print group name and shading data
		fprintf( out_handle, "g %s\n", obj_groups[ i ]->name );
		fprintf( out_handle, "s %s\n", obj_groups[ i ]->shading );
		
		// Print vertex data
		fprintf( out_handle, "%s", "v " );
		for( int s = 0; s < obj_groups[ i ]->v_index; s++ ) {
			if( _include_w_coords ) {
				fprintf( out_handle, "%lf %lf %lf %lf",
					obj_groups[ i ]->data_v[ s ][ 0 ],
					obj_groups[ i ]->data_v[ s ][ 1 ],
					obj_groups[ i ]->data_v[ s ][ 2 ],
					obj_groups[ i ]->data_v[ s ][ 3 ] );
			}
			else {
				fprintf( out_handle, "%lf %lf %lf",
					obj_groups[ i ]->data_v[ s ][ 0 ],
					obj_groups[ i ]->data_v[ s ][ 1 ],
					obj_groups[ i ]->data_v[ s ][ 2 ] );	
			}
			if( s < obj_groups[ i ]->v_index - 1 ) {
				fprintf( out_handle, "%s", " " );
			}
		}
		fprintf( out_handle, "\n" );
		
		// Print vertex texture data
		fprintf( out_handle, "%s", "vt " );
		for( int s = 0; s < obj_groups[ i ]->vt_index; s++ ) {
			fprintf( out_handle, "%lf %lf %lf",
				obj_groups[ i ]->data_vt[ s ][ 0 ],
				obj_groups[ i ]->data_vt[ s ][ 1 ],
				obj_groups[ i ]->data_vt[ s ][ 2 ] );
			if( s < obj_groups[ i ]->vt_index - 1 ) {
				fprintf( out_handle, "%s", " " );
			}
		}
		fprintf( out_handle, "\n" );
		
		// Print vertex parameter data
		fprintf( out_handle, "%s", "vp " );
		for( int s = 0; s < obj_groups[ i ]->vp_index; s++ ) {
			fprintf( out_handle, "%lf %lf %lf",
				obj_groups[ i ]->data_vp[ s ][ 0 ],
				obj_groups[ i ]->data_vp[ s ][ 1 ],
				obj_groups[ i ]->data_vp[ s ][ 2 ] );
			if( s < obj_groups[ i ]->vp_index - 1 ) {
				fprintf( out_handle, "%s", " " );
			}
		}
		fprintf( out_handle, "\n" );
		
		// Print vertex normal data
		fprintf( out_handle, "%s", "vn " );
		for( int s = 0; s < obj_groups[ i ]->vn_index; s++ ) {
			fprintf( out_handle, "%lf %lf %lf",
				obj_groups[ i ]->data_vn[ s ][ 0 ],
				obj_groups[ i ]->data_vn[ s ][ 1 ],
				obj_groups[ i ]->data_vn[ s ][ 2 ] );
			if( s < obj_groups[ i ]->vn_index - 1 ) {
				fprintf( out_handle, "%s", " " );
			}
		}
		fprintf( out_handle, "\n" );
		
		// Print face data
		fprintf( out_handle, "%s", "f " );
		for( int s = 0; s < obj_groups[ i ]->f_index; s++ ) {
			fprintf( out_handle, "%s %s %s %s",
				obj_groups[ i ]->data_f[ s ][ 0 ],
				obj_groups[ i ]->data_f[ s ][ 1 ],
				obj_groups[ i ]->data_f[ s ][ 2 ],
				obj_groups[ i ]->data_f[ s ][ 3 ] );
			if( s < obj_groups[ i ]->f_index - 1 ) {
				fprintf( out_handle, "%s", " " );
			}
		}
		fprintf( out_handle, "\n" );
		
		// Print line data
		fprintf( out_handle, "%s", "l " );
		for( int s = 0; s < obj_groups[ i ]->l_index; s++ ) {
			fprintf( out_handle, "%d %d %d %d %d %d",
				obj_groups[ i ]->data_l[ s ][ 0 ],
				obj_groups[ i ]->data_l[ s ][ 1 ],
				obj_groups[ i ]->data_l[ s ][ 2 ],
				obj_groups[ i ]->data_l[ s ][ 3 ],
				obj_groups[ i ]->data_l[ s ][ 4 ],
				obj_groups[ i ]->data_l[ s ][ 5 ] );
			if( s < obj_groups[ i ]->l_index - 1 ) {
				fprintf( out_handle, "%s", " " );
			}
		}
		
		if( i < obj_index - 1 ) {
			fprintf( out_handle, "\n" );
		}
	}
	
	fclose( in_handle );
	fclose( out_handle );
	
	return EXIT_SUCCESS;
}