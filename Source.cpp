#include "Header.h"
DWORD _getProcPID(char* procName) {
	PROCESSENTRY32 proc;
	HANDLE sShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (sShot == INVALID_HANDLE_VALUE) {
		printf_s("_getProcPID Error: %d\n", GetLastError());
		return -2;
	}
	proc.dwSize = sizeof(PROCESSENTRY32);
	Process32First(sShot, &proc);
	do
		if (!strcmp(proc.szExeFile, procName))
			return proc.th32ProcessID;
	while (Process32Next(sShot, &proc));
	return -1;
}
DWORD _getProcModule(char* _module, DWORD procPID) {
	MODULEENTRY32 mod;
	HANDLE sShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, procPID);

	if (sShot == INVALID_HANDLE_VALUE) {
		printf_s("_getProcModule Error: %d\n", GetLastError());
		return -2;
	}
		
	mod.dwSize = sizeof(MODULEENTRY32);
	Module32First(sShot, &mod);
	do
		if (!strcmp(mod.szModule, _module))
			return (DWORD)mod.modBaseAddr;
		else
			printf_s("Module: %s\n", mod.szModule);
	while (Module32Next(sShot, &mod));

	return 0;
}
DWORD _searchVal(int value,HANDLE hProc, DWORD startAddr = 0x400000, DWORD endAddr = 0x1FF0000) {
	int temp = 0; int found = 0;static DWORD arrList[1500];
	for (; startAddr < endAddr; startAddr++) {
		ReadProcessMemory(hProc, (LPVOID)startAddr, &temp, 4, NULL);
		if (temp == value) {
			printf_s("Adding 0x%X (%d) to arrList[%d]\n", startAddr, temp, found);
			arrList[found] = startAddr;
			found++;
		}
	}
	goto1:printf_s("%d Adresses with Value %d, Continue? Type or -1\n", found, value);
	scanf_s("%d", &value); found = 0;
	if (value > -1) {
		for (int i = 0; i < 1500; i++) {
			if (arrList[i] == NULL)
				continue;
			ReadProcessMemory(hProc, (LPVOID)arrList[i], &temp, 4, NULL);
			if (temp != value)
				arrList[i] = NULL;
			else {
				printf_s("Still Same 0x%X : %d\n", arrList[i], temp);
				arrList[found] = arrList[i];
				if(found != i)
					arrList[i] = NULL;
				found++;
			}
		}
		goto goto1;
	}
	else
		printf_s("Returning..\n");
	return arrList[0];
}

int main() {
	int temp = 0; DWORD pid = _getProcPID("zula.exe"); DWORD baseAddr = _getProcModule("d3d9.dll", pid);
	//WriteProcessMemory(hWnd, (LPVOID)(baseAddr + 0x23b7e), new BYTE(0x1), 1, NULL);
	//printf_s("Patched :)\n");
	SetConsoleTitle("NVIDIA");
	printf_s("Result: Pid: 0x%X | baseAddr: 0x%X\n", pid, baseAddr);
	int value = 0;
	goto2:printf_s("Enter Va:\n");
	scanf_s("%d", &value);
	HANDLE hWnd = OpenProcess(PROCESS_VM_READ, 0, pid);
	DWORD addr = _searchVal(value, hWnd);
	printf_s("Enter New:\n");
	scanf_s("%d", &value);
	hWnd = OpenProcess(PROCESS_VM_WRITE, 0, pid);
	WriteProcessMemory(hWnd, (LPVOID)(addr), (LPVOID)&value, 4, NULL);
	goto goto2;
	getchar();
	return 0;
}