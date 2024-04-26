#include "Renderer.h"

#include <d3d11.h>
#include "ComPtr.h"

class PolyLines : public Renderer
{
public:
	PolyLines(HWND h) : Renderer(h) {}

	virtual void Update(double elapsed);
	virtual void Render();

	struct Vertex {
		float pos_ss[2];
		static const D3D11_INPUT_ELEMENT_DESC desc[];
	};

private:
	virtual void Initialize();

	ComPtr<ID3D11VertexShader> polyLineVS;
	ComPtr<ID3D11PixelShader> polyLinePS;
	ComPtr<ID3D11InputLayout> polyLineInputLayout;
	ComPtr<ID3D11Buffer> viewportCBuffer;
	ComPtr<ID3D11Buffer> lineVertexConstants;
	ComPtr<ID3D11Buffer> lineParamsBuffer;
	ComPtr<ID3D11BlendState> pBlendState;

	double elapsedSum{ 0.f };
	unsigned bgIndex{ 0 };
};
