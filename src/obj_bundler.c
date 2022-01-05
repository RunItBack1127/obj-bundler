#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define ARG__INPUT_FILENAME "--in"
#define ARG__PRESERVE_COMMENTS "--preserve-comments"
#define ARG__PRESERVE_GROUP_NAMES "--preserve-group-names"

#define _ERROR_NO_INPUT_FILE_PROVIDED 100
#define _ERROR_NO_OUTPUT_FILE_PROVIDED 101
#define _ERROR_INVALID_INPUT_FILE_PROVIDED 102
#define _ERROR_INVALID_OUTPUT_FILE_PROVIDED 103
#define _ERROR_INVALID_MTL_SPACE_PROVIDED 104
#define _ERROR_INVALID_COMMAND_PROVIDED 105

#define INPUT_FILE_EXT "obj"
#define OUTPUT_FILE_EXT "objb"
#define MIN_MTL_SPACE 3
#define MAX_MTL_SPACE 10

char *_input_filename;
char *_output_filename;

bool _input_file_provided = false;

void _BUNDLER_HELP() {
	
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
			_BUNDLER_HELP();
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
		else if( strcmp(argv[i], ARG__PRESERVE_COMMENTS ) == 0) {
			
		}
		else if( strcmp(argv[i], ARG__PRESERVE_GROUP_NAMES ) == 0) {
			
		}
	}
	
	// Check if input file provided
	
	return EXIT_SUCCESS;
}