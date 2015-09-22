
#pragma once

/*
	Reads a binary file using TIO completely into memory.
*/
vector<char> *TioReadBinaryFile(const string &filename);

/*
	Checks if a given directory exists.
*/
bool TioDirExists(const string &path);

/*
	Clears a directory of its content.
*/
bool TioClearDir(const string &path);
