
sampler texSampler : register(s0);

static const float Pixels[13] =
{
   -6,
   -5,
   -4,
   -3,
   -2,
   -1,
    0,
    1,
    2,
    3,
    4,
    5,
    6,
};

static const float BlurWeights[13] =
{
   0.002216,
   0.008764,
   0.026995,
   0.064759,
   0.120985,
   0.176033,
   0.199471,
   0.176033,
   0.120985,
   0.064759,
   0.026995,
   0.008764,
   0.002216,
};


float4 main(float2 uv : TEXCOORD0) : COLOR
{
    // Pixel width
    float pixelWidth = 1/256.0f;

    float4 color = {0, 0, 0, 0};

    float2 blur;
#ifdef HOR
    blur.y = uv.y;
#else
	blur.x = uv.x;
#endif

    for (int i = 0; i < 13; i++) 
    {
#ifdef HOR
        blur.x = uv.x + Pixels[i] * pixelWidth;
#else
		blur.y = uv.y + Pixels[i] * pixelWidth;
#endif
        color.a += tex2D(texSampler, blur.xy).a * BlurWeights[i];
    }  

    return color;
}
