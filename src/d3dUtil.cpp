#include "d3dUtil.h"


HRESULT CreateShaderFromFile (
    const WCHAR *csoFileNameInOut, 
    const WCHAR *hlslFileName, 
    LPCSTR entryPoint, 
    LPCSTR shaderModel, 
    ID3DBlob **ppBolbOut )
{
    HRESULT hr = S_OK;

    // find vertex shader that was already compiled
    if (csoFileNameInOut && D3DReadFileToBlob(csoFileNameInOut, ppBolbOut) == S_OK)
    {
        return hr;
    }
    else
    {
        DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        // set D3DCOMPILE_DEBUG flag/sign for shader debug information
        // it can improve debug experience
        // but it still allow shader optimistic operation
        dwShaderFlags |= D3DCOMPILE_DEBUG;

        // ban optimistic operation in debug model to avoid some unreasonable Situation
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        ID3DBlob* errorBlob = nullptr;
        hr = D3DCompileFromFile(hlslFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                 entryPoint, shaderModel, dwShaderFlags, 0, ppBolbOut, & errorBlob);
        if (FAILED(hr))
        {
            if (errorBlob != nullptr)
            {
                OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer())); 
            }
            SAFE_RELEASE(errorBlob);
            return hr;
        }

        // if already assign output file name, return the shader byte information
        if (csoFileNameInOut)
        {
            return D3DWriteBlobToFile(*ppBolbOut, csoFileNameInOut, FALSE);
        }
    }
    return hr;
}
