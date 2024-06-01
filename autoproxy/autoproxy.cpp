#include <windows.h>
#include <tchar.h>
#include <stdio.h>

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
void WINAPI ServiceCtrlHandler(DWORD request);
void InstallService();
void DeleteService();
void StartService();
void StopService();

int _tmain(int argc, TCHAR* argv[]) {
    if (argc > 1) {
        if (_tcscmp(argv[1], TEXT("install")) == 0) {
            InstallService();
            return 0;
        }
        if (_tcscmp(argv[1], TEXT("remove")) == 0) {
            DeleteService();
            return 0;
        }
        if (_tcscmp(argv[1], TEXT("start")) == 0) {
            StartService();
            return 0;
        }
        if (_tcscmp(argv[1], TEXT("stop")) == 0) {
            StopService();
            return 0;
        }
    }

    SERVICE_TABLE_ENTRY ServiceTable[] = {
        { (LPWSTR)L"SimpleService",
        (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcher(ServiceTable)) {
        return GetLastError();
    }

    return 0;
}

void InstallService() {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager) {
        _tprintf(TEXT("OpenSCManager failed (%d)\n"), GetLastError());
        return;
    }

    TCHAR szPath[MAX_PATH];
    if (!GetModuleFileName(NULL, szPath, MAX_PATH)) {
        _tprintf(TEXT("GetModuleFileName failed (%d)\n"), GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    SC_HANDLE hService = CreateService(
        hSCManager,
        TEXT("SimpleService"),
        TEXT("Simple Service"),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        szPath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    if (!hService) {
        _tprintf(TEXT("CreateService failed (%d)\n"), GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    _tprintf(TEXT("Service installed successfully\n"));

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

void DeleteService() {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) {
        _tprintf(TEXT("OpenSCManager failed (%d)\n"), GetLastError());
        return;
    }

    SC_HANDLE hService = OpenService(hSCManager, TEXT("SimpleService"), DELETE);
    if (!hService) {
        _tprintf(TEXT("OpenService failed (%d)\n"), GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    if (!DeleteService(hService)) {
        _tprintf(TEXT("DeleteService failed (%d)\n"), GetLastError());
    }
    else {
        _tprintf(TEXT("Service deleted successfully\n"));
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

void StartService() {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) {
        _tprintf(TEXT("OpenSCManager failed (%d)\n"), GetLastError());
        return;
    }

    SC_HANDLE hService = OpenService(hSCManager, TEXT("SimpleService"), SERVICE_START);
    if (!hService) {
        _tprintf(TEXT("OpenService failed (%d)\n"), GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    if (!StartService(hService, 0, NULL)) {
        _tprintf(TEXT("StartService failed (%d)\n"), GetLastError());
    }
    else {
        _tprintf(TEXT("Service started successfully\n"));
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

void StopService() {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager) {
        _tprintf(TEXT("OpenSCManager failed (%d)\n"), GetLastError());
        return;
    }

    SC_HANDLE hService = OpenService(hSCManager, TEXT("SimpleService"), SERVICE_STOP);
    if (!hService) {
        _tprintf(TEXT("OpenService failed (%d)\n"), GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    SERVICE_STATUS status;
    if (!ControlService(hService, SERVICE_CONTROL_STOP, &status)) {
        _tprintf(TEXT("ControlService failed (%d)\n"), GetLastError());
    }
    else {
        _tprintf(TEXT("Service stopped successfully\n"));
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

void WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
    ServiceStatus.dwServiceType = SERVICE_WIN32;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;

    hStatus = RegisterServiceCtrlHandler(TEXT("SimpleService"), (LPHANDLER_FUNCTION)ServiceCtrlHandler);
    if (hStatus == (SERVICE_STATUS_HANDLE)0) {
        return;
    }

    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;

    if (!SetServiceStatus(hStatus, &ServiceStatus)) {
        return;
    }

    while (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
        // Здесь можно добавить основной код службы
        Sleep(1000); // Заглушка, чтобы служба не завершалась мгновенно
    }

    return;
}

void WINAPI ServiceCtrlHandler(DWORD request) {
    switch (request) {
    case SERVICE_CONTROL_STOP:
        ServiceStatus.dwWin32ExitCode = 0;
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        ServiceStatus.dwCheckPoint = 0;
        ServiceStatus.dwWaitHint = 0;
        if (!SetServiceStatus(hStatus, &ServiceStatus)) {
            return;
        }
        return;

    case SERVICE_CONTROL_SHUTDOWN:
        ServiceStatus.dwWin32ExitCode = 0;
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        ServiceStatus.dwCheckPoint = 0;
        ServiceStatus.dwWaitHint = 0;
        if (!SetServiceStatus(hStatus, &ServiceStatus)) {
            return;
        }
        return;

    default:
        break;
    }

    if (!SetServiceStatus(hStatus, &ServiceStatus)) {
        return;
    }
}
