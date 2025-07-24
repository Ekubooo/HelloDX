#include "GameApp.h"
#include <iostream>

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE prevInstance,
    _In_ LPSTR cmdLine, _In_ int showCmd)
{
    std::cout<<"Start"<<std::endl;
    // discard these paramart
    UNREFERENCED_PARAMETER(prevInstance);
    UNREFERENCED_PARAMETER(cmdLine);
    UNREFERENCED_PARAMETER(showCmd);
    // allowing run mem allocated and leak detection in debug version
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    GameApp theApp(hInstance, L"YUTAKA Engine", 1920, 1080);

    if (!theApp.Init())
        return 0;

    return theApp.Run();
}




