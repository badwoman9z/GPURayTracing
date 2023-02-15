#version 430 core

layout(binding = 0,rgba16f) uniform image2D LastFrame;
uniform int nTriangles;
uniform sampler2D hdrmap;
uniform vec3 eye;
uniform mat4 cameraRotate;
in vec3 pix;

out vec4 fragColor;
// ----------------------------------------------------------------------------- //
#define PI              3.1415926
#define INF             114514.0
#define SIZE_TRIANGLE   12
#define SIZE_BVHNODE    4

struct Material {
    vec3 emissive;          // ��Ϊ��Դʱ�ķ�����ɫ
    vec3 baseColor;
    float subsurface;
    float metallic;
    float specular;
    float specularTint;
    float roughness;
    float anisotropic;
    float sheen;
    float sheenTint;
    float clearcoat;
    float clearcoatGloss;
    float IOR;
    float transmission;
};
struct Triangle {
    vec4 p1, p2, p3;    // ��������
    vec4 n1, n2, n3;    // ���㷨��
    vec4 emissive;      // �Է������
    vec4 baseColor;     // ��ɫ
    vec4 param1;        // (subsurface, metallic, specular)
    vec4 param2;        // (specularTint, roughness, anisotropic)
    vec4 param3;        // (sheen, sheenTint, clearcoat)



};
// BVH ���ڵ�
struct BVHNode {
    int left;           // ������
    int right;          // ������
    int n;              // ������������Ŀ
    int index;          // ����������
    vec4 AA, BB;        // ��ײ��
};
layout(std430,binding=3) buffer ssbo{

    Triangle ts[];

};

layout(std430,binding=2) buffer bvhssbo{
    BVHNode bvh[];
};


// ����
struct Ray {
    vec3 startPoint;
    vec3 direction;
};

// �����󽻽��
struct HitResult {
    bool isHit;             // �Ƿ�����
    bool isInside;          // �Ƿ���ڲ�����
    float dis;         // �뽻��ľ���
    vec3 hitPoint;          // �������е�
    vec3 normal;            // ���е㷨��
    vec3 viewDir;           // ���иõ�Ĺ��ߵķ���
    Material material;      // ���е�ı������
};

Material getMaterial(int i){

    Material m;
    
    m.emissive = ts[i].emissive.rgb;
    m.baseColor = ts[i].baseColor.rgb;
    m.subsurface = ts[i].param1.x;
    m.metallic = ts[i].param1.y;
    m.specular = ts[i].param1.z;
    m.specularTint =ts[i].param1.w;
    m.roughness = ts[i].param2.x;
    m.anisotropic = ts[i].param2.y;
    m.sheen = ts[i].param2.z;
    m.sheenTint = ts[i].param2.w;
    m.clearcoat = ts[i].param3.x;
    m.clearcoatGloss = ts[i].param3.y;
    m.IOR = ts[i].param3.z;
    m.transmission = ts[i].param3.w;

    return m;

}
//BVHNode getBVHNode(int i){
//    BVHNode node ;
//        // ��������
//    int offset = i * SIZE_BVHNODE;
//    ivec3 childs = ivec3(texelFetch(nodes, offset + 0).xyz);
//    ivec3 leafInfo = ivec3(texelFetch(nodes, offset + 1).xyz);
//    node.left = int(childs.x);
//    node.right = int(childs.y);
//    node.n = int(leafInfo.x);
//    node.index = int(leafInfo.y);
//
//    // ��Χ��
//    node.AA = texelFetch(nodes, offset + 2).xyz;
//    node.BB = texelFetch(nodes, offset + 3).xyz;
//
//    return node;
//
//}
HitResult hitTriangle(Triangle triangle,Ray ray){

    HitResult res;
    res.dis = INF;
    res.isHit = false;
    res.isInside = false;
    vec3 p1 = triangle.p1.xyz;
    vec3 p2 = triangle.p2.xyz;
    vec3 p3 = triangle.p3.xyz;

    vec3 S = ray.startPoint;
    vec3 d = ray.direction;
    vec3 N = normalize(cross(p2-p1,p3-p1));

    if(dot(d,N)>0.0f){
        N = -N;
        res.isInside = true;
        
     }
    if (abs(dot(N, d)) < 0.00001f) return res;
        // ����
    float t = (dot(N, p1) - dot(S, N)) / dot(d, N);
    if (t < 0.0005f) return res;    // ����������ڹ��߱���

    // �������
    vec3 P = S + d * t;

    // �жϽ����Ƿ�����������
    vec3 c1 = cross(p2 - p1, P - p1);
    vec3 c2 = cross(p3 - p2, P - p2);
    vec3 c3 = cross(p1 - p3, P - p3);
    bool r1 = (dot(c1, N) > 0 && dot(c2, N) > 0 && dot(c3, N) > 0);
    bool r2 = (dot(c1, N) < 0 && dot(c2, N) < 0 && dot(c3, N) < 0);

    // ���У���װ���ؽ��
    if (r1 || r2) {
        res.isHit = true;
        res.hitPoint = P;
        res.dis = t;
        res.normal = N;
        res.viewDir = d;
        // ���ݽ���λ�ò�ֵ���㷨��
        float alpha = (-(P.x-p2.x)*(p3.y-p2.y) + (P.y-p2.y)*(p3.x-p2.x)) / (-(p1.x-p2.x-0.00005)*(p3.y-p2.y+0.00005) + (p1.y-p2.y+0.00005)*(p3.x-p2.x+0.00005));
        float beta  = (-(P.x-p3.x)*(p1.y-p3.y) + (P.y-p3.y)*(p1.x-p3.x)) / (-(p2.x-p3.x-0.00005)*(p1.y-p3.y+0.00005) + (p2.y-p3.y+0.00005)*(p1.x-p3.x+0.00005));
        float gama  = 1.0 - alpha - beta;
        vec3 Nsmooth = alpha * triangle.n1.xyz + beta * triangle.n2.xyz + gama * triangle.n3.xyz;
        Nsmooth = normalize(Nsmooth);
        res.normal = (res.isInside) ? (-Nsmooth) : (Nsmooth);
    }
    return res;

}
HitResult hitArray(int l,int r,Ray ray){
    HitResult res;
    res.isHit = false;
    res.dis = INF;

    for(int i=l;i<=r;i++){
        //Triangle t = getTrianle(i);
        Triangle t = ts[i];
        HitResult r = hitTriangle(t,ray);
        if(r.isHit&&r.dis<res.dis){
            res = r;
            res.material = getMaterial(i);
           // res.material =t.material; 
        }
    }
    return res;

}
float hitAABB(Ray r, vec3 AA, vec3 BB) {
    vec3 invdir = 1.0 / r.direction;

    vec3 f = (BB - r.startPoint) * invdir;
    vec3 n = (AA - r.startPoint) * invdir;

    vec3 tmax = max(f, n);
    vec3 tmin = min(f, n);

    float t1 = min(tmax.x, min(tmax.y, tmax.z));
    float t0 = max(tmin.x, max(tmin.y, tmin.z));

    return (t1 >= t0) ? ((t0 > 0.0) ? (t0) : (t1)) : (-1);
}
HitResult hitBVH(Ray ray){
    HitResult res;
    res.isHit = false;
    res.dis = INF;

    int stack[256];
    int sp =0;

    stack[sp++] = 1;
    while(sp>0){
    
        int top = stack[--sp];
        BVHNode node = bvh[top];
        if(node.n>0){
            int l = node.index;
            int r = node.index+node.n-1;
            HitResult temp ;
            temp = hitArray(l,r,ray);
            if(temp.isHit&&temp.dis<res.dis){
                res = temp;
                
            }
            continue;
     
        }
        float d1 = INF;
        float d2 = INF;
        if(node.left>0){
            d1 = hitAABB(ray,bvh[node.left].AA.xyz,bvh[node.left].BB.xyz);
        
        }
        if(node.right>0){
            d2 = hitAABB(ray,bvh[node.right].AA.xyz,bvh[node.right].BB.xyz);
        }
        if(d1>0&&d2>0){
            if(d1>=d2){
            
                stack[sp++] = node.left;
                stack[sp++] = node.right;
            }else{
                stack[sp++] = node.right;
                stack[sp++] = node.left;
                
            }
             
        }else if(d1>0){
            stack[sp++] = node.left;
            
        }else if(d2>0){
            stack[sp++] = node.right;
            
        }
    
    }
    return res;

}



uniform int frameCounter;
uniform int width;
uniform int height;
uint seed = uint(
    uint((pix.x * 0.5 + 0.5) * width)  * uint(1973) + 
    uint((pix.y * 0.5 + 0.5) * height) * uint(9277) + 
    uint(frameCounter) * uint(26699)) | uint(1);

uint wang_hash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}
 
float rand() {
    return float(wang_hash(seed)) / 4294967296.0;
}

vec3 SampleHemisphere() {
    float z = rand();
    float r = max(0, sqrt(1.0 - z*z));
    float phi = 2.0 * PI * rand();
    return vec3(r * cos(phi), r * sin(phi), z);
}
// ������ v ͶӰ�� N �ķ������
vec3 toNormalHemisphere(vec3 v, vec3 N) {
    vec3 helper = vec3(1, 0, 0);
    if(abs(N.x)>0.999) helper = vec3(0, 0, 1);
    vec3 tangent = normalize(cross(N, helper));
    vec3 bitangent = normalize(cross(N, tangent));
    return v.x * tangent + v.y * bitangent + v.z * N;
}















void getTangent(vec3 N, inout vec3 tangent, inout vec3 bitangent) {
    /*
    vec3 helper = vec3(0, 0, 1);
    if(abs(N.z)>0.999) helper = vec3(0, -1, 0);
    tangent = normalize(cross(N, helper));
    bitangent = normalize(cross(N, tangent));
    */
    vec3 helper = vec3(1, 0, 0);
    if(abs(N.x)>0.999) helper = vec3(0, 0, 1);
    bitangent = normalize(cross(N, helper));
    tangent = normalize(cross(N, bitangent));
}

// ----------------------------------------------------------------------------- //

float sqr(float x) { 
    return x*x; 
}

float SchlickFresnel(float u) {
    float m = clamp(1-u, 0, 1);
    float m2 = m*m;
    return m2*m2*m; // pow(m,5)
}

float GTR1(float NdotH, float a) {
    if (a >= 1) return 1/PI;
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return (a2-1) / (PI*log(a2)*t);
}

float GTR2(float NdotH, float a) {
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return a2 / (PI * t*t);
}

float GTR2_aniso(float NdotH, float HdotX, float HdotY, float ax, float ay) {
    return 1 / (PI * ax*ay * sqr( sqr(HdotX/ax) + sqr(HdotY/ay) + NdotH*NdotH ));
}

float smithG_GGX(float NdotV, float alphaG) {
    float a = alphaG*alphaG;
    float b = NdotV*NdotV;
    return 1 / (NdotV + sqrt(a + b - a*b));
}

float smithG_GGX_aniso(float NdotV, float VdotX, float VdotY, float ax, float ay) {
    return 1 / (NdotV + sqrt( sqr(VdotX*ax) + sqr(VdotY*ay) + sqr(NdotV) ));
}








vec3 BRDF_Evaluate(vec3 V, vec3 N, vec3 L, vec3 X, vec3 Y, in Material material) {
    float NdotL = dot(N, L);
    float NdotV = dot(N, V);
    if(NdotL < 0 || NdotV < 0) return vec3(0);

    vec3 H = normalize(L + V);
    float NdotH = dot(N, H);
    float LdotH = dot(L, H);

    // ������ɫ
    vec3 Cdlin = material.baseColor;
    float Cdlum = 0.3 * Cdlin.r + 0.6 * Cdlin.g  + 0.1 * Cdlin.b;
    vec3 Ctint = (Cdlum > 0) ? (Cdlin/Cdlum) : (vec3(1));   
    vec3 Cspec = material.specular * mix(vec3(1), Ctint, material.specularTint);
    vec3 Cspec0 = mix(0.08*Cspec, Cdlin, material.metallic); // 0�� ���淴����ɫ
    vec3 Csheen = mix(vec3(1), Ctint, material.sheenTint);   // ֯����ɫ

    // ������
    float Fd90 = 0.5 + 2.0 * LdotH * LdotH * material.roughness;
    float FL = SchlickFresnel(NdotL);
    float FV = SchlickFresnel(NdotV);
    float Fd = mix(1.0, Fd90, FL) * mix(1.0, Fd90, FV);

    // �α���ɢ��
    float Fss90 = LdotH * LdotH * material.roughness;
    float Fss = mix(1.0, Fss90, FL) * mix(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1.0 / (NdotL + NdotV) - 0.5) + 0.5);

    /* 
    // ���淴�� -- ����ͬ��
    float alpha = material.roughness * material.roughness;
    float Ds = GTR2(NdotH, alpha);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs = smithG_GGX(NdotL, material.roughness);
    Gs *= smithG_GGX(NdotV, material.roughness);
    */
    // ���淴�� -- ��������
    float aspect = sqrt(1.0 - material.anisotropic * 0.9);
    float ax = max(0.001, sqr(material.roughness)/aspect);
    float ay = max(0.001, sqr(material.roughness)*aspect);
    float Ds = GTR2_aniso(NdotH, dot(H, X), dot(H, Y), ax, ay);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0, vec3(1), FH);
    float Gs;
    Gs  = smithG_GGX_aniso(NdotL, dot(L, X), dot(L, Y), ax, ay);
    Gs *= smithG_GGX_aniso(NdotV, dot(V, X), dot(V, Y), ax, ay);

    // ����
    float Dr = GTR1(NdotH, mix(0.1, 0.001, material.clearcoatGloss));
    float Fr = mix(0.04, 1.0, FH);
    float Gr = smithG_GGX(NdotL, 0.25) * smithG_GGX(NdotV, 0.25);

    // sheen
    vec3 Fsheen = FH * material.sheen * Csheen;
    
    vec3 diffuse = (1.0/PI) * mix(Fd, ss, material.subsurface) * Cdlin + Fsheen;
    vec3 specular = Gs * Fs * Ds;
    vec3 clearcoat = vec3(0.25 * Gr * Fr * Dr * material.clearcoat);

    return diffuse * (1.0 - material.metallic) + specular + clearcoat;
}
// ����ά���� v תΪ HDR map ���������� uv
vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv /= vec2(2.0 * PI, PI);
    uv += 0.5;
    uv.y = 1.0 - uv.y;
    return uv;
}

// ��ȡ HDR ������ɫ
vec3 sampleHdr(vec3 v) {
    vec2 uv = SampleSphericalMap(normalize(v));
    vec3 color = texture2D(hdrmap, uv).rgb;
    //color = min(color, vec3(10));
    return color;
}

vec3 pathTracing(HitResult hit,int maxBounce){
    vec3 Lo = vec3(0);
    vec3 history = vec3(1);
    for(int bounce = 0;bounce<maxBounce;bounce++){
    
        vec3 V = -hit.viewDir;
        vec3 N = hit.normal;
        vec3 L = toNormalHemisphere(SampleHemisphere(), hit.normal);    // ������䷽�� wi
        float pdf = 1.0 / (2.0 * PI);                                   // ������Ȳ��������ܶ�
        float cosine_o = max(0, dot(V, N));                             // �����ͷ��߼н�����
        float cosine_i = max(0, dot(L, N));                             // �����ͷ��߼н�����
        vec3 tangent, bitangent;
        getTangent(N, tangent, bitangent);
        vec3 f_r = BRDF_Evaluate(V, N, L, tangent, bitangent, hit.material);
        //vec3 f_r = hit.material.baseColor/PI;
       Ray randomRay;
       randomRay.startPoint = hit.hitPoint;
       randomRay.direction = L;
       HitResult newhit = hitBVH(randomRay);
       if(!newhit.isHit){
        vec3 skyColor = sampleHdr(randomRay.direction);
        Lo += history * skyColor * f_r * cosine_i / pdf;
        break;
       }

       vec3 Le = newhit.material.emissive;

       Lo+=history*Le*f_r*cosine_i/pdf;
       hit = newhit;
       history*=f_r*cosine_i/pdf;
    
    
    
    }
    return Lo;



}





vec3 toneMapping(in vec3 c, float limit) {
    float luminance = 0.3*c.x + 0.6*c.y + 0.1*c.z;
    return c * 1.0 / (1.0 + luminance / limit);
}




void main(){
 Ray ray ;
 ray.startPoint = eye;
 vec2 AA = vec2((rand()-0.5)/float(width), (rand()-0.5)/float(height));
 vec4 dir = cameraRotate * vec4(pix.xy+AA, -1.5, 0.0);
 ray.direction = normalize(dir.xyz);

 HitResult firstHit = hitBVH(ray);
 //HitResult res = hitArray(0,nTriangles-1,ray);
 vec3 color;
 float blender = 1.0;
if(!firstHit.isHit) {
color = vec3(0);
blender = 0.0;
//color = sampleHdr(ray.direction);

}else{
    vec3 Le = firstHit.material.emissive;
    int maxBounce = 4;
    vec3 Li = pathTracing(firstHit, maxBounce);
    color = Le + Li;
}



vec3 lastColor = imageLoad(LastFrame,ivec2(gl_FragCoord.xy)).rgb;
color = mix(lastColor, color, 1.0/float(frameCounter+1));
imageStore(LastFrame,ivec2(gl_FragCoord.xy),vec4(color,1.0));
color = toneMapping(color, 1.5);
color = pow(color, vec3(1.0 / 2.2));
fragColor = vec4(color,blender);


}