#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define ARG__INPUT_FILENAME "--in"
#define ARG__OUTPUT_FILENAME "--out"
#define ARG__PRESERVE_COMMENTS "--preserve-comments"
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
#define _ERROR_INVALID_COMMAND_PROVIDED 109

#define INPUT_FILE_EXT "obj"
#define OUTPUT_FILE_EXT "objb"
#define MIN_MTL_SPACE 3
#define MAX_MTL_SPACE 10

#define LINE_BUFFER_LEN 100
#define DEFAULT_DATA_LIMIT 1000000
#define MAX_DATA_LIMIT 99999999
#define V_BUFFER_LEN 4
#define VTVPVNF_BUFFER_LEN 3
#define VALUE_BUFFER_LEN 50
#define L_BUFFER_LEN 6
#define MTL_FILENAME_BUFFER_LEN 50
#define MTL_NAME_BUFFER_LEN 100

char *_input_filename;
char *_output_filename;

bool _input_file_provided = false;
bool _preserve_comments = false;
bool _include_w_coords = false;
bool _preserve_group_names = false;

int _mtl_spacing = MIN_MTL_SPACE;
unsigned int _data_limit = DEFAULT_DATA_LIMIT;

double **_data_v;
double **_data_vt;
double **_data_vp;
double **_data_vn;
char ***_data_f;
char ** _data_mtllib;
char ** _data_mtls;
int **_data_l;

void _BUNDLER_HELP() {
	
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
		else if( strcmp( argv[i], ARG__PRESERVE_COMMENTS ) == 0 ) {
			_preserve_comments = true;
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
	
	// Initialize data buffers
	_data_v = (double **) malloc( _data_limit );
	_data_vt = (double **) malloc( _data_limit );
	_data_vp = (double **) malloc( _data_limit );
	_data_vn = (double **) malloc( _data_limit );
	_data_f = (char ***) malloc( _data_limit );
	_data_mtllib = (char **) malloc( _data_limit );
	_data_mtls = (char **) malloc( _data_limit );
	_data_l = (int **) malloc( _data_limit );
	
	int v_index = 0;
	int vt_index = 0;
	int vp_index = 0;
	int vn_index = 0;
	int f_index = 0;
	int mtllib_index = 0;
	int mtls_index = 0;
	int l_index = 0;
	
	FILE *in_handle = fopen( _input_filename, "r" );
	FILE *out_handle = fopen( _output_filename, "w" );
	
	if( in_handle == NULL ) {
		// TODO: IO error
		exit( EXIT_FAILURE );
	}
	
	char *line_buffer = (char *) malloc( LINE_BUFFER_LEN );
	while( fgets( line_buffer, LINE_BUFFER_LEN, in_handle ) != NULL )  {
		
		// TODO: Check for mtllib and g
		printf("%s", line_buffer);
		
		if( line_buffer[ 0 ] == 'v' ) {
			if( line_buffer[ 1 ] == 't' ) {
				_data_vt[ vt_index ] = (double *) malloc( VTVPVNF_BUFFER_LEN );
				char *ignore_item = (char *) malloc(2);
				
				sscanf( line_buffer, "%s %lf %lf %lf %lf", ignore_item, &( _data_vt[ vt_index ][ 0 ] ),
					&( _data_vt[ vt_index ][ 1 ] ), &( _data_vt[ vt_index ][ 2 ] ),
					&( _data_vt[ vt_index ][ 3 ] ) );
				++vt_index;
			}
			else if( line_buffer[ 1 ] == 'p' ) {
				_data_vp[ vp_index ] = (double *) malloc( VTVPVNF_BUFFER_LEN );
				char *ignore_item = (char *) malloc(2);
				
				sscanf( line_buffer, "%s %lf %lf %lf", ignore_item, &( _data_vp[ vp_index ][ 0 ] ),
					&( _data_vp[ vp_index ][ 1 ] ), &( _data_vp[ vp_index ][ 2 ] ) );
				++vp_index;
			}
			else if( line_buffer[ 1 ] == 'n' ) {
				_data_vn[ vn_index ] = (double *) malloc( VTVPVNF_BUFFER_LEN );
				char *ignore_item = (char *) malloc(2);
				
				sscanf( line_buffer, "%s %lf %lf %lf", ignore_item, &( _data_vn[ vn_index ][ 0 ] ),
					&( _data_vn[ vn_index ][ 1 ] ), &( _data_vn[ vn_index ][ 2 ] ) );
				++vn_index;
			}
			else {
				_data_v[ v_index ] = (double *) malloc( V_BUFFER_LEN );
				char *ignore_item = (char *) malloc(2);
				
				sscanf( line_buffer, "%s %lf %lf %lf %lf", &( _data_v[ v_index ][ 0 ] ),
					&( _data_v[ v_index ][ 1 ] ), &( _data_v[ v_index ][ 2 ] ),
					&( _data_v[ v_index ][ 3 ] ) );
				++v_index;
			}
		}
		else if( line_buffer[ 0 ] == 'f' ) {
			_data_f[ f_index ] = (char **) malloc( VTVPVNF_BUFFER_LEN );
			for( int i = 0; i < VTVPVNF_BUFFER_LEN; i++ ) {
				_data_f[ f_index ][ i ] = (char *) malloc( VALUE_BUFFER_LEN );
			}
			char *ignore_item = (char *) malloc(2);
			
			sscanf( line_buffer, "%s %s %s %s", ignore_item, &( _data_f[ f_index ][ 0 ] ),
				&( _data_f[ f_index ][ 1 ] ), &( _data_f[ f_index ][ 2 ] ) );
			++f_index;
		}
		else if( line_buffer[ 0 ] == 'l' ) {
			_data_l[ l_index ] = (int *) malloc( L_BUFFER_LEN );
			char *ignore_item = (char *) malloc(2);
			
			sscanf( line_buffer, "%s %d %d %d %d %d %d", ignore_item, &( _data_l[ l_index ][ 0 ] ),
				&( _data_l[ l_index ][ 1 ] ), &( _data_l[ l_index ][ 2 ] ),
				&( _data_l[ l_index ][ 3 ] ), &( _data_l[ l_index ][ 4 ] ),
				&( _data_l[ l_index ][ 5 ] ) );
			++l_index;
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
				
				char *ignore_item = (char *) malloc(2);
				sscanf( line_buffer, "%s %s %s", ignore_item, initial_name_store,
					fallback_name_store );
					
				char *mtl_filename = fallback_name_store == NULL ?
					initial_name_store : fallback_name_store;
				_data_mtllib[ mtllib_index ] = mtl_filename;
				
				free( initial_name_store );
				free( fallback_name_store );
				
				++mtllib_index;
			}
			else {
				char *mtls_found = strstr( line_buffer, "usemtl" );
				
				if( line_buffer - mtls_found == 0 ) {
					
					char *mtl_name = (char *) malloc( MTL_NAME_BUFFER_LEN );
					
					char *ignore_item = (char *) malloc(2);
					sscanf( line_buffer, "%s %s", ignore_item, mtl_name );
					
					_data_mtls[ mtls_index ] = mtl_name;
					free( mtl_name );
					
					++mtls_index;
				}
			}
		}
	}
	
	free( line_buffer );
	fclose( in_handle );
	fclose( out_handle );
	
	return EXIT_SUCCESS;
}