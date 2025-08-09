#include "VideoCube.h"

//------------------------------------------------------------------------------------------
CVideoCube::CVideoCube()
{
	pD3DDevice = nullptr;
	pD3DDeviceContext = nullptr;
	pVertexBuffer = nullptr;
	pD3DRenderTargetView = nullptr;
	pVertexShader = nullptr;
	pPixelShader = nullptr;
	pConstantBuffer = nullptr;
	pInputLayout = nullptr;
	pRasterizerState = nullptr;
	ZeroMemory(pNewVertices, 36 * sizeof(TLVERTEX));
	ZeroMemory(&m_ConstantBufferData, sizeof(VS_CONSTANTBUFFER));
	ZeroMemory(&m_SliderValue, 4 * sizeof(float));
	m_Direct3D_On = false;
	m_WidthOnDeviceInit = 0;
	m_HeightOnDeviceInit = 0;
	m_Width = 0;
	m_Height = 0;
	m_Depth = 0;
	m_VertexCount = 0;
	m_Alpha = 1.0f;
	m_Zoom = 0.0f;
	m_Speed = 0.0f;
	m_RotationAxe = 0;
	m_RotationInverted = 0;
	m_RotationDisk = 0;
	m_BackgroundColor = 0;
	m_Angle = 0;
	m_HoldDisk = 0;
	m_Beats = 0;
}
//------------------------------------------------------------------------------------------
CVideoCube::~CVideoCube()
{

}
//------------------------------------------------------------------------------------------
HRESULT VDJ_API CVideoCube::OnLoad()
{
	HRESULT hr = S_FALSE;

	hr = DeclareParameterSlider(&m_SliderValue[0], ID_SLIDER_1, "Transparency", "TR", 1.0f);
	hr = DeclareParameterSlider(&m_SliderValue[1], ID_SLIDER_2, "Zoom", "ZOOM", 0.14f);
	hr = DeclareParameterSlider(&m_SliderValue[2], ID_SLIDER_3, "Speed", "SPEED", 0.08f);
	hr = DeclareParameterSlider(&m_SliderValue[3], ID_SLIDER_4, "BackgroundColor", "COLR", 0.25f);
	hr = DeclareParameterRadio(&m_RotationAxe, ID_RADIO_1, "Rotation X", "ROTX", 0.0f);
	hr = DeclareParameterRadio(&m_RotationAxe, ID_RADIO_2, "Rotation Y", "ROTY", 1.0f);
	hr = DeclareParameterRadio(&m_RotationAxe, ID_RADIO_3, "Rotation Z", "ROTZ", 0.0f);
	hr = DeclareParameterSwitch(&m_RotationInverted, ID_SWITCH_1, "Inverted", "INV", 0.0f);
	hr = DeclareParameterSwitch(&m_RotationDisk, ID_SWITCH_2, "Scratch", "S", 0.0f);
	hr = DeclareParameterSwitch(&m_HoldDisk, ID_SWITCH_3, "Scratch Hold", "SH", 0.0f);
	hr = DeclareParameterSwitch(&m_Beats, ID_SWITCH_4, "Beats", "B", 0.0f);
	
	hr = OnParameter(ID_INIT);

	return S_OK;
}
//------------------------------------------------------------------------------------------
HRESULT VDJ_API CVideoCube::OnGetPluginInfo(TVdjPluginInfo8 *info)
{
	info->Author = "djcel";
	info->PluginName = "VideoCube";
	info->Description = "Your video in a real 3D environment.";
	info->Flags = 0x00; // VDJFLAG_VIDEO_OVERLAY | VDJFLAG_VIDEO_OUTPUTRESOLUTION | VDJFLAG_VIDEO_OUTPUTASPECTRATIO;
	info->Version = "3.0 (64-bit)";

	return S_OK;
}
//------------------------------------------------------------------------------------------
ULONG VDJ_API CVideoCube::Release()
{
	delete this;
	return 0;
}
//------------------------------------------------------------------------------------------
HRESULT VDJ_API CVideoCube::OnParameter(int id)
{
	if (id == ID_INIT)
	{
		for (int i = ID_SLIDER_1; i <= ID_SLIDER_4; i++) OnSlider(i);
	}
	else OnSlider(id);

	if(id == ID_SWITCH_4) OnSlider(ID_SLIDER_3);


	return S_OK;
}

//------------------------------------------------------------------------------------------
void CVideoCube::OnSlider(int id)
{
	switch (id)
	{
		case ID_SLIDER_1:
			m_Alpha = 1.0f - m_SliderValue[0];
			break;

		case ID_SLIDER_2:
			m_Zoom = 30.0f + m_SliderValue[1] * (180.0f - 30.0f);
			break;

		case ID_SLIDER_3:
			if (m_Beats)
			{
				m_Speed = (float) floor(m_SliderValue[2] * MAX_BEATS);
			}
			else
			{
				m_Speed = m_SliderValue[2] * 5.0f;
			}
			break;

		case ID_SLIDER_4:
			if (m_SliderValue[3] < 0.25f) m_BackgroundColor = 0;
			else if (m_SliderValue[3] >= 0.25f && m_SliderValue[3] < 0.5f) m_BackgroundColor = 1;
			else if (m_SliderValue[3] >= 0.5f && m_SliderValue[3] < 0.75f) m_BackgroundColor = 2;
			else m_BackgroundColor = 3;
			break;
	}

}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CVideoCube::OnGetParameterString(int id, char* outParam, int outParamSize)
{
	switch (id)
	{
		case ID_SLIDER_1:
			sprintf_s(outParam, outParamSize, "%.0f%%", (1.0f - m_Alpha) * 100.0f);
			break;

		case ID_SLIDER_2:
			sprintf_s(outParam, outParamSize, "%.0f", m_Zoom);
			break;

		case ID_SLIDER_3:
			if (m_Speed == 0) sprintf_s(outParam, outParamSize, "Off");
			else if (m_Beats) sprintf_s(outParam, outParamSize, "%.0f beat(s)", m_Speed);
			else sprintf_s(outParam, outParamSize, "%.2f", m_Speed * 100.0f);
			break;

		case ID_SLIDER_4:
			if (m_BackgroundColor == 0) sprintf_s(outParam, outParamSize, "None");
			else if (m_BackgroundColor == 1) sprintf_s(outParam, outParamSize, "Black");
			else if (m_BackgroundColor == 2) sprintf_s(outParam, outParamSize, "Blue");
			else sprintf_s(outParam, outParamSize, "White");
			break;
	}

	return S_OK;
}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CVideoCube::OnDeviceInit()
{
	HRESULT hr = S_FALSE;

	m_Direct3D_On = true;
	m_WidthOnDeviceInit = width;
	m_HeightOnDeviceInit = height;
	m_Width = width;
	m_Height = height;
	m_Depth = width;

	// GetDevice() doesn't AddRef(), so we don't need to release pD3DDevice later
	hr = GetDevice(VdjVideoEngineDirectX11, (void**)  &pD3DDevice);
	if(hr!=S_OK || pD3DDevice==nullptr) return S_FALSE;

	hr = Initialize_D3D11(pD3DDevice);
	if (hr != S_OK) return S_FALSE;

	return S_OK;
}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CVideoCube::OnDeviceClose()
{
	Release_D3D11();
	SAFE_RELEASE(pD3DRenderTargetView);
	SAFE_RELEASE(pD3DDeviceContext);
	pD3DDevice = nullptr;
	m_Direct3D_On = false;

	return S_OK;
}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CVideoCube::OnStart() 
{
	m_Angle = 0;
	return S_OK;
}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CVideoCube::OnStop() 
{
	return S_OK;
}
//-------------------------------------------------------------------------------------------
HRESULT VDJ_API CVideoCube::OnDraw()
{
	HRESULT hr = S_FALSE;
	ID3D11ShaderResourceView *pTextureView = nullptr;
	TVertex8* vertices = nullptr;

	if (width != m_Width || height != m_Height)
	{
		OnResizeVideo();
	}

	if (!pD3DDevice) return S_FALSE;

	pD3DDevice->GetImmediateContext(&pD3DDeviceContext);
	if (!pD3DDeviceContext) return S_FALSE;

	pD3DDeviceContext->OMGetRenderTargets(1, &pD3DRenderTargetView, nullptr);
	if (!pD3DRenderTargetView) return S_FALSE;

	// GetTexture() doesn't AddRef, so doesn't need to be released
	hr = GetTexture(VdjVideoEngineDirectX11, (void**) &pTextureView, &vertices);
	if (hr != S_OK) return S_FALSE;

	hr = Rendering_D3D11(pD3DDevice, pD3DDeviceContext, pD3DRenderTargetView, pTextureView, vertices);
	if (hr != S_OK) return S_FALSE;

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT VDJ_API CVideoCube::OnAudioSamples(float* buffer, int nb) 
{ 
	return E_NOTIMPL; 
}
//-----------------------------------------------------------------------
void CVideoCube::OnResizeVideo()
{
	HRESULT hr = S_FALSE;

	m_Width = width;
	m_Height = height;
	m_Depth = width;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::Initialize_D3D11(ID3D11Device* pDevice)
{
	HRESULT hr = S_FALSE;

	hr = Create_VertexShader_D3D11(pDevice);
	if (hr != S_OK) return S_FALSE;

	hr = Create_PixelShader_D3D11(pDevice);
	if (hr != S_OK) return S_FALSE;

	hr = Create_InputLayout_D3D11(pDevice);
	if (hr != S_OK) return S_FALSE;

	hr = Create_VertexBufferDynamic_D3D11(pDevice);
	if (hr != S_OK) return S_FALSE;

	hr = Create_ConstantBufferDynamic_D3D11(pDevice);
	if (hr != S_OK) return S_FALSE;

	hr = Create_RasterizerState_D3D11(pDevice);
	if (hr != S_OK) return S_FALSE;

	return S_OK;
}
//-----------------------------------------------------------------------
void CVideoCube::Release_D3D11()
{
	SAFE_RELEASE(pVertexShader);
	SAFE_RELEASE(pPixelShader);
	SAFE_RELEASE(pInputLayout);
	SAFE_RELEASE(pVertexBuffer);
	SAFE_RELEASE(pConstantBuffer);
	SAFE_RELEASE(pRasterizerState);
}
// -----------------------------------------------------------------------
HRESULT CVideoCube::Rendering_D3D11(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RenderTargetView* pRenderTargetView, ID3D11ShaderResourceView* pTextureView, TVertex8* pVertices)
{
	HRESULT hr = S_FALSE;

#ifdef _DEBUG
	InfoTexture2D InfoRTV = {};
	InfoTexture2D InfoSRV = {};
	hr = GetInfoFromRenderTargetView(pRenderTargetView, &InfoRTV);
	hr = GetInfoFromShaderResourceView(pTextureView, &InfoSRV);
#endif

	if (m_BackgroundColor == 0)
	{
		hr = DrawDeck();
		if (hr != S_OK) return S_FALSE;
	}
	else
	{
		D3DXCOLOR Color;
		if (m_BackgroundColor == 1) Color = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
		else if (m_BackgroundColor == 2) Color = D3DXCOLOR(0.1f, 0.2f, 0.6f, 1.0f);
		else if (m_BackgroundColor == 3) Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		else Color = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);

		FLOAT ColorRGBA[4] = {Color.r, Color.g, Color.b, Color.a};
		pDeviceContext->ClearRenderTargetView(pRenderTargetView, ColorRGBA);

		pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);
	}


	if (m_RotationDisk && m_HoldDisk)
	{
		double hold = 0;
		hr = GetInfo("hold", &hold);
		if (hr != S_OK) hold = 0.0f;

		if (hold == 0.0f) return S_OK;
	}

	hr = Update_VertexBufferDynamic_D3D11(pDeviceContext);
	if (hr != S_OK) return S_FALSE;

	hr = Update_ConstantBufferDynamic_D3D11(pDeviceContext);
	if (hr != S_OK) return S_FALSE;

	D3D11_VIEWPORT viewport = { 0.0f, 0.0f, (FLOAT) m_Width, (FLOAT) m_Height, D3D11_MIN_DEPTH, D3D11_MAX_DEPTH };
	pDeviceContext->RSSetViewports(1, &viewport);

	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	if (pInputLayout)
	{
		pDeviceContext->IASetInputLayout(pInputLayout);
	}

	if (pVertexShader)
	{
		pDeviceContext->VSSetShader(pVertexShader, nullptr, 0);
	}
	
	if (pPixelShader)
	{
		pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);
	}

	if (pConstantBuffer)
	{
		pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	}
	
	if (pRasterizerState)
	{
		pDeviceContext->RSSetState(pRasterizerState);
	}
	
	if (pTextureView)
	{
		pDeviceContext->PSSetShaderResources(0, 1, &pTextureView);
	}

	if (pVertexBuffer)
	{
		UINT m_VertexStride = sizeof(TLVERTEX);
		UINT m_VertexOffset = 0;
		pDeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &m_VertexStride, &m_VertexOffset);
	}

	pDeviceContext->Draw(m_VertexCount, 0);

	return S_OK;
}
// ---------------------------------------------------------------------- -
HRESULT CVideoCube::Create_VertexBufferDynamic_D3D11(ID3D11Device* pDevice)
{
	HRESULT hr = S_FALSE;

	if (!pDevice) return E_FAIL;

	// Set the number of vertices in the vertex array.
	m_VertexCount = 36; // = ARRAYSIZE(pNewVertices);
	
	// Fill in a buffer description.
	D3D11_BUFFER_DESC VertexBufferDesc;
	ZeroMemory(&VertexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	VertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;   // CPU_Access=Write_Only & GPU_Access=Read_Only
	VertexBufferDesc.ByteWidth = sizeof(TLVERTEX) * m_VertexCount;
	VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Use as vertex buffer
	VertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // Allow CPU to write in buffer
	VertexBufferDesc.MiscFlags = 0;
	VertexBufferDesc.StructureByteStride = sizeof(TLVERTEX);


	// Create the vertex buffer.
	hr = pDevice->CreateBuffer(&VertexBufferDesc, nullptr, &pVertexBuffer);
	if (hr != S_OK || !pVertexBuffer) return S_FALSE;

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::Create_ConstantBufferDynamic_D3D11(ID3D11Device* pDevice)
{
	HRESULT hr = S_FALSE;

	if (!pDevice) return E_FAIL;

	UINT SIZEOF_VS_CONSTANTBUFFER = sizeof(VS_CONSTANTBUFFER);
	UINT CB_BYTEWIDTH = SIZEOF_VS_CONSTANTBUFFER + 0xf & 0xfffffff0;


	D3D11_BUFFER_DESC ConstantBufferDesc = {};
	ConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;  // CPU_Access=Write_Only & GPU_Access=Read_Only
	ConstantBufferDesc.ByteWidth = CB_BYTEWIDTH;
	ConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;  // Allow CPU to write in buffer
	ConstantBufferDesc.MiscFlags = 0;


	// Create the constant buffer to send to the cbuffer in hlsl file
	hr = pDevice->CreateBuffer(&ConstantBufferDesc, nullptr, &pConstantBuffer);
	if (hr != S_OK || !pConstantBuffer) return S_FALSE;

	return hr;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::Create_VertexShader_D3D11(ID3D11Device* pDevice)
{
	HRESULT hr = S_FALSE;
	const WCHAR* pShaderHLSLFilepath = L"VertexShader.hlsl";
	const WCHAR* pShaderCSOFilepath = L"VertexShader.cso";
	const WCHAR* resourceType = RT_RCDATA;
	const WCHAR* resourceName = L"VERTEXSHADER8_CSO";

	SAFE_RELEASE(pVertexShader);

	hr = Create_VertexShaderFromResourceCSOFile_D3D11(pDevice, resourceType, resourceName);
	if (hr != S_OK || !pVertexShader) return S_FALSE;

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::Create_PixelShader_D3D11(ID3D11Device* pDevice)
{
	HRESULT hr = S_FALSE;
	const WCHAR* pShaderHLSLFilepath = L"PixelShader.hlsl";
	const WCHAR* pShaderCSOFilepath = L"PixelShader.cso";
	const WCHAR* resourceType = RT_RCDATA;
	const WCHAR* resourceName = L"PIXELSHADER8_CSO";

	SAFE_RELEASE(pPixelShader);

	hr = Create_PixelShaderFromResourceCSOFile_D3D11(pDevice, resourceType, resourceName);
	if (hr != S_OK || !pPixelShader) return S_FALSE;

	return S_OK;
}
//-------------------------------------------------------------------------------------------
HRESULT CVideoCube::Create_InputLayout_D3D11(ID3D11Device* pDevice)
{
	HRESULT hr = S_FALSE;

	if (!pDevice) return E_FAIL;

	const D3D11_INPUT_ELEMENT_DESC InputElmentDesc[3] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",	  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(InputElmentDesc);

	const WCHAR* resourceType = RT_RCDATA;
	const WCHAR* resourceName = L"VERTEXSHADER8_CSO";
	void* pShaderBytecode = nullptr;
	SIZE_T BytecodeLength = 0;

	hr = ReadResource(resourceType, resourceName, &BytecodeLength, &pShaderBytecode);
	if (hr != S_OK) return S_FALSE;

	hr = pDevice->CreateInputLayout(InputElmentDesc, numElements, pShaderBytecode, BytecodeLength, &pInputLayout);
	if (hr != S_OK || !pInputLayout) return S_FALSE;

	return hr;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::Create_RasterizerState_D3D11(ID3D11Device* pDevice)
{
	HRESULT hr;

	D3D11_RASTERIZER_DESC RasterizerDesc;
	ZeroMemory(&RasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	RasterizerDesc.FillMode = D3D11_FILL_SOLID;
	RasterizerDesc.CullMode = D3D11_CULL_FRONT; // cull front faces (D3D11_CULL_FRONT or D3D11_CULL_BACK or D3D11_CULL_NONE); 
	RasterizerDesc.FrontCounterClockwise = TRUE; // CCW = front
	RasterizerDesc.DepthClipEnable = FALSE;

	hr = pDevice->CreateRasterizerState(&RasterizerDesc, &pRasterizerState);
	if (hr != S_OK || !pRasterizerState) return S_FALSE;

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::Update_VertexBufferDynamic_D3D11(ID3D11DeviceContext* ctx)
{
	HRESULT hr = S_FALSE;

	if (!ctx) return S_FALSE;
	if (!pVertexBuffer) return S_FALSE;

	hr = Update_NewVertices_D3D11();

	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	ZeroMemory(&MappedSubResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	hr = ctx->Map(pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource);
	if (hr != S_OK) return S_FALSE;

	memcpy(MappedSubResource.pData, pNewVertices, m_VertexCount * sizeof(TLVERTEX));

	ctx->Unmap(pVertexBuffer, NULL);

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::Update_ConstantBufferDynamic_D3D11(ID3D11DeviceContext* ctx)
{
	HRESULT hr = S_FALSE;

	if (!ctx) return S_FALSE;
	if (!pConstantBuffer) return S_FALSE;

	hr = Update_ConstantBufferData_D3D11();

	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	ZeroMemory(&MappedSubResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	hr = ctx->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource);
	if (hr != S_OK) return S_FALSE;

	memcpy(MappedSubResource.pData, &m_ConstantBufferData, sizeof(VS_CONSTANTBUFFER));

	ctx->Unmap(pConstantBuffer, 0);

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::Update_NewVertices_D3D11()
{
	float frameWidth = (float) m_Width;
	float frameHeight = (float) m_Height;
	float frameDepth = (float) m_Depth;

	D3DXPOSITION P1 = { 0.0f, 0.0f, 0.0f }, // FTL : Front Top Left (0,0,0)
		P2 = { frameWidth, 0.0f, 0.0f }, // FTR: Front Top Right (w,0,0)
		P3 = { 0.0f, frameHeight, 0.0f }, // FBL: Front Bottom Left (0,h,0)
		P4 = { frameWidth, frameHeight, 0.0f }, // FBR: Front Bottom Right (w,h,0)
		P5 = { 0.0f, 0.0f, frameDepth }, // BTL: Back Top Left (0,0,d)
		P6 = { frameWidth, 0.0f, frameDepth }, // BTR: Back Top Right (w,0,d) - 
		P7 = { 0.0f, frameHeight, frameDepth }, // BBL: Back Bottom Left (0,h,d)
		P8 = { frameWidth, frameHeight, frameDepth }; // BBR: Back Bottom Right (w,h,d)

	D3DXCOLOR color_vertex = D3DXCOLOR(1.0f, 1.0f, 1.0f, m_Alpha); // white color with alpha
	D3DXTEXCOORD T1 = { 0.0f , 0.0f },
		T2 = { 1.0f , 0.0f },
		T3 = { 0.0f , 1.0f },
		T4 = { 1.0f , 1.0f };
	

	TLVERTEX Cube[36] = {
		// FRONT face (z = 0)
		{P1, color_vertex, T3}, {P3, color_vertex, T1}, {P4, color_vertex, T2}, // FTL + FBL + FBR
		{P1, color_vertex, T3}, {P4, color_vertex, T2}, {P2, color_vertex, T4}, // FTL + FBR + FTR

		// BACK face (z = frameDepth)
		{P6, color_vertex, T3}, {P8, color_vertex, T1}, {P7, color_vertex, T2}, // BTR + BBR + BBL
		{P6, color_vertex, T3}, {P7, color_vertex, T2}, {P5, color_vertex, T4}, // BTR + BBL + BTL

		// LEFT face (x = 0)
		{P5, color_vertex, T3}, {P7, color_vertex, T1}, {P3, color_vertex, T2}, // BTL + BBL + FBL
		{P5, color_vertex, T3}, {P3, color_vertex, T2}, {P1, color_vertex, T4}, // BTL + FBL + FTL

		// RIGHT face (x = frameWidth)
		{P2, color_vertex, T3}, {P4, color_vertex, T1}, {P8, color_vertex, T2}, // FTR + FBR + BBR
		{P2, color_vertex, T3}, {P8, color_vertex, T2}, {P6, color_vertex, T4}, // FTR + BBR + BTR

		// TOP face (y = 0)
		{P5, color_vertex, T3}, {P1, color_vertex, T1}, {P2, color_vertex, T2}, // BTL + FTL + FTR
		{P5, color_vertex, T3}, {P2, color_vertex, T2}, {P6, color_vertex, T4}, // BTL + FTR + BTR

		// BOTTOM face (y = frameHeight)
		{P3, color_vertex, T3}, {P7, color_vertex, T1}, {P8, color_vertex, T2}, // FBL + BBL + BBR
		{P3, color_vertex, T3}, {P8, color_vertex, T2}, {P4, color_vertex, T4}  // FBL + BBR + FBR
	};

	memcpy(pNewVertices, Cube, m_VertexCount * sizeof(TLVERTEX));
	

	
	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::Update_ConstantBufferData_D3D11()
{
	DirectX::XMMATRIX WorldMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX ViewMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixIdentity();
	

	ViewMatrix = SetViewMatrix_D3D11();
	ProjectionMatrix = SetProjectionMatrix_D3D11();
	WorldMatrix = SetWorldMatrix_D3D11();

	DirectX::XMMATRIX WVPMatrix = WorldMatrix * ViewMatrix * ProjectionMatrix;

	// Transposed because HLSL uses row-major by default whereas XMMATRIX is column-major.
	m_ConstantBufferData.m_WVPMatrix = XMMatrixTranspose(WVPMatrix);

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::Create_VertexShaderFromResourceCSOFile_D3D11(ID3D11Device* pDevice, const WCHAR* resourceType, const WCHAR* resourceName)
{
	HRESULT hr = S_FALSE;

	void* pShaderBytecode = nullptr;
	SIZE_T BytecodeLength = 0;

	hr = ReadResource(resourceType, resourceName, &BytecodeLength, &pShaderBytecode);
	if (hr != S_OK) return S_FALSE;

	hr = pDevice->CreateVertexShader(pShaderBytecode, BytecodeLength, nullptr, &pVertexShader);
	
	return hr;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::Create_PixelShaderFromResourceCSOFile_D3D11(ID3D11Device* pDevice, const WCHAR* resourceType, const WCHAR* resourceName)
{
	HRESULT hr = S_FALSE;
	if (!pDevice) return S_FALSE;

	void* pShaderBytecode = nullptr;
	SIZE_T BytecodeLength = 0;

	hr = ReadResource(resourceType, resourceName, &BytecodeLength, &pShaderBytecode);
	if (hr != S_OK) return S_FALSE;

	hr = pDevice->CreatePixelShader(pShaderBytecode, BytecodeLength, nullptr, &pPixelShader);

	return hr;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::ReadResource(const WCHAR* resourceType, const WCHAR* resourceName, SIZE_T* size, LPVOID* data)
{
	HRESULT hr = S_FALSE;

	HRSRC rc = FindResource(hInstance, resourceName, resourceType);
	if (!rc) return S_FALSE;

	HGLOBAL rcData = LoadResource(hInstance, rc);
	if (!rcData) return S_FALSE;

	*size = (SIZE_T)SizeofResource(hInstance, rc);
	if (*size == 0) return S_FALSE;

	*data = LockResource(rcData);
	if (*data == nullptr) return S_FALSE;

	return S_OK;
}
//-----------------------------------------------------------------------
DirectX::XMMATRIX CVideoCube::SetViewMatrix_D3D11()
{
	float cam_posX = (float) m_Width / 2.0f;
	float cam_posY = (float) m_Height / 2.0f;
	float cam_posZ = (float)(max(m_Width, m_Height) + 100.0f) * (-1.0f);  // Z is negative because we are looking at the origin from above

	float cam_tgtX = (float) m_Width / 2.0f;
	float cam_tgtY = (float) m_Height / 2.0f;
	float cam_tgtZ = 0.0f;

	//Camera information
	DirectX::XMVECTOR camPosition = DirectX::XMVectorSet(cam_posX, cam_posY, cam_posZ, 1.0f); // camera location
	DirectX::XMVECTOR camTarget = DirectX::XMVectorSet(cam_tgtX, cam_tgtY, cam_tgtZ, 1.0f); // what the camera looks at (here: looking at the origin)
	DirectX::XMVECTOR camUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // Y-axis up

	//Set the View matrix
	//This is using left - handed coordinates, which is the default in DirectX.If you're using a right-handed coordinate system, you would use XMMatrixLookAtRH instead.
	DirectX::XMMATRIX ViewMatrix = DirectX::XMMatrixLookAtLH(camPosition, camTarget, camUp);

	return ViewMatrix;
}
//-----------------------------------------------------------------------
DirectX::XMMATRIX CVideoCube::SetProjectionMatrix_D3D11()
{
	float FovDegreesAngleY = m_Zoom;
	float AspectRatio = float(m_Width) / float(m_Height);
	float NearZ = 0.1f;
	float FarZ = 1000.0f;

	float FovRadiansAngleY = DirectX::XMConvertToRadians(FovDegreesAngleY);

	//Set the Projection matrix
	DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(FovRadiansAngleY, AspectRatio, NearZ, FarZ);

	return ProjectionMatrix;
}
//-----------------------------------------------------------------------
DirectX::XMMATRIX CVideoCube::SetWorldMatrix_D3D11()
{
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixIdentity();
	float fRadians_Angle = 0;
	double x = 0;
	HRESULT hr = S_FALSE;

	if (m_RotationDisk)
	{
		hr = GetInfo("get_rotation", &x);
		if (hr != S_OK) x = 0.0f;

		if (m_RotationInverted)
		{
			m_Angle = (-1.0f) * float(x) * 360.0f;
		}
		else
		{
			m_Angle = float(x) * 360.0f;
		}
	}
	else if (m_Beats)
	{
		if (m_Speed == 0) x = 0;
		else
		{
			x = fmod(SongPosBeats, m_Speed) / m_Speed;
		}
			
		if (m_RotationInverted)
		{
			m_Angle = (-1.0f) * float(x) * 360.0f;
		}
		else
		{
			m_Angle = float(x) * 360.0f;
		}
	}
	else
	{
		if (m_RotationInverted)
		{
			m_Angle -= m_Speed;
			if (m_Angle < 0.0f)
				m_Angle = 360.0f - m_Angle;
		}
		else
		{
			m_Angle += m_Speed;
			if (m_Angle > 360.0f)
				m_Angle = m_Angle - 360.0f;
		}
	}

	fRadians_Angle = DirectX::XMConvertToRadians(m_Angle);

	switch (m_RotationAxe)
	{
		case ID_RADIO_1:
			RotationMatrix = DirectX::XMMatrixRotationX(fRadians_Angle);
			break;
		case ID_RADIO_2:
			RotationMatrix = DirectX::XMMatrixRotationY(fRadians_Angle);
			break;
		case ID_RADIO_3:
			RotationMatrix = DirectX::XMMatrixRotationZ(fRadians_Angle);
			break;
	}

	// Center point of the rotation
	D3DXPOSITION Co = { (float) m_Width / 2.0f , (float) m_Height / 2.0f , (float) m_Depth / 2.0f };

	DirectX::XMMATRIX TranslationMatrix1 = DirectX::XMMatrixTranslation(-Co.x, -Co.y, -Co.z);
	DirectX::XMMATRIX TranslationMatrix2 = DirectX::XMMatrixTranslation(Co.x, Co.y, Co.z);

	DirectX::XMMATRIX WorldMatrix = TranslationMatrix1 * RotationMatrix * TranslationMatrix2;

	return WorldMatrix;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::GetInfoFromShaderResourceView(ID3D11ShaderResourceView* pShaderResourceView, InfoTexture2D* info)
{
	HRESULT hr = S_FALSE;

	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));

	pShaderResourceView->GetDesc(&viewDesc);

	DXGI_FORMAT ViewFormat = viewDesc.Format;
	D3D11_SRV_DIMENSION ViewDimension = viewDesc.ViewDimension;

	ID3D11Resource* pResource = nullptr;
	pShaderResourceView->GetResource(&pResource);
	if (!pResource) return S_FALSE;

	if (ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D)
	{
		ID3D11Texture2D* pTexture = nullptr;
		hr = pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);
		if (hr != S_OK || !pTexture) return S_FALSE;

		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

		pTexture->GetDesc(&textureDesc);

		info->Format = textureDesc.Format;
		info->Width = textureDesc.Width;
		info->Height = textureDesc.Height;

		SAFE_RELEASE(pTexture);
	}

	SAFE_RELEASE(pResource);

	return S_OK;
}
//-----------------------------------------------------------------------
HRESULT CVideoCube::GetInfoFromRenderTargetView(ID3D11RenderTargetView* pRenderTargetView, InfoTexture2D* info)
{
	HRESULT hr = S_FALSE;

	D3D11_RENDER_TARGET_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

	pRenderTargetView->GetDesc(&viewDesc);

	DXGI_FORMAT ViewFormat = viewDesc.Format;
	D3D11_RTV_DIMENSION ViewDimension = viewDesc.ViewDimension;

	ID3D11Resource* pResource = nullptr;
	pRenderTargetView->GetResource(&pResource);
	if (!pResource) return S_FALSE;

	if (ViewDimension == D3D11_RTV_DIMENSION_TEXTURE2D)
	{
		ID3D11Texture2D* pTexture = nullptr;
		hr = pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTexture);
		if (hr != S_OK || !pTexture) return S_FALSE;

		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));

		pTexture->GetDesc(&textureDesc);

		info->Format = textureDesc.Format;
		info->Width = textureDesc.Width;
		info->Height = textureDesc.Height;

		SAFE_RELEASE(pTexture);
	}

	SAFE_RELEASE(pResource);

	return S_OK;
}
