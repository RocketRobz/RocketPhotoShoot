/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2017
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/

#include "file_browse.h"
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include <3ds.h>

int numberOfExportedCharacters = 0;

using namespace std;

struct DirEntry {
	string name;
	bool isDirectory;
	bool gender;
} ;

vector<DirEntry> exportedCharacterContents;

bool nameEndsWith (const string& name, const vector<string> extensionList) {

	if (name.size() == 0) return false;

	if (extensionList.size() == 0) return true;

	for (int i = 0; i < (int)extensionList.size(); i++) {
		const string ext = extensionList.at(i);
		if ( strcasecmp (name.c_str() + name.size() - ext.size(), ext.c_str()) == 0) return true;
	}
	return false;
}

bool dirEntryPredicate (const DirEntry& lhs, const DirEntry& rhs) {

	if (!lhs.isDirectory && rhs.isDirectory) {
		return false;
	}
	if (lhs.isDirectory && !rhs.isDirectory) {
		return true;
	}
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

void getDirectoryContents (vector<DirEntry>& dirContents, const vector<string> extensionList, bool showDir, const char* dirPath) {
	struct stat st;

	dirContents.clear();

	DIR *pdir = opendir (dirPath);

	if (pdir == NULL) {
		//iprintf ("Unable to open the directory.\n");
	} else {

		while(true) {
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if(pent == NULL) break;

			stat(pent->d_name, &st);
			dirEntry.name = pent->d_name;
			dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;

			if (dirEntry.name.compare(".") != 0 && ((dirEntry.isDirectory && showDir) || nameEndsWith(dirEntry.name, extensionList))) {
				if (!dirEntry.isDirectory) {
					for (int i = dirEntry.name.size(); i > 0; i--) {
						if (dirEntry.name[i] == '.') {
							dirEntry.name[i] = '\0';	// Hide file extension from display
							break;
						}
					}
				}
				dirContents.push_back (dirEntry);
			}

		}

		closedir(pdir);
	}

	sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);
}

/*void getDirectoryContents (vector<DirEntry>& dirContents) {
	vector<string> extensionList;
	getDirectoryContents (dirContents, extensionList);
}*/

void getExportedCharacterContents (void) {
	vector<string> extensionList;
	extensionList.emplace_back(".chr");
	getDirectoryContents (exportedCharacterContents, extensionList, false, "sdmc:/3ds/SavvyManager/SS4/characters");
	numberOfExportedCharacters = exportedCharacterContents.size();

	// Get genders from .chr files
	char chrPath[256];
	for (int i = 0; i < numberOfExportedCharacters; i++) {
		DirEntry* entry = &exportedCharacterContents.at(i);

		sprintf(chrPath, "sdmc:/3ds/SavvyManager/SS4/characters/%s.chr", entry->name.c_str());

		u8 result = 0;

		FILE* chrFile = fopen(chrPath, "rb");
		fread(&result, 1, 1, chrFile);
		fclose(chrFile);

		entry->gender = (result == 2);
	}
}

const char* getExportedCharacterName (int num) {
	const DirEntry* entry = &exportedCharacterContents.at(num);
	return entry->name.c_str();
}

bool getExportedCharacterGender (int num) {
	const DirEntry* entry = &exportedCharacterContents.at(num);
	return entry->gender;
}
