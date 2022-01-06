Shader "SolarLand/Billboard"
{
	Properties
	{
		[Enum(Plane, 0, NormalModel, 1)] _ObjectType("ObjectType", Int) = 0
		_MainTex("Base texture", 2D) = "white" {}
		_YRotateFactor("Y Rotate Factor", Range(0, 1)) = 1
		_ScaleX("_ScaleX", Float) = 1
		_ScaleY("_ScaleY", Float) = 1
	}
		
	SubShader
	{
		Tags { "Queue" = "Transparent" "IgnoreProjector" = "True" "RenderType" = "Transparent" "DisableBatching" = "True"}

		Pass
		{
			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag

			#include "UnityCG.cginc"

			int _ObjectType;
			sampler2D _MainTex;
			float _YRotateFactor;
			float _ScaleX;
			float _ScaleY;

			struct vertexInput {
				float4 vertex : POSITION;
				float4 tex : TEXCOORD0;
			};

			struct vertexOutput {
				float4 pos : SV_POSITION;
				float4 tex : TEXCOORD0;
			};

			void CalcLocalCoordinate(float3 dir, out float3 right, out float3 up)
			{
				up = abs(dir.y) > 0.999f ? float3(0, 0, 1) : float3(0, 1, 0);
				right = normalize(cross(float3(0, 1, 0),dir));
				up = cross(dir, right);
			}

			// vertex Shader
			vertexOutput vert(vertexInput input)
			{
				vertexOutput output;

				float3	centerLocal = float3(0, 0, 0);
				float3	centerOffs = input.vertex.xyz;

				float3	cameraLocal = mul(unity_WorldToObject, float4(_WorldSpaceCameraPos, 1));
				// vector from obj look at camera
				float3	localDir = normalize(cameraLocal - centerLocal);
				localDir.y = localDir.y * _YRotateFactor;

				float3	rightLocal = normalize(cross(float3(0, 1, 0), localDir));
				float3	upLocal = cross(localDir, rightLocal);

				/*CalcLocalCoordinate(normalize(localDir), rightLocal, upLocal);*/

				float3	finalLocalPos;

				// Plane
				if (_ObjectType == 0)
				{
					finalLocalPos = centerLocal - (rightLocal * centerOffs.z * _ScaleX + upLocal * centerOffs.x * _ScaleY);
				}
				// Normal 3D Model
				else if (_ObjectType == 1)
				{
					finalLocalPos = centerLocal + (rightLocal * centerOffs.x * _ScaleX + upLocal * centerOffs.y * _ScaleY);
				}

				output.pos = UnityObjectToClipPos(float4(finalLocalPos, 1));
				output.tex = input.tex;

				return output;
			}

			// fragment Shander
			fixed4 frag(vertexInput input) : COLOR
			{
				return tex2D(_MainTex, float2(input.tex.xy));
			}

			ENDCG
		}
	}
}