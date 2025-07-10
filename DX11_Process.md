## the whole process of dx based graphics engine

### overall initiative DX11
- start with main() function:

```
create GameApp instance     // D3DApp() or GameApp()
    constructor function: initiative and bind instance to global pointer.
    
init and test:
    InitMainWindow();       // win32 windows setting
    InitDirect3D()             
    {    
        create DEVICE and CONTEXT.
        test DRIVER, FEATURE and setting .
        init the DXGI (DirectX Graphics Infrastructure).
        init DXGI swap chain:
            if D3D11.1: CreateSwapChainForHwnd();
            else D3D11: CreateSwapChainForHwnd();
        OnResize()  
            release resource of RP.
            reset swap chain.
            recreate render target view.
            CreateTexture2D();      // depth buffer
            CreateDepthStencilView(); 
            OMSetRenderTargets();   // merged view (trarget and depth buffer)
            RSSetViewports();       // viewport setting
    }

GameApp::Run()               // D3DApp.Run();
    timer reset.
    while loop:
        timer.tick();
        CalculateFrameStats();
        DrawScene()
            ClearRenderTargetView();
            ClearDepthStencilView();
            Present();      // swap the chain and present

instance destruction        // ~D3DApp();
```


### Render Pipeline and ImGui 
- start with main function

```
GameApp(): D3DApp()         // init 
   D3DApp global pointer bind this;
    
GameApp::Init()
    D3DApp::Init();         // Inheritance
        InitMainWindow();       // win32 windows setting
        InitDirect3D()      
            DEVICE, CONTEXT, DRIVER, 
            FEATURE, DXGI, SWAP CHAIN;
            OnResize()  
        InitImGui();
    GameApp::InitEffect()
        ComPtr<ID3DBlob> blob;
        create and compile shader file;
        create and bind layout;
    GameApp::InitResource()
        load mesh;
        // Vertex, Index, Constant Buffer: /////////////////
        buffer description and create buffer;
        input assemble;
        debug test setting;

D3DApp::Run()
    timer reset.
    while loop:
        debug msg detect
        timer.tick();
        CalculateFrameStats();
        ImGui new frame record for dx11 and win32;
        D3DApp::GameApp::UpdateScene();     // GmaeApp/SubClass Implementation.
            ImGui::Begin(), ImGui::End();       // ImGui component instance.
            ImGui io event;                     // mouse and keyboard control.
            update constant buffer data         // ImGui controller modify.
            constance buffer mapping            // write back?
        D3DApp::GameApp::DrawScene()        // GmaeApp/SubClass Implementation.
            ClearRenderTargetView();
            ClearDepthStencilView();
            DrawIndex();                        // index for cube or model.
            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData();    // trigger for Direct3D Draw.
            Present();                          // swap the chain and present.

~GameApp(): ~D3DApp();
```

### Lighting, Geometry and Rasterization state
- start with main function

```
GameApp(): D3DApp()         // init 
   D3DApp global pointer bind this;
    
GameApp::Init()
    D3DApp::Init();         // Inheritance
        InitMainWindow();       // win32 windows setting
        InitDirect3D()      
            DEVICE, CONTEXT, DRIVER, 
            FEATURE, DXGI, SWAP CHAIN;
            OnResize()  
        InitImGui();            // fonts setting here
    GameApp::InitEffect()
        ComPtr<ID3DBlob> blob;
        create and compile shader file;
        create and bind layout; // layout important
    GameApp::InitResource()
        resetMesh();            // init mesh model from geometry.h
        const buffer init/set/create/bind/map/unmap;
        init Rasterizer State DESC;  // RS cull model etc.
        input assemble;

D3DApp::Run()
    timer reset.
    while loop:
        debug, timer tick, Frame Stats and ImGui newFrame(dx11/win32);
        GameApp::D3DApp::UpdateScene();     
            ImGui component(Begin/End);     // ImGui component instance.
                ResetMesh();                    // runtime change Mesh
                RSSetState();                   // runtime change RS
            ImGui Render();
            ImGui io event;                 // mouse and keyboard control.(cancel in c7)
            update/mapping const buffer;    // ImGui controller modify.
        GameApp::D3DApp::DrawScene()        
            ClearRenderTargetView();
            ClearDepthStencilView();
            DrawIndex();                    // index for cube or model.
            ImGui lDX11 DrawData;           // trigger for Direct3D Draw.
            Present();                      // swap the chain and present.

~GameApp(): ~D3DApp();
```