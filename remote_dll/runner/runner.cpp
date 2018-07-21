#include <Windows.h>
#include <iostream>

using namespace std;

int main()
{
    printf("hello\n");

    HANDLE proc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, 10948);
    if (proc == NULL) {
        cout << "Failed OpenProcess" << endl;
        return -1;
    }

    void* remote_addr = VirtualAllocEx(proc, 0, 1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (remote_addr == NULL) {
        cout << "Failed VirtualAllocEx" << endl;
        return -1;
    }

    const char* dll_name = "C:\\Projects\\my_lessons\\remote_dll\\Win32_Debug\\remote_dll.dll";
    SIZE_T len = strlen(dll_name);
    SIZE_T written = 0;

    if (WriteProcessMemory(proc, remote_addr, dll_name, len+1, &written) == 0) {
        cout << "Failed WriteProcessMemory" << endl;
        return -1;
    }

    DWORD tid = 0;
    HANDLE th = CreateRemoteThread(proc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, remote_addr, 0, &tid);
    if (th == NULL) {
        cout << "Failed CreateRemoteThread" << endl;
        return -1;
    }

    cout << "done!" << endl;


    return 0;
}

