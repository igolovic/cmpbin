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

	/*ListDataItem(std::wstring hash, std::vector<std::wstring> filesFromDirectory1, std::vector<std::wstring> filesFromDirectory2)
	{
		Hash = hash;
		FilesFromDirectory1 = filesFromDirectory1;
		FilesFromDirectory2 = filesFromDirectory2;
	}*/
};