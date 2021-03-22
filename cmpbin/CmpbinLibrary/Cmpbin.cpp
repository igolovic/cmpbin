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

	textOutput.Append("Running comparison...\n");
	textOutput.Append(wxString::Format(wxT("\n1st directory is '%s'\n"), dirPath1));
	textOutput.Append(wxString::Format(wxT("2nd directory is '%s'\n\n"), dirPath2));

	// Create dictionaries with hashes of file contents and file names
	std::map<std::string, std::vector<std::string>> dictionaries[2];
	for (int counter = 0; counter < 2; counter++)
	{
		textOutput.Append(wxString::Format(wxT("Creating hashes for directory '%s'\n"), dirPaths[counter]));

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
                status(pParent, statusEvent, wxString::Format(wxT("%s - hashing file %zu of %zu"), dirPath, i + 1, files->Count()));

				wxString filePath = files->Item(i);
				if (filePath.empty() == false)
				{
					std::string filePathStr = filePath.ToStdString();
					std::ifstream inputFile(filePathStr, std::ios::binary);
					if (inputFile.fail())
					{
						textOutput.Append(wxString::Format(wxT("Invalid stream for: '%s'\nExiting\n"), filePath));
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

	status(pParent, statusEvent, wxT("Detecting files unique for directory 1 and file matches..."));

	// Compare hashes and associate file names
	int matchCount = 0, directory1UniqueCount = 0, directory2UniqueCount = 0;
	std::unordered_set<std::string> keysInDictionary1;
	std::map<std::string, std::vector<std::string>>::iterator itDict;
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
			listDataItem.FilesFromDirectory1 = itDict->second;
			keysInDictionary1.insert(key);

            directory1UniqueCount++;
		}
		pListDataItems->push_back(listDataItem);
	}

    status(pParent, statusEvent, wxT("Detecting files unique for directory 2..."));

	for (itDict = dictionaries[1].begin(); itDict != dictionaries[1].end(); itDict++)
	{
        if (isCancelled(pParent, statusEvent))
        {
            FreeResources(pListDataItems);
            return;
        }

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

	textOutput.Append(wxT("\nFile comparisons using generated hashes:\n"));

	constexpr int filenamePaddedLength = 64;
	wxString blank = L" ";

	textOutput.Append(wxT("\nFile hash\n"));
	textOutput.Append(wxString(wxT("1st directory file(s)")).Pad(filenamePaddedLength - 21, ' ', true));
	textOutput.Append(wxString(wxT("2nd directory file(s)")).Pad(filenamePaddedLength - 21, ' ', false));
	textOutput.Append(wxT("\n"));

	for (auto listDataItem : *pListDataItems) // access by reference to avoid copying
	{
        if (isCancelled(pParent, statusEvent))
        {
            FreeResources(pListDataItems);
            return;
        }

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

    status(pParent, statusEvent, wxString::Format("Comparison finished - files matched: %d, unique files in directory 1: %d, unique files in directory 2: %d", matchCount, directory1UniqueCount, directory2UniqueCount));
    finished(pParent, finishedEvent, 0, textOutput, pListDataItems);
}

void FreeResources(std::vector<ListDataItem> *pListDataItems)
{
    delete pListDataItems;
    pListDataItems = NULL;
}
