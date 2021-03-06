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
    CmpbinFrame *pParent,
    wxCommandEvent statusEvent,
    wxCommandEvent finishedEvent,
    void (*status)(CmpbinFrame*, wxCommandEvent, wxString),
    void (*finished)(CmpbinFrame*, wxCommandEvent, int, wxString, std::vector<ListDataItem>*),
    bool (*isCancelled)(CmpbinFrame*, wxCommandEvent)
    )
{
	const wxString dirPaths[2] = { dirPath1 , dirPath2 };
    wxString textOutput;
	std::vector<ListDataItem> *pListDataItems_unique1 = new std::vector<ListDataItem>();
	std::vector<ListDataItem> *pListDataItems_unique2 = new std::vector<ListDataItem>();
	std::vector<ListDataItem> *pListDataItems_matched = new std::vector<ListDataItem>();
	std::vector<ListDataItem> *pListDataItems = new std::vector<ListDataItem>();

    status(pParent, statusEvent, wxT("Starting comparison..."));

	// Create dictionaries with file sizes and file names
	std::map<unsigned long, std::vector<std::string>> dictionaries_fileSize_filePath[2];
	for (int counterDict = 0; counterDict < 2; counterDict++)
	{
		wxString statusMessage = wxString::Format(wxT("Creating size-name dictionary for '%s'"), dirPaths[counterDict]);
		status(pParent, statusEvent, statusMessage);

		const wxString &dirPath = dirPaths[counterDict];
		wxDir dir;
		dir.Open(dirPath);
		if (dir.IsOpened())
		{
			wxArrayString *files = new wxArrayString;

			// Get all files for diresctory
			dir.GetAllFiles(dirPath, files, wxEmptyString, wxDIR_FILES);
			for (size_t counterFile = 0; counterFile < files->Count(); counterFile++)
			{
				if (isCancelled(pParent, statusEvent))
				{
					FreeResources(pListDataItems);
					return;
				}

				wxString filePath = files->Item(counterFile);
				if (filePath.empty() == false)
				{
					std::string filePathStr = filePath.ToStdString();

					wxStructStat strucStat;
					wxStat(filePathStr, &strucStat);
					unsigned long filesize = (unsigned long)strucStat.st_size;

					// If that size is already in dictionary, add it to vector.
					if (dictionaries_fileSize_filePath[counterDict].find(filesize) != dictionaries_fileSize_filePath[counterDict].end())
						dictionaries_fileSize_filePath[counterDict][filesize].push_back(filePathStr);
					else
					{
						std::vector<std::string> filePathStrs = std::vector<std::string>();
						filePathStrs.push_back(filePathStr);
						dictionaries_fileSize_filePath[counterDict].insert(std::make_pair(filesize, filePathStrs));
					}
				}
			}
		}
	}

	status(pParent, statusEvent, wxT("Detecting - by file size - unique files and matches"));

	std::unordered_set<unsigned long> fileSizesMatched;
	std::map<unsigned long, std::vector<std::string>>::iterator itDict_fileSize_filePath;

	// By filesize determine: unique files from directory 1, matched files
	std::vector<std::string> nonUniqueFileLists[2];
	std::map<std::string, unsigned long> dictionary_nonUniqueFileSize_filePath_fileSize;
	for (itDict_fileSize_filePath = dictionaries_fileSize_filePath[0].begin(); itDict_fileSize_filePath != dictionaries_fileSize_filePath[0].end(); itDict_fileSize_filePath++)
	{
		if (isCancelled(pParent, statusEvent))
		{
			FreeResources(pListDataItems);
			return;
		}

		ListDataItem listDataItem = ListDataItem();
		unsigned long fileSize = itDict_fileSize_filePath->first;

		if (dictionaries_fileSize_filePath[1].find(fileSize) != dictionaries_fileSize_filePath[1].end())
		{
			// Add matched files by size from directory 1
			auto matchedFilePathsDir1 = itDict_fileSize_filePath->second;
			for (auto matchedFilePath : matchedFilePathsDir1)
			{
				nonUniqueFileLists[0].push_back(matchedFilePath);
				dictionary_nonUniqueFileSize_filePath_fileSize.insert(std::make_pair(matchedFilePath, fileSize));
			}

			// Add matched files by size from directory 2
			std::vector<std::string> matchedFilePathsDir2 = dictionaries_fileSize_filePath[1][fileSize];
			fileSizesMatched.insert(fileSize);

			for (auto matchedFilePath : matchedFilePathsDir2)
			{
				nonUniqueFileLists[1].push_back(matchedFilePath);
				dictionary_nonUniqueFileSize_filePath_fileSize.insert(std::make_pair(matchedFilePath, fileSize));
			}
		}
		else
		{
			listDataItem.FileSize = fileSize;
			listDataItem.FilesFromDirectory1 = itDict_fileSize_filePath->second;

			// Unique file size in directory 1
			pListDataItems_unique1->push_back(listDataItem);
		}
	}

	// By file size determine: unique files from directory 2
	for (itDict_fileSize_filePath = dictionaries_fileSize_filePath[1].begin(); itDict_fileSize_filePath != dictionaries_fileSize_filePath[1].end(); itDict_fileSize_filePath++)
	{
		if (isCancelled(pParent, statusEvent))
		{
			FreeResources(pListDataItems);
			return;
		}

		// Unique file in directory 2
		if (fileSizesMatched.find(itDict_fileSize_filePath->first) == fileSizesMatched.end())
		{
			ListDataItem listDataItem = ListDataItem();
			listDataItem.FileSize = itDict_fileSize_filePath->first;
			listDataItem.FilesFromDirectory2 = itDict_fileSize_filePath->second;

			pListDataItems_unique2->push_back(listDataItem);
		}
	}

	// Create hashes for files from directories 1 and 2 that have same file sizes
	std::map<std::string, std::vector<std::string>> dictionaries_fileHash_fileName[2];
	for (int counterDict = 0; counterDict < 2; counterDict++)
	{
		wxString statusMessage = wxString::Format(wxT("Creating hash-name dictionary for '%s'"), dirPaths[counterDict]);
		status(pParent, statusEvent, statusMessage);

		for (size_t counterFilePath = 0; counterFilePath < nonUniqueFileLists[counterDict].size(); counterFilePath++)
		{
			if (isCancelled(pParent, statusEvent))
			{
				FreeResources(pListDataItems);
				return;
			}

			// Reporting progress in GUI incurs performance penalty, comment it out
			//status(pParent, statusEvent, wxString::Format(wxT("%s - hashing file %zu of %zu"), dirPath, i + 1, files->Count()));

			// Get file path
			wxString filePath = nonUniqueFileLists[counterDict][counterFilePath];
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

				// Resolve "most vexing parse" and read all byte content from file stream to vector
				std::vector<char> data((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
				uint64_t len = data.size();

				// Make file hash.
				uint64_t seed = 1;
				uint64_t hashParts[2];
				MurmurHash3_x64_128(&data[0], len, seed, hashParts);

				std::string fileHash = std::to_string(hashParts[0]);
				fileHash += std::to_string(hashParts[1]);

				// Put file name into dictionary.
				if (dictionaries_fileHash_fileName[counterDict].find(fileHash) != dictionaries_fileHash_fileName[counterDict].end())
					dictionaries_fileHash_fileName[counterDict][fileHash].push_back(filePathStr);
				else
				{
					std::vector<std::string> filePathStrs = std::vector<std::string>();
					filePathStrs.push_back(filePathStr);
					dictionaries_fileHash_fileName[counterDict].insert(std::make_pair(fileHash, filePathStrs));
				}

				inputFile.close();
				data.clear();
			}
		}
	}

	status(pParent, statusEvent, wxT("Detecting - by file hash - unique files and matches"));


	std::unordered_set<std::string> fileHashesMatched;
	std::map<std::string, std::vector<std::string>>::iterator itDict_fileHash_fileName;

	// Compare hashes to find matched files and unique files in directory 1
	for (itDict_fileHash_fileName = dictionaries_fileHash_fileName[0].begin(); itDict_fileHash_fileName != dictionaries_fileHash_fileName[0].end(); itDict_fileHash_fileName++)
	{
        if (isCancelled(pParent, statusEvent))
        {
            FreeResources(pListDataItems);
            return;
        }

		ListDataItem listDataItem = ListDataItem();
		std::string filePath = itDict_fileHash_fileName->second.front();
		if (dictionary_nonUniqueFileSize_filePath_fileSize.find(filePath) != dictionary_nonUniqueFileSize_filePath_fileSize.end())
            listDataItem.FileSize = dictionary_nonUniqueFileSize_filePath_fileSize[filePath];

		std::string fileHash = itDict_fileHash_fileName->first;
		listDataItem.FileHash = fileHash;
		listDataItem.FilesFromDirectory1 = itDict_fileHash_fileName->second;

		// Matched file
		if (dictionaries_fileHash_fileName[1].find(fileHash) != dictionaries_fileHash_fileName[1].end())
		{
			std::vector<std::string> matchedFiles = dictionaries_fileHash_fileName[1][fileHash];
			listDataItem.FilesFromDirectory2 = matchedFiles;
			fileHashesMatched.insert(fileHash);
			pListDataItems_matched->push_back(listDataItem);
		}
		else
		{
			// Unique file in directory 1
			pListDataItems_unique1->push_back(listDataItem);
		}
	}

    status(pParent, statusEvent, wxT("Detecting files unique for directory 2..."));

	// Compare hashes to find unique files in directory 2
	for (itDict_fileHash_fileName = dictionaries_fileHash_fileName[1].begin(); itDict_fileHash_fileName != dictionaries_fileHash_fileName[1].end(); itDict_fileHash_fileName++)
	{
        if (isCancelled(pParent, statusEvent))
        {
            FreeResources(pListDataItems);
            return;
        }

		// Unique file in directory 2
		if (fileHashesMatched.find(itDict_fileHash_fileName->first) == fileHashesMatched.end())
		{
			ListDataItem listDataItem = ListDataItem();

			for (auto fileName : itDict_fileHash_fileName->second)
				if (dictionary_nonUniqueFileSize_filePath_fileSize.find(fileName) != dictionary_nonUniqueFileSize_filePath_fileSize.end())
				{
					listDataItem.FileSize = dictionary_nonUniqueFileSize_filePath_fileSize[fileName];
					break;
				}

			listDataItem.FileHash = itDict_fileHash_fileName->first;
			listDataItem.FilesFromDirectory2 = itDict_fileHash_fileName->second;

			pListDataItems_unique2->push_back(listDataItem);
		}
	}

	status(pParent, statusEvent, wxT("Creating clipboard text..."));

	// Add all unique and matched files to single list
	size_t countUnique1 = 0, countUnique2 = 0, countMatched1 = 0, countMatched2 = 0;
	for (auto item : *pListDataItems_unique1)
	{
		pListDataItems->push_back(item);
		countUnique1 += item.FilesFromDirectory1.size();
	}

	for (auto item : *pListDataItems_matched)
	{
		pListDataItems->push_back(item);
		countMatched1 += item.FilesFromDirectory1.size();
		countMatched2 += item.FilesFromDirectory2.size();
	}

	for (auto item : *pListDataItems_unique2)
	{
		pListDataItems->push_back(item);
		countUnique2 += item.FilesFromDirectory2.size();
	}

	// Build CSV textual description
	textOutput.Append("File size;File hash;Directory 1 files;Directory 2 files\n");
	for (auto listDataItem : *pListDataItems)
	{
        if (isCancelled(pParent, statusEvent))
        {
            FreeResources(pListDataItems);
            return;
        }

		textOutput.Append(wxString::Format(wxT("%lu;"), listDataItem.FileSize));

		textOutput.Append(wxString::Format(wxT("%s;"), listDataItem.FileHash));

		for (size_t i = 0; i < listDataItem.FilesFromDirectory1.size(); i++)
			textOutput.Append(wxString::Format(wxT(" \"%s\""), wxFileName(listDataItem.FilesFromDirectory1[i]).GetFullName()));

		textOutput.Append(";");

		for (size_t i = 0; i < listDataItem.FilesFromDirectory2.size(); i++)
			textOutput.Append(wxString::Format(wxT(" \"%s\""), wxFileName(listDataItem.FilesFromDirectory2[i]).GetFullName()));

		textOutput.Append("\n");
	}

    status(pParent, statusEvent, wxString::Format("Comparison :: directory 1 files: unique: %zu, matched: %zu :: directory 2 files: unique: %zu, matched: %zu",
		countUnique1,
		countMatched1,
		countUnique2,
		countMatched2
	));

	finished(pParent, finishedEvent, 0, textOutput, pListDataItems);
}

void FreeResources(std::vector<ListDataItem> *pListDataItems)
{
    delete pListDataItems;
    pListDataItems = NULL;
}
