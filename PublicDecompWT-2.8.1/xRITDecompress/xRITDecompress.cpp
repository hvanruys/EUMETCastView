/*
 * Copyright 2011-2019, European Organisation for the Exploitation of Meteorological Satellites (EUMETSAT)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*****************************************************************************
****  DADF MODULE HEADER   ***

TYPE:
Main program.
					
PURPOSE:
Command line tool for manual decompression of HRIT/LRIT files.

FUNCTION:
Command line tool for manual decompression of HRIT/LRIT files.
Used for integration testing and in the off-line environment.

INTERFACES:
See Usage() function.

RESOURCES:	
Heap Memory (>2K).

REFERENCES:
None.

PROCESSING:
Opens the specified compressed HRIT/LRIT file, decompresses it and writes
the decompressed file to the current working directory.

DATA:
See 'DATA:' in the class header below.

LOGIC:		
-

****  END MODULE HEADER   ***
*******************************************************************************/

#include <stdio.h>

#include "ErrorHandling.h"			// Util
#include "CxRITFile.h"				// DISE
#include "CxRITFileDecompressed.h"	// DISE


const char* VERSION_NUMBER="2.04";

// Description:	Tells the correct command line syntax.
// Returns:		Nothing.
void Usage
(
	const std::string& i_ProgramName
)
{
	std::cout << "Usage: " << i_ProgramName << 
		" [-s:]<Name of compressed HRIT/LRIT file>"
		" [--help]"
		" [--version]"
		"\n";
}

struct Parameters {
	char* sourceFile;
	bool versionRequested;
	bool helpRequested;
	int errorCode;
};

// Description:	Parses the parameters.
// Returns:		true when processing should be halted (either help or version requested or errors found)
bool getParameters(int argc, char* argv[], Parameters* params) {
	params->sourceFile = NULL;
	params->versionRequested = false;
	params->helpRequested = false;
	params->errorCode=0;
	
	for (int i=1; i<argc; i++) {
		if (!strncmp(argv[i], "-s:", strlen("-s:"))) {
			params->sourceFile = argv[i] + strlen("-s:");
		} else if (!strcmp(argv[i], "--version")) {
			params->versionRequested = true;
		} else if (!strcmp(argv[i], "--help")) {
			params->helpRequested = true;
		} else if (params->sourceFile == NULL) {
			params->sourceFile = argv[i];
		} else {
			std::cerr << "Invalid usage\n";
			Usage(argv[0]);
			params->errorCode=1;
			return true;
		}
	}
	if (params->helpRequested) {
		Usage(argv[0]);
		params->errorCode=0;
		return true;
	} else if (params->versionRequested) {
		std::cout << "Version: " << VERSION_NUMBER << "\n";
		params->errorCode=0;
		return true;
	} else if (params->sourceFile == NULL) {
		std::cerr << "No HRIT/LRIT file specified\n";
		Usage(argv[0]);
		params->errorCode=1;
		return true;
	} else {
		return false;
	}
}




// Description:	Entry function of program.
// Returns:		0 on success,
//				1 on error on parameters.
//				2 on error on execution.
int main
(
	int   argc,
	char* argv[]
)
{
	Parameters params;
	bool halt = getParameters(argc, argv, &params);
	if (halt) {
		return params.errorCode;
	}
	
	// Open input file.
	DISE::CxRITFile compressedFile;
	try
	{
		compressedFile = DISE::CxRITFile(params.sourceFile);
	}
	catch (...)
	{
		std::cerr << "Opening/reading input file failed.\n";
		Usage(argv[0]);
		return 1;
	}

	// Decompress input file.
	DISE::CxRITFileDecompressed decompressedFile(compressedFile);

	// Store decompressed file.
	decompressedFile.Write(decompressedFile.GetAnnotation().GetText());
	std::cout << "Decompressed file: " << decompressedFile.GetAnnotation().GetText() << "\n";

	return 0;
}
