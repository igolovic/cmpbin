#ifndef _LISTDATAITEM_H_
#define _LISTDATAITEM_H_

#include <vector>
#include <iostream>

struct ListDataItem
{
public:
	unsigned long FileSize;
	std::string GetFileSizeStr() { return std::to_string(FileSize); }
	std::string FileHash;
	std::vector<std::string> FilesFromDirectory1;
	std::vector<std::string> FilesFromDirectory2;

	ListDataItem()
	{
	}
};

#endif
