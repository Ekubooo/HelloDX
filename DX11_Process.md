# The process of dx based graphics engine

## 1 overall initiative DX11
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


## 2 Render Pipeline and ImGui 
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

## 3 Lighting, Geometry and Rasterization state
- start with main function

```
GameApp(): D3DApp()         // init 
   D3DApp global pointer bind this;
    
GameApp::Init()
    D3DApp::Init();         // Inheritance
        InitMainWindow();           // win32 windows setting
        InitDirect3D()      
            DEVICE, CONTEXT, DRIVER, 
            FEATURE, DXGI, SWAP CHAIN;
            OnResize();
        InitImGui();                // fonts setting here
    GameApp::InitEffect()
        ComPtr<ID3DBlob> blob;
        create and compile shader file;
        create and bind layout;     // layout important
    GameApp::InitResource()
        resetMesh();                // init mesh model from geometry.h
            INIT and CREATE: vertex and index buffer;
        const buffer init/set/create/bind/map/unmap;
        init Rasterizer State DESC; // RS cull model etc.
        input assemble;

D3DApp::Run()
    timer reset.
    while loop:
        debug, timer tick, Frame Stats and ImGui newFrame(dx11/win32);
        GameApp::D3DApp::UpdateScene();     
            ImGui component(Begin/End);     // ImGui component instance.
                ResetMesh();                    // runtime change Mesh
                    INIT and CREATE: vertex and index buffer;
                RSSetState();                   // runtime change RS
            ImGui Render();
            ImGui io event;                 // mouse and keyboard control.(cancel in c7)
            update/mapping const buffer;    // ImGui controller modify.
        GameApp::D3DApp::DrawScene()        
            ClearRenderTargetView();
            ClearDepthStencilView();
            DrawIndex();                    // index for cube or model.
            ImGui lDX11 DrawData;           // trigger for Direct3D Draw.
            Present();                      // Swap Chain flip and present.
    end loop;

~GameApp(): ~D3DApp();
```



## 4 Texture Loader
- start with main function
- extra content: frame animation demo
- extra content: multiple light source demo

```
GameApp(): D3DApp()         // init 
   D3DApp global pointer bind this;
    
GameApp::Init()
    D3DApp::Init();         // Inheritance
        InitMainWindow();           // win32 windows setting
        InitDirect3D()      
            DEVICE, CONTEXT, DRIVER, FEATURE, DXGI, SWAP CHAIN;
            OnResize();             // resize window and stuff
        InitImGui();                // fonts setting here
    GameApp::InitEffect()
        ComPtr<ID3DBlob> blob;
        create and compile shader file;
        create and bind layout;     // layout is important
    GameApp::InitResource()
        resetMesh();                // init mesh model from geometry.h
            INIT and CREATE: vertex and index buffer;
        const buffer init/set/create/bind/map/unmap;
        INIT Texture and Sampler;
        INIT Rasterizer State;      // RS cull model etc.
        input assemble;
end Init();

D3DApp::Run()
    timer reset;
    while loop:
        debug, timer tick, Frame Stats and ImGui newFrame(dx11/win32);
        GameApp::D3DApp::UpdateScene();     
            ImGui component(Begin/End);     // ImGui component instance.
                ResetMesh();                    // runtime change Mesh
                    INIT and CREATE: vertex and index buffer;
                RSSetState();                   // runtime change RS
            ImGui Render();
            ImGui io event;                 // mouse and keyboard control.(cancel in c7)
            update/mapping const buffer;    // ImGui controller modify.
        GameApp::D3DApp::DrawScene()        
            ClearView();                    // render target view, depgh and stencil
            DrawIndex();                    // index data of model.
            ImGui lDX11 DrawData;           // trigger of Direct3D Draw.
            Present();                      // Swap Chain flip and present.
    end loop;
end run();

~GameApp(): ~D3DApp();
```

## 5 Camera, GameObject and Transform
- start with main function
- gemotry viewer NOT INCLUDE
- Class GameObject
    - DATA: Transform, Mesh, Texture, World Matrix.
    - INIT: Vertex buffer, Index buffer, Const buffer. 
    - FUNC: Update Const buffer, Draw Index.
- Class TransForm:
    - DATA: Scale, Rotation, Position.
    - FUNC: GameObject Transform date read and store.
- Class Camera:
    - DATA: Camera data.
    - FUNC: Data and Movement operation.
    - Descendants subclass of different kind of camera.

- Structure:
```
GameApp(): D3DApp()         // init 
   D3DApp global pointer bind this;
    
GameApp::Init()
    D3DApp::Init();         // Inheritance
        InitMainWindow();           // win32 windows setting
        InitDirect3D()      
            DEVICE, CONTEXT, DRIVER, FEATURE, DXGI, SWAP CHAIN;
            OnResize();             // resize window and stuff
        InitImGui();                // fonts setting here
    GameApp::InitEffect()
        create and compile shader file;
        create and bind layout;     // layout is important
    GameApp::InitResource()
        const buffer init/set/create/bind/map/unmap;
        INIT Texture and Sampler    // for every GameObject
            GameObject.SetBuffer(); 
            GameObject.SetTexture();
            GameObject.GetTransform().SetPosition();
        // INIT Rasterizer State;      // RS cull model etc.
        input assemble;
end Init();

D3DApp::Run()
    timer reset;
    while loop:
        debug, timer tick, Frame Stats and ImGui newFrame(dx11/win32);
        GameApp::D3DApp::UpdateScene();     
            ImGui component(Begin/End/Render);  // Movement controller
            update pos for camera and woodBox;
            update/mapping const buffer;        // ImGui controller modify.
        GameApp::D3DApp::DrawScene()        
            ClearView();                        // render target view, depgh and stencil
            GameObject.Draw();                  // index data of model.
            ImGui DX11 DrawData();              // trigger of Direct3D Draw.
            Present();                          // Swap Chain flip and present.
    end loop;
end run();

~GameApp(): ~D3DApp();

```


## 6 Blending and Render State.
- start with main function
- gemotry viewer NOT INCLUDE
- Class GameObject
    - DATA: Transform, Mesh, Texture, World Matrix.
    - INIT: Vertex buffer, Index buffer, Const buffer. 
    - FUNC: Update Const buffer, Draw Index.
- Class TransForm:
    - DATA: Scale, Rotation, Position.
    - FUNC: GameObject Transform date read and store.
- Class Camera:
    - DATA: Camera data.
    - FUNC: Data and Movement operation.
    - Descendants subclass of different kind of camera.
- Class Render State 
    - ComPtr for memory management.
    - static STATE member.
    - INIT: Blend Description.

- Structure:
```
GameApp(): D3DApp()         // init 
   D3DApp global pointer bind this;
    
GameApp::Init()
    D3DApp::Init();         // Inheritance
        InitMainWindow();           // win32 windows setting
        InitDirect3D()      
            DEVICE, CONTEXT, DRIVER, FEATURE, DXGI, SWAP CHAIN;
            OnResize();             // resize window and stuff
        InitImGui();                // fonts setting here
    GameApp::InitEffect()
        create and compile shader file;
        create and bind layout;     // layout is important
    GameApp::InitResource()
        INIT and SET constant buffer; 
        for: every GameObject       // INIT Texture and Sampler
            GameObject.SetBuffer(); 
            GameObject.SetTexture();
            GameObject.SetMaterial();
            GameObject.GetTransform().SetPosition();
        Input Assemble;
            UPDATE constant buffer;
            INIT and SET RenderStates;
            SET Blend State;
end Init();

D3DApp::Run()
    timer reset;
    while loop:
        debug, timer tick, Frame Stats and ImGui newFrame(dx11/win32);
        GameApp::D3DApp::UpdateScene();     
            SET ImGui component;                // Controller
            update pos for camera and GameObject;
            update/mapping const buffer;       
        GameApp::D3DApp::DrawScene()        
            ClearView();                        // render target view, depgh and stencil
            // Blending draw //////////////////////////////////
            for: every non-transparent GameObject
                GameObject.Draw();                  // index data of model.
            for: every transparent GameObject
                GameObject.Draw();                  // index data of model.
            // Blending end ///////////////////////////////////
            ImGui DX11 DrawData();              // trigger of Direct3D Draw.
            Present();                          // Swap Chain flip and present.
    end loop;
end run();

~GameApp(): ~D3DApp();

```


