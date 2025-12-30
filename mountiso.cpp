/* 
 * MultiOS-USB © 2020-2024 MexIT
 * https://gitlab.com/MultiOS-USB/mountiso
 * https://github.com/Mexit/MultiOS-USB
 * Read LICENSE file for details 
 */

/*
 * Mount ISO to virtual drive
 */

#include <windows.h>
#include <initguid.h>
#include <virtdisk.h>
#include <algorithm> 
#include <fstream>
#include <iostream>
#include <string>
#include <tchar.h>

void displayError(const wchar_t* message) {
	MessageBox(NULL, message, L"mountiso", MB_ICONERROR | MB_OK);
	exit(1);
}

int _tmain() {

	VIRTUAL_STORAGE_TYPE isoImageType;
	memset(&isoImageType, 0, sizeof(isoImageType));
	isoImageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_ISO;
	isoImageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT;

	ATTACH_VIRTUAL_DISK_PARAMETERS attachParameters;
	memset(&attachParameters, 0, sizeof(attachParameters));
	attachParameters.Version = ATTACH_VIRTUAL_DISK_VERSION_1;

	OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
	memset(&openParameters, 0, sizeof(openParameters));
	openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_1;
	HANDLE h;

	std::ifstream file("grubenv");
	if (!file.is_open()) {
		displayError(L"Error: Unable to open grubenv file!");
	}

	std::wstring isoPath;
	std::string line;
	while (std::getline(file, line)) {
		if (line.find("iso_path=") != std::string::npos) {
			isoPath = std::wstring(line.begin() + 10, line.end());
			break;
		}
	}
	file.close();

	if (isoPath.empty()) {
		displayError(L"Error: ISO file path not found in grubenv file!");
	}
	else {
		std::replace(isoPath.begin(), isoPath.end(), '/', '\\');
	}

	for (char driveLetter = 'A'; driveLetter <= 'Z'; ++driveLetter) {
		std::wstring drivePath = std::wstring(1, driveLetter) + L":\\";
		std::wstring fullIsoPath = drivePath + isoPath;

		std::ifstream file(fullIsoPath);
		if (file.good()) {
			if (OpenVirtualDisk(&isoImageType, fullIsoPath.c_str(), VIRTUAL_DISK_ACCESS_READ, OPEN_VIRTUAL_DISK_FLAG_NONE, &openParameters, &h) == ERROR_SUCCESS) {
				if (AttachVirtualDisk(h, NULL, ATTACH_VIRTUAL_DISK_FLAG_READ_ONLY | ATTACH_VIRTUAL_DISK_FLAG_PERMANENT_LIFETIME, 0, &attachParameters, NULL) == ERROR_SUCCESS) {
					break;
				}
				else {
					displayError(L"Error: AttachVirtualDisk failed!");
				}
			}
			else {

				displayError(L"Error: OpenVirtualDisk failed!");
			}
		}
	}
	file.close();
	return 0;
}