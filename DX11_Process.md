## the whole process of dx based graphics engine
### overall initiative DX11
- start with main() function:

```
create GameApp instance     // D3DAPP() or GameApp()
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

GameApp.Run()               // D3DAPP.Run();
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

### Render Pipeline
- start with 



