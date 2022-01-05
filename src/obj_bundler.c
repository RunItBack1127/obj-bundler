#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define ARG__INPUT_FILENAME "--in"
#define ARG__OUTPUT_FILENAME "--out"
#define ARG__PRESERVE_COMMENTS "--preserve-comments"
#define ARG__PRESERVE_GROUP_NAMES "--preserve-group-names"
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

char *_input_filename;
char *_output_filename;

bool _input_file_provided = false;
bool _preserve_comments = false;
bool _preserve_group_names = false;

int _mtl_spacing = MIN_MTL_SPACE;
unsigned int _data_limit = DEFAULT_DATA_LIMIT;

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
	
	for( int i = 1; i < argc; i++ ) {
		if( strcmp(argv[i], ARG__INPUT_FILENAME ) == 0 ) {
			
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
				}
				else {
					_BUNDLER_ERROR( _ERROR_INVALID_INPUT_FILE_PROVIDED, argv[i + 1] );
					exit( EXIT_FAILURE );
				}
			}
		}
		else if( strcmp( argv[i], ARG__PRESERVE_COMMENTS ) == 0) {
			_preserve_comments = true;
		}
		else if( strcmp( argv[i], ARG__PRESERVE_GROUP_NAMES ) == 0) {
			_preserve_group_names = true;
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
						if( _data_limit > MAX_DATA_LIMIT ) {
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
	
	if( in_handle == NULL ) {
		// TODO: IO error
		exit( EXIT_FAILURE );
	}
	
	char *line_buffer = (char *) malloc( LINE_BUFFER_LEN );
	while( fgets( line_buffer, LINE_BUFFER_LEN, in_handle ) ) {
		switch( line_buffer[ 0 ] ) {
			case 'v':
				if( line_buffer[ 1 ] == 't' ) {
					
				}
			break;
		}
	}
	
	return EXIT_SUCCESS;
}