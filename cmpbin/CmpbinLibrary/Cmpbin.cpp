// wx includes must go before standard includes
#include <wx/dir.h>
#include <wx/log.h>
#include <wx/string.h>

#include <iostream>
#include <map>
#include <regex>
#include <fstream>
#include <unordered_set>

#include "Cmpbin.h"
#include "MurMurHash3.h"

int Compare(wxString dirPath1, wxString dirPath2, wxString &textOutput, std::vector<ListDataItem> &listDataItems)
{
	const wxString dirPaths[2] = { dirPath1 , dirPath2 };

	textOutput.Append("Running comparison...\n");
	textOutput.Append(wxString::Format(wxT("\n1st directory is '%s'\n"), dirPath1));
	textOutput.Append(wxString::Format(wxT("2nd directory is '%s'\n\n"), dirPath2));

	// Create dictionaries with hashes of file contents and file names
	std::map<std::wstring, std::vector<std::wstring>> dictionaries[2];
	for (int counter = 0; counter < 2; counter++)
	{
		textOutput.Append(wxString::Format(wxT("Creating hashes for directory '%s'\n"), dirPaths[counter]));

		const wxString &dirPath = dirPaths[counter];
		wxDir dir;
		dir.Open(dirPath);
		if (dir.IsOpened())
		{
			wxArrayString *files = new wxArrayString;

			size_t s = dir.GetAllFiles(dirPath, files, wxEmptyString, wxDIR_FILES);
			for (int i = 0; i < files->Count(); i++)
			{
				wxString filePath = files->Item(i);
				if (filePath.empty() == false)
				{
					std::wstring filePathStr = filePath.ToStdWstring();
					std::ifstream inputFile(filePathStr, std::ios::binary);
					if (inputFile.fail())
					{
						textOutput.Append(wxString::Format(wxT("Invalid stream for: '%s'\nExiting\n"), filePath));
						return -1;
					}
					wxString fileName = wxFileNameFromPath(filePath);
					std::wstring fileNameStr = fileName.ToStdWstring();

					// Resolve "most vexing parse" and read all byte content from file stream to vector
					std::vector<char> data((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
					uint64_t len = data.size();

					// Make file hash.
					uint64_t seed = 1;
					uint64_t hashParts[2];
					MurmurHash3_x64_128(&data[0], len, seed, hashParts);

					std::wstring hash = std::to_wstring(hashParts[0]);
					hash += std::to_wstring(hashParts[1]);

					// Put file name into dictionary.
					if (dictionaries[counter].find(hash) != dictionaries[counter].end())
						dictionaries[counter][hash].push_back(fileNameStr);
					else
					{
						std::vector<std::wstring> filePaths = std::vector<std::wstring>();
						filePaths.push_back(fileNameStr);
						dictionaries[counter].insert(std::make_pair(hash, filePaths));
					}

					inputFile.close();
					data.clear();
				}
			}
		}
	}

	// Compare hashes and associate file names
	std::unordered_set<std::wstring> keysInDictionary1;
	std::map<std::wstring, std::vector<std::wstring>>::iterator itDict;
	for (itDict = dictionaries[0].begin(); itDict != dictionaries[0].end(); itDict++)
	{
		ListDataItem listDataItem = ListDataItem();
		std::wstring key = itDict->first;
		listDataItem.Hash = key;

		if (dictionaries[1].find(key) != dictionaries[1].end())
		{
			listDataItem.Hash = itDict->first;
			listDataItem.FilesFromDirectory1 = itDict->second;

			std::vector<std::wstring> matchedFiles = dictionaries[1][key];
			listDataItem.FilesFromDirectory2 = matchedFiles;
			keysInDictionary1.insert(key);
		}
		else
		{
			listDataItem.FilesFromDirectory1 = itDict->second;
			keysInDictionary1.insert(key);
		}
		listDataItems.push_back(listDataItem);
	}

	for (itDict = dictionaries[1].begin(); itDict != dictionaries[1].end(); itDict++)
	{
		if (keysInDictionary1.find(itDict->first) == keysInDictionary1.end())
		{
			ListDataItem listDataItem = ListDataItem();
			listDataItem.Hash = itDict->first;
			listDataItem.FilesFromDirectory2 = itDict->second;

			listDataItems.push_back(listDataItem);
		}
	}

	textOutput.Append(wxT("\nFiles comparisons using generated hashes:\n"));

	constexpr int filenamePaddedLength = 64;
	wxString blank = L" ";

	textOutput.Append(wxT("\nFile hash\n"));
	textOutput.Append(wxString(wxT("1st directory file(s)")).Pad(filenamePaddedLength - 21, ' ', true));
	textOutput.Append(wxString(wxT("2nd directory file(s)")).Pad(filenamePaddedLength - 21, ' ', false));
	textOutput.Append(wxT("\n"));
	
	for (auto listDataItem : listDataItems) // access by reference to avoid copying
	{
		textOutput.Append(wxString::Format(wxT("\n%s\n"), listDataItem.Hash));

		// Build text descriptions
		int size1 = listDataItem.FilesFromDirectory1.size();
		int size2 = listDataItem.FilesFromDirectory2.size();
		int maxCount = size1 > size2 ? size1 : size2;
		for (int i = 0; i < maxCount; i++)
		{
			blank = L" ";
			if (i < size1)
			{
				wxString filename = listDataItem.FilesFromDirectory1[i], filenamePadded;
				if (filename.size() < filenamePaddedLength)
					filenamePadded = filename.Pad(filenamePaddedLength - filename.size(), ' ', true);
				else
					filenamePadded = filename + " ";

				textOutput.Append(filenamePadded);
			}
			else
				textOutput.Append(blank.Pad(filenamePaddedLength - 1, ' ', true));

			if (i < size2)
			{
				wxString filename = listDataItem.FilesFromDirectory2[i], filenamePadded;
				if (filename.size() < filenamePaddedLength)
					filenamePadded = filename.Pad(filenamePaddedLength - filename.size(), ' ', false);
				else
					filenamePadded = filename + " ";

				textOutput.Append(filenamePadded + "\n");
			}
			else
				textOutput.Append(blank.Pad(filenamePaddedLength - 1, ' ', true) + "\n");
		}
	}

	return 0;
}