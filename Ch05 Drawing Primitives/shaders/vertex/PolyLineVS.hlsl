cbuffer viewport : register(b0)
{
	float2 vpTopLeft;
	float2 vpDimensions;
	float2 vpDepth;
}

cbuffer points : register(b1)
{
	float2 begin;
	float2 end;
}

float4 NDCFromVec2SS(float2 v)
{
	return float4(((2 * (v + .501)) / vpDimensions) - 1, 0, 1);
	//return float4(((2 * v) / vpDimensions) - 1, 0, 1);
}

static const float2 QuadVerts[4] =
{
	float2(1,  0),
	float2(1,  1),
	float2(-1, 0),
	float2(-1, 1)
};

struct VSOut
{
	float4 position : SV_Position;
	float normal : NORMAL;
};

VSOut main(uint vertexId : SV_VertexID)
{
	float2 v = end - begin;
	float2 n = 3 * normalize(v.yx * float2(-1, 1));
	float2 ssPos = n * QuadVerts[vertexId].x + begin;
	ssPos += v * QuadVerts[vertexId].y;
	
	VSOut vsOut = {
		NDCFromVec2SS(ssPos),
		QuadVerts[vertexId].x
	};
	
	//return NDCFromVec2SS(vsIn.pos);
	return vsOut;
}
