#ifndef _LISTDATAITEM_H_
#define _LISTDATAITEM_H_

#include <vector>
#include <iostream>

struct ListDataItem
{
public:
	std::string Hash;
	std::vector<std::string> FilesFromDirectory1;
	std::vector<std::string> FilesFromDirectory2;

	ListDataItem()
	{
	}
};

#endif
