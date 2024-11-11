#version 460 core
#define PI 3.1415926535897932384626433832795
#extension GL_ARB_bindless_texture : require

in vec2 uv;


layout(std140, binding = 0) uniform CameraMatrices
{
	mat4 view;
	mat4 proj;
	mat4 invView;
};

//Light properties
layout(std140, binding = 1) uniform DirLight
{
	vec3 dirDir;
	vec4 dirCol; //w is the intensity (0-5)
};
struct PointLight
{
	vec4 pos; //w is the radius
	vec4 col;//a is intensity
};
readonly layout(std430, binding = 0) buffer PointLights
{
	uint numPLights;
	PointLight pLights[];
};
struct SpotLight
{
	vec4 pos; //w intensity
	vec4 aimD;//w cos inner angle
	vec4 col; //w cos outer angle
	float radius;
	int shadowIndex;
};
readonly layout(std430, binding = 1) buffer SpotLights
{
	uint numSLights;
	SpotLight sLights[];
};

struct Shadow
{
	mat4 viewProjMatrix;
	sampler2D shadowMap;
	float bias;
};
readonly layout(std430, binding = 4) buffer SpotLightShadows
{
	Shadow shadows[];
};


//Gbuffer
layout(binding = 0)uniform sampler2D diffuseTex;
layout(binding = 1)uniform sampler2D metalRoughTex;
layout(binding = 2)uniform sampler2D normalTex;
layout(binding = 3)uniform sampler2D depthTex;
layout(binding = 4)uniform sampler2D emissiveTex;
//Ambient
layout(binding = 5)uniform samplerCube prefilteredIBL;
layout(binding = 6)uniform samplerCube diffuseIBL;
layout(binding = 7)uniform sampler2D environmentBRDF;
uniform uint numLevels;

layout(binding = 10) uniform usamplerBuffer pointLightList;
layout(binding = 11) uniform usamplerBuffer spotLightList;
layout(location = 2) uniform uint lightListSize;
layout(location = 3) uniform uvec2 numTiles;
layout(location = 4) uniform uvec2 tileSize;

layout(binding = 9) uniform sampler2D ambientOcclusion;
uniform bool activeAO;

//layout (location = 3) uniform sampler2D theSampler;
//layout (location = 9) uniform sampler2D normalMapSampler;
//layout (location = 5)uniform vec3 lightCol;
//layout (location = 6)uniform vec3 ambientCol;
//layout (location = 8)uniform float kD;
//layout (location = 10)uniform float brightness;

out vec4 outCol;

vec3 cDif;
//vec3 cSpec;
float kD;
float rough;
vec3 N;
vec3 V;
float depth;
vec3 pos;
vec3 emissiveCol;

float GetLinearZ(float inputDepth)
{
	return -proj[3][2] / (proj[2][2] + (inputDepth * 2.0 - 1.0));
}

vec3 GetWorldPos(float depth, vec2 uvs)
{
	float viewZ = GetLinearZ(depth);
	float viewX = (-viewZ * (uvs.x * 2.0 - 1.0)) / proj[0][0];
	float viewY = (-viewZ * (uvs.y * 2.0 - 1.0)) / proj[1][1];
	return (invView * vec4(vec3(viewX, viewY, viewZ), 1.0)).xyz;
}

vec3 GetLightCol(vec3 lDir, vec3 lCol, float lInt, float lAtt)
{
	float brightness = (1.0f - rough) * 500;
	vec3 L = -normalize(lDir);
	vec3 R = normalize(reflect(L, N));
	vec3 Li = lInt * lAtt * lCol.rgb;  //Incoming radiance
	float diffuse = max(dot(N, L), 0.0);
	float specular = pow(max(dot(R, V), 0.0), brightness);
	return kD * diffuse * cDif * Li + specular * Li * (1 - kD);
}

void main()
{
	N = normalize(texture(normalTex, uv).rgb * 2.0 - 1.0);
	depth = texture(depthTex, uv).r;
	pos = GetWorldPos(depth, uv);
	//pos = texture(positionTex, uv).rgb;
	emissiveCol = texture(emissiveTex, uv).rgb;
	vec3 cameraPos = vec3(invView[3][0], invView[3][1], invView[3][2]);
	V = normalize(cameraPos - pos);

	vec3 baseColor = texture(diffuseTex, uv).rgb;
	vec2 specColorTex = texture(metalRoughTex, uv).rg;
	float metal = specColorTex.y;
	rough = max(specColorTex.x * specColorTex.x, 0.001f);

	cDif = baseColor * (1 - metal);
	//cSpec = mix(vec3(0.04), baseColor, metal);
	kD = (1 - metal);

	vec3 col = vec3(0);
	//Directional light
	col += GetLightCol(dirDir.xyz, dirCol.xyz, dirCol.w, 1);

	const uvec2 currTile = uvec2(gl_FragCoord.xy) / tileSize;
	const uint tileIdx = currTile.y * numTiles.x + currTile.x;
	//Point lights
	uint idx = texelFetch(pointLightList, int(tileIdx * lightListSize)).x;
	for (uint i = 0; i < lightListSize && idx != 255; idx = texelFetch(pointLightList, int(tileIdx * lightListSize + i)).x)
	{
		PointLight pLight = pLights[idx];
		vec3 mVector = pos - pLight.pos.xyz;
		float dist = length(mVector);
		vec3 pDir = normalize(mVector);
		float att = pow(max(1 - pow(dist / pLight.pos.w, 4), 0.0), 2) / (dist * dist + 1);
		col += GetLightCol(pDir, pLight.col.rgb, pLight.col.w, att);
		++i;
	}

	//Spot lights
	idx = texelFetch(spotLightList, int(tileIdx * lightListSize)).x;
	for (uint i = 0; i < lightListSize && idx != 255; idx = texelFetch(spotLightList, int(tileIdx * lightListSize + i)).x)
	{
		SpotLight sLight = sLights[idx];
		//Shadows
		bool inShadow = false;
		if (sLight.shadowIndex >= 0)
		{
			Shadow shadow = shadows[sLight.shadowIndex];
			vec4 lightClipSpace = shadow.viewProjMatrix * vec4(pos, 1);
			vec3 lightNDC = lightClipSpace.xyz / lightClipSpace.w;
			lightNDC.xyz = lightNDC.xyz * 0.5 + 0.5;
			float shadowDepth = texture(shadow.shadowMap, lightNDC.xy).r + shadow.bias;

			if (!(lightNDC.x >= 0.0f && lightNDC.x <= 1.0f &&
				lightNDC.y >= 0.0f && lightNDC.y <= 1.0f &&
				lightNDC.z < shadowDepth))
			{
				inShadow = true;
			}
		}
		if (!inShadow)
		{
			vec3 mVector = pos - sLight.pos.xyz;
			vec3 sDir = normalize(mVector);
			vec3 aimDir = normalize(sLight.aimD.xyz);
			float dist = dot(mVector, aimDir);
			//TODO: Check that the radius of spot light is correct
			float r = sLight.radius;
			float att = pow(max(1 - pow(dist / r, 4), 0), 2) / (dist * dist + 1);
			float c = dot(sDir, aimDir);
			float cInner = sLight.aimD.w;
			float cOuter = sLight.col.w;
			//float cAtt = 1;
			//if(cInner > c && c > cOuter)
				//cAtt = (c - cOuter) / (cInner - cOuter);
			float cAtt = clamp((c - cOuter) / (cInner - cOuter), 0.0, 1.0);
			att *= cAtt;
			col += GetLightCol(sDir, sLight.col.rgb, sLight.pos.w, att);
		}
		++i;
	}

	vec3 occlusionFactor = vec3(1.0);
	if (activeAO)
	{
		occlusionFactor = vec3(texture(ambientOcclusion, uv).r);
	}

	//ambient
	float dotNV = clamp(dot(N, V), 0.001, 1.0);
	vec3 irradiance = texture(diffuseIBL, N).rgb;
	col += cDif * irradiance * cDif * occlusionFactor;

	//emissive
	col += emissiveCol;

	//Output
	outCol = vec4(col, 1.0f);
}