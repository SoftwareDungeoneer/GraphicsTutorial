#include "util.h"

std::vector<BYTE> LoadFile(LPCTSTR filename)
{
	HANDLE hFile = CreateFileW(
		filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	std::vector<BYTE> bytes(GetFileSize(hFile, nullptr));

	DWORD dwRead{ 0 };
	ReadFile(hFile, bytes.data(), (DWORD)bytes.size(), &dwRead, nullptr);

	CloseHandle(hFile);

	return bytes;
}

std::vector<std::filesystem::directory_entry>
GetPathEntries(const std::string& path, bool includeDirs)
{
	namespace fs = std::filesystem;

	decltype(GetPathEntries("", false)) out;

	for (const auto& entry : fs::directory_iterator(path)) {
		if (!includeDirs && entry.is_directory())
			continue;
		out.emplace_back(entry);
	}

	return out;
}
