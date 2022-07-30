#include "stdafx.h"
#include "XboxController.h"
Direct3D* g_d3d;
D3DDevice* g_d3dDevice;
D3DVertexBuffer* g_vertexBuffer;
D3DVertexDeclaration* g_vertexDeclaration;
D3DVertexShader* g_vertexShader;
D3DPixelShader* g_pixelShader;
XMMATRIX g_matrixWorld;
XMMATRIX g_matrixProjection;
XMMATRIX g_matrixView;
bool g_shouldBreak = false;
bool g_widescreen = true;
const char* g_vertexShaderSource =
"float4x4 matWVP : register(c0);             "
"struct VS_IN {                              "
"    float4 ObjPos : POSITION;               " 
"    float4 Color : COLOR;                   "                
"};                                          "
"struct VS_OUT {                             "
"    float4 ProjPos : POSITION;              "
"    float4 Color : COLOR;                   "
"};                                          "
"VS_OUT main(VS_IN In) {                     "
"    VS_OUT Out;                             "
"    Out.ProjPos = mul(matWVP, In.ObjPos);   "
"    Out.Color = In.Color;                   "
"    return Out;                             "
"}                                           ";
const char* g_pixelShaderSource =
"struct PS_IN {                              "
"    float4 Color : COLOR;                   "
"};                                          "
"float4 main(PS_IN In) : COLOR {             "
"    return In.Color;                        "
"}                                           ";
struct HWVertex {
	float position[3];
	DWORD color;
};
void initScene() {
	ID3DXBuffer* vertexShaderCode;
	ID3DXBuffer* vertexErrorMessage;
	HRESULT hr = D3DXCompileShader(g_vertexShaderSource, (unsigned int) strlen(g_vertexShaderSource), NULL, NULL, "main", "vs_2_0", 0, &vertexShaderCode, &vertexErrorMessage, NULL);
	g_d3dDevice->CreateVertexShader((DWORD*) vertexShaderCode->GetBufferPointer(), &g_vertexShader);
	ID3DXBuffer* pixelShaderCode;
	ID3DXBuffer* pixelErrorMessage;
	hr = D3DXCompileShader(g_pixelShaderSource, (unsigned int) strlen(g_pixelShaderSource), NULL, NULL, "main", "ps_2_0", 0, &pixelShaderCode, &pixelErrorMessage, NULL);
	g_d3dDevice->CreatePixelShader((DWORD*) pixelShaderCode->GetBufferPointer(), &g_pixelShader);
	D3DVERTEXELEMENT9 vertexElements[3] = {
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END(),
	};
	g_d3dDevice->CreateVertexDeclaration(vertexElements, &g_vertexDeclaration);
	g_d3dDevice->CreateVertexBuffer(3*sizeof(HWVertex), D3DUSAGE_WRITEONLY, NULL, D3DPOOL_MANAGED, &g_vertexBuffer, NULL);
	HWVertex vertices[] = {
		{0.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255)},
		{-1.0f, 0.5f, 0.0f, D3DCOLOR_XRGB(0, 255, 0)},
		{1.0f, 0.5f, 0.0f, D3DCOLOR_XRGB(255, 0, 0)},
	};
	void* data;
	g_vertexBuffer->Lock(0, 0, &data, 0);
	memcpy(data, vertices, 3*sizeof(HWVertex));
	g_vertexBuffer->Unlock();
	g_matrixWorld = XMMatrixIdentity();
	float aspect = g_widescreen ? 16.0f/9.0f : 4.0f/3.0f;
	g_matrixProjection = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspect, 1.0f, 200.0f);
	XMVECTOR eyePosition = {0.0f, 0.0f, -7.0f, 0.0f};
	XMVECTOR focusPosition = {0.0f, 0.0f, 0.0f, 0.0f};
	XMVECTOR up = {0.0f, 1.0f, 0.0f, 0.0f};
	g_matrixView = XMMatrixLookAtLH(eyePosition, focusPosition, up);
}
HRESULT initD3D() {
	g_d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (!g_d3d) return E_FAIL;
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	XVIDEO_MODE videoMode;
	XGetVideoMode(&videoMode);
	g_widescreen = videoMode.fIsWideScreen;
	d3dpp.BackBufferWidth = min(videoMode.dwDisplayWidth, 1280);
	d3dpp.BackBufferHeight = min(videoMode.dwDisplayHeight, 720);
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	if (FAILED(g_d3d->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_d3dDevice))) {
		return E_FAIL;
	}
	return S_OK;
}
void render() {
	g_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(52, 182, 142), 1.0f, 0);
	g_d3dDevice->SetVertexDeclaration(g_vertexDeclaration);
	g_d3dDevice->SetStreamSource(0, g_vertexBuffer, 0, sizeof(HWVertex));
	g_d3dDevice->SetVertexShader(g_vertexShader);
	g_d3dDevice->SetPixelShader(g_pixelShader);
	XMMATRIX matrixWVP = g_matrixWorld * g_matrixView * g_matrixProjection;
	g_d3dDevice->SetVertexShaderConstantF(0, (float*) &matrixWVP, 4);
	g_d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);
	g_d3dDevice->Present(NULL, NULL, NULL, NULL);
}
void updateInputs() {
	g_shouldBreak = getControllerState() & BUTTON_BACK;
}
void deinitD3D() {
	g_d3dDevice->Release();
	g_d3d->Release();
}
void __cdecl main() {
	initD3D();
	initScene();
	while (true) {
		updateInputs();
		render();
		if (g_shouldBreak) break;
	}
	deinitD3D();
}