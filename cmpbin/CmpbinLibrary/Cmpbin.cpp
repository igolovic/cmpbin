// wx includes must go before standard includes
#include <wx/dir.h>
#include <wx/log.h>
#include <wx/string.h>

#include <iostream>
#include <map>
#include <regex>
#include <fstream>
#include <string>
#include <unordered_set>

#include "Cmpbin.h"
#include "MurmurHash3.h"
#include "../WorkerThread.h"
#include "../CmpbinFrame.h"

void Compare(
    wxString dirPath1,
    wxString dirPath2,
    CmpbinFrame* pParent,
    wxCommandEvent statusEvent,
    wxCommandEvent finishedEvent,
    void (*status)(CmpbinFrame*, wxCommandEvent, wxString),
    void (*finished)(CmpbinFrame*, wxCommandEvent, int, wxString, std::vector<ListDataItem>*),
    bool (*isCancelled)(CmpbinFrame*, wxCommandEvent)
    )
{
	const wxString dirPaths[2] = { dirPath1 , dirPath2 };
    wxString textOutput;
    std::vector<ListDataItem> *pListDataItems = new std::vector<ListDataItem>();

    status(pParent, statusEvent, wxT("Starting comparison..."));
	
	/* TODO:
	- add pre-hash comparison by file size (hash comparison is expensive)
	- list files (using GetNextFile to get file size immediatelly?)
	- create dictionaries with file sizes and file names
	- find file size matches, unique1 file sizes, unique2 file sizes
	- unique1 file sizes, unique2 file sizes - go to unique results immediatelly
	- for file size matches run hash creation and comparison
	*/

	// Create dictionaries with hashes of file contents and file names
	std::map<std::string, std::vector<std::string>> dictionaries[2];
	for (int counter = 0; counter < 2; counter++)
	{
		wxString statusMessage = wxString::Format(wxT("Creating hashes for '%s'\n"), dirPaths[counter]);
		status(pParent, statusEvent, statusMessage);

		const wxString &dirPath = dirPaths[counter];
		wxDir dir;
		dir.Open(dirPath);
		if (dir.IsOpened())
		{
			wxArrayString *files = new wxArrayString;

			dir.GetAllFiles(dirPath, files, wxEmptyString, wxDIR_FILES);
			for (size_t i = 0; i < files->Count(); i++)
			{
                if (isCancelled(pParent, statusEvent))
                {
                    FreeResources(pListDataItems);
                    return;
                }
                
				// Reporting progress in GUI incurs performance penalty, comment it out
				//status(pParent, statusEvent, wxString::Format(wxT("%s - hashing file %zu of %zu"), dirPath, i + 1, files->Count()));

				wxString filePath = files->Item(i);
				if (filePath.empty() == false)
				{
					std::string filePathStr = filePath.ToStdString();
					std::ifstream inputFile(filePathStr, std::ios::binary);
					if (inputFile.fail())
					{
						textOutput.Append(wxString::Format(wxT("Invalid stream for: '%s'. Exiting."), filePath));
						finished(pParent, finishedEvent, -1, textOutput, pListDataItems);
						return;
					}
					wxString fileName = wxFileNameFromPath(filePath);
					std::string fileNameStr = fileName.ToStdString();

					// Resolve "most vexing parse" and read all byte content from file stream to vector
					std::vector<char> data((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
					uint64_t len = data.size();

					// Make file hash.
					uint64_t seed = 1;
					uint64_t hashParts[2];
					MurmurHash3_x64_128(&data[0], len, seed, hashParts);

					std::string hash = std::to_string(hashParts[0]);
					hash += std::to_string(hashParts[1]);

					// Put file name into dictionary.
					if (dictionaries[counter].find(hash) != dictionaries[counter].end())
						dictionaries[counter][hash].push_back(fileNameStr);
					else
					{
						std::vector<std::string> filePaths = std::vector<std::string>();
						filePaths.push_back(fileNameStr);
						dictionaries[counter].insert(std::make_pair(hash, filePaths));
					}

					inputFile.close();
					data.clear();
				}
			}
		}
	}

	status(pParent, statusEvent, wxT("Detecting file matches and files unique for directory 1..."));

	int matchCount = 0, directory1UniqueCount = 0, directory2UniqueCount = 0;
	std::unordered_set<std::string> keysInDictionary1;
	std::map<std::string, std::vector<std::string>>::iterator itDict;

	// Compare hashes to find matched files and unique files in directory 1
	for (itDict = dictionaries[0].begin(); itDict != dictionaries[0].end(); itDict++)
	{
        if (isCancelled(pParent, statusEvent))
        {
            FreeResources(pListDataItems);
            return;
        }

		ListDataItem listDataItem = ListDataItem();
		std::string key = itDict->first;
		listDataItem.Hash = key;

		// Matched file
		if (dictionaries[1].find(key) != dictionaries[1].end())
		{
			listDataItem.Hash = itDict->first;
			listDataItem.FilesFromDirectory1 = itDict->second;

			std::vector<std::string> matchedFiles = dictionaries[1][key];
			listDataItem.FilesFromDirectory2 = matchedFiles;
			keysInDictionary1.insert(key);

			matchCount++;
		}
		else
		{
			// Unique file in directory 1
			listDataItem.FilesFromDirectory1 = itDict->second;
			keysInDictionary1.insert(key);

            directory1UniqueCount++;
		}
		pListDataItems->push_back(listDataItem);
	}

    status(pParent, statusEvent, wxT("Detecting files unique for directory 2..."));

	// Compare hashes to find unique files in directory 2
	for (itDict = dictionaries[1].begin(); itDict != dictionaries[1].end(); itDict++)
	{
        if (isCancelled(pParent, statusEvent))
        {
            FreeResources(pListDataItems);
            return;
        }

		// Unique file in directory 2
		if (keysInDictionary1.find(itDict->first) == keysInDictionary1.end())
		{
			ListDataItem listDataItem = ListDataItem();
			listDataItem.Hash = itDict->first;
			listDataItem.FilesFromDirectory2 = itDict->second;

			directory2UniqueCount++;

			pListDataItems->push_back(listDataItem);
		}
	}

	status(pParent, statusEvent, wxT("Creating clipboard text..."));

	// Build CSV textual description
	for (auto listDataItem : *pListDataItems)
	{
        if (isCancelled(pParent, statusEvent))
        {
            FreeResources(pListDataItems);
            return;
        }

		textOutput.Append(wxString::Format(wxT("%s,"), listDataItem.Hash));

		for (int i = 0; i < listDataItem.FilesFromDirectory1.size(); i++)
			textOutput.Append(wxString::Format(wxT(" \"%s\""), listDataItem.FilesFromDirectory1[i]));

		textOutput.Append(",");

		for (int i = 0; i < listDataItem.FilesFromDirectory2.size(); i++)
			textOutput.Append(wxString::Format(wxT(" \"%s\""), listDataItem.FilesFromDirectory2[i]));

		textOutput.Append("\n");
	}

    status(pParent, statusEvent, wxString::Format("Comparison finished - files matched: %d, unique files in directory 1: %d, unique files in directory 2: %d - please note that 'unique files' exclude from count binary copies of a same file in same directory", matchCount, directory1UniqueCount, directory2UniqueCount));
    finished(pParent, finishedEvent, 0, textOutput, pListDataItems);
}

void FreeResources(std::vector<ListDataItem> *pListDataItems)
{
    delete pListDataItems;
    pListDataItems = NULL;
}