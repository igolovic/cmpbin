#ifndef _LISTDATAITEM_H_
#define _LISTDATAITEM_H_

#include <vector>
#include <iostream>

struct ListDataItem
{
public:
	std::wstring Hash;
	std::vector<std::wstring> FilesFromDirectory1;
	std::vector<std::wstring> FilesFromDirectory2;

	ListDataItem()
	{
	}
};

#endif
