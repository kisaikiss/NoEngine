struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};
Texture2D<float4> gTexture : register(t0);
Texture2D<float4> gMask : register(t1);
SamplerState gSampler : register(s0);

cbuffer gMaterial : register(b0)
{
    float4 color;
};

cbuffer gMaskParams : register(b1)
{
    float fill; // 0..1 （UV.x に対する切り詰め量）
    int useMask; // 0 = UVクリップを使う, 1 = テクスチャマスクを使う
    float angle; // 描画しない範囲の角度
    float padding; // パディング
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    if (angle > 0.f)//値が入っている場合のみ処理する
    {
	    //--------------------------------------------
	    //描画しない条件に当てはまるか
	    //--------------------------------------------
	
	    //1.UVの値を考えやすくする
	    //画像の中心が(0.5,0.5)だと難しいので
	    //中心(0,0)にするために0.5を引く
        float uvX = (input.texcoord.x - 0.5f);
        float uvY = (input.texcoord.y - 0.5f) * -1.f; //上に行くほど数字が大きくなるようにするために-1をかける
 
        //2.距離からこのピクセルの角度をだす
        //基準となる軸からの距離を求める → atan2で角度を求める
        //-180になる位置が基準になる
        float blue = -1 * uvY;
        float red = -1 * uvX;
        float thisAngle = atan2(red, blue);
        	
        //3.値を修正
        //すべて正になるように180°加える
        thisAngle += 3.141592f;
        
        //4.定数バッファの角度と比較
        //定数バッファより小さかったら描画しない
        if (thisAngle < angle)
        {
            discard; //ここで終了　以下は読まれない
        }
    }
    
    float4 tex = gTexture.Sample(gSampler, input.texcoord);

    float alpha = tex.a;

    float mask = 0;
    if (useMask == 1)
    {
        mask = gMask.Sample(gSampler, input.texcoord).r;
        float maskAllow = 1.0 - mask;

        float fillAllow = step(input.texcoord.x, fill);

        alpha *= maskAllow * fillAllow;
    }

    if (alpha < 0.5)
    {
        discard;
    }
    
    output.color = float4(tex.rgb * color.rgb, alpha * color.a);
    return output;
}
