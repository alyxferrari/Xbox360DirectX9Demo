#include "stdafx.h"
#include "XboxController.h"
#define PI 3.14159265358979323846264
Direct3D* g_d3d;
D3DDevice* g_d3dDevice;
D3DVertexBuffer* g_vertexBuffer;
D3DVertexDeclaration* g_vertexDeclaration;
D3DVertexShader* g_vertexShader;
D3DPixelShader* g_pixelShader;
XMMATRIX g_matrixRotation;
XMMATRIX g_matrixTranslation;
XMMATRIX g_matrixWorld;
XMMATRIX g_matrixProjection;
XMMATRIX g_matrixView;
float g_xRotation = 0.0f;
float g_yRotation = 0.0f;
float g_xTranslation = 0.0f;
float g_yTranslation = 0.0f;
float g_zTranslation = -5.0f;
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
struct DemoVertex {
	float x, y, z;
	unsigned long color;
};
const DemoVertex vertices[] = {
	// floor
	{-3.0f, -1.0f, 3.0f, D3DCOLOR_XRGB(160, 160, 160)},
	{-3.0f, -1.0f, -3.0f, D3DCOLOR_XRGB(160, 160, 160)},
	{3.0f, -1.0f, 3.0f, D3DCOLOR_XRGB(160, 160, 160)},
	{-3.0f, -1.0f, -3.0f, D3DCOLOR_XRGB(160, 160, 160)},
	{3.0f, -1.0f, 3.0f, D3DCOLOR_XRGB(160, 160, 160)},
	{3.0f, -1.0f, -3.0f, D3DCOLOR_XRGB(160, 160, 160)},
	// triangle
	{-1.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 0)},
	{0.0f, 1.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 0)},
	{1.0f, -1.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255)},
};
void initScene() {
	ID3DXBuffer* vertexShaderCode;
	ID3DXBuffer* vertexErrorMessage;
	D3DXCompileShader(g_vertexShaderSource, (unsigned int) strlen(g_vertexShaderSource), NULL, NULL, "main", "vs_2_0", 0, &vertexShaderCode, &vertexErrorMessage, NULL);
	g_d3dDevice->CreateVertexShader((unsigned long*) vertexShaderCode->GetBufferPointer(), &g_vertexShader);
	ID3DXBuffer* pixelShaderCode;
	ID3DXBuffer* pixelErrorMessage;
	D3DXCompileShader(g_pixelShaderSource, (unsigned int) strlen(g_pixelShaderSource), NULL, NULL, "main", "ps_2_0", 0, &pixelShaderCode, &pixelErrorMessage, NULL);
	g_d3dDevice->CreatePixelShader((DWORD*) pixelShaderCode->GetBufferPointer(), &g_pixelShader);
	D3DVERTEXELEMENT9 vertexElements[3] = {
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
		D3DDECL_END(),
	};
	g_d3dDevice->CreateVertexDeclaration(vertexElements, &g_vertexDeclaration);
	g_d3dDevice->CreateVertexBuffer(sizeof(vertices), D3DUSAGE_WRITEONLY, NULL, D3DPOOL_MANAGED, &g_vertexBuffer, NULL);
	void* data;
	g_vertexBuffer->Lock(0, 0, &data, 0);
	memcpy(data, vertices, sizeof(vertices));
	g_vertexBuffer->Unlock();
	g_matrixWorld = XMMatrixIdentity();
	float aspect = g_widescreen ? 16.0f/9.0f : 4.0f/3.0f;
	g_matrixProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, aspect, 1.0f, 200.0f);
	XMVECTOR eye = {g_xTranslation, 0.0f, g_zTranslation, 0.0f};
	XMVECTOR focus = {sin(g_yRotation)+g_xTranslation, sin(g_xRotation), (cos(g_xRotation)*cos(g_yRotation))+g_zTranslation, 0.0f};
	XMVECTOR up = {0.0f, 1.0f, 0.0f, 0.0f};
	g_matrixView = XMMatrixLookAtLH(eye, focus, up);
}
void initD3D() {
	g_d3d = Direct3DCreate9(D3D_SDK_VERSION);
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	XVIDEO_MODE videoMode;
	XGetVideoMode(&videoMode);
	g_widescreen = videoMode.fIsWideScreen;
	d3dpp.BackBufferWidth = min(videoMode.dwDisplayWidth, 1280);
	d3dpp.BackBufferHeight = min(videoMode.dwDisplayHeight, 720);
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	g_d3d->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_d3dDevice);
	g_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_d3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
}
void render() {
	g_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(52, 182, 142), 1.0f, 0);
	g_d3dDevice->SetVertexDeclaration(g_vertexDeclaration);
	g_d3dDevice->SetStreamSource(0, g_vertexBuffer, 0, sizeof(DemoVertex));
	g_d3dDevice->SetVertexShader(g_vertexShader);
	g_d3dDevice->SetPixelShader(g_pixelShader);
	XMMATRIX matrixWVP = g_matrixWorld * g_matrixView * g_matrixProjection;
	g_d3dDevice->SetVertexShaderConstantF(0, (float*) &matrixWVP, 4);
	g_d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 3);
	g_d3dDevice->Present(NULL, NULL, NULL, NULL);
}
void updateInputs() {
	g_shouldBreak = (getControllerButtons() & BUTTON_BACK) > 0;
	ThumbStickState* left = getLeftThumbStick();
	ThumbStickState* right = getRightThumbStick();
	if (left->x < -8192 || left->x > 8192) {
		g_xTranslation += 0.1f * cos(-g_yRotation) * ((float) left->x / 32768.0);
		g_zTranslation += 0.1f * sin(-g_yRotation) * ((float) left->x / 32768.0);
	}
	if (left->y < -8192 || left->y > 8192) {
		g_xTranslation += 0.1f * sin(g_yRotation) * ((float) left->y / 32768.0);
		g_zTranslation += 0.1f * cos(g_yRotation) * ((float) left->y / 32768.0);
	}
	if (right->x < -8192) {
		g_yRotation -= 0.03f;
	} else if (right->x > 8192) {
		g_yRotation += 0.03f;
	}
	if (right->y < -8192) {
		g_xRotation -= 0.03f;
		g_xRotation = g_xRotation > XM_PI / 2.0f ? XM_PI / 2.0f : g_xRotation;
		g_xRotation = g_xRotation < -XM_PI / 2.0f ? -XM_PI / 2.0f : g_xRotation;
	} else if (right->y > 8192) {
		g_xRotation += 0.03f;
		g_xRotation = g_xRotation > XM_PI / 2.0f ? XM_PI / 2.0f : g_xRotation;
		g_xRotation = g_xRotation < -XM_PI / 2.0f ? -XM_PI / 2.0f : g_xRotation;
	}
}
void updateScene() {
	XMVECTOR eye = {g_xTranslation, 0.0f, g_zTranslation, 0.0f};
	XMVECTOR focus = {sin(g_yRotation)+g_xTranslation, sin(g_xRotation), (cos(g_xRotation)*cos(g_yRotation))+g_zTranslation, 0.0f};
	XMVECTOR up = {0.0f, 1.0f, 0.0f, 0.0f};
	g_matrixView = XMMatrixLookAtLH(eye, focus, up);
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
		updateScene();
		render();
		if (g_shouldBreak) break;
	}
	deinitD3D();
}