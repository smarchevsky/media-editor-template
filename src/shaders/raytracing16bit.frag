#version 330 core
#define IDENTITY mat4(1,0,0,0,  0,1,0,0,  0,0,1,0,  0,0,0,1)
#define IDENTITY2 mat4(0,1,0,0,  0,0,1,0,  1,0,0,0,  0,0,0,1)

in VS_OUT {
    vec4 wPos;  // world pos
    vec3 n;
    vec2 uv;
} vs;

out vec4 color;

uniform vec3 cameraPosition;

uniform mat4 modelWorld;
uniform mat4 modelWorldInv;

uniform sampler2D texGeometry;
uniform ivec2 texGeometrySize;

//#define debugShowBVH

//------------------- STACK -----------------------

int countTI = 0;
int _stack[15];
int _index = -1;
void stackClear() { _index = -1; }
int stackSize() { return _index + 1; }
void stackPush(in int node) { if(_index > 14) discard; _stack[++_index] = node; }
int stackPop() { return _stack[_index--]; }

//------------------- STRUCTS -----------------------

struct Node
{
    int leftChild;
    int rightChild;
    vec3 aabbMin;
    vec3 aabbMax;
};

struct Ray
{
    vec3 origin;
    vec3 direction;
    float tStart;
    float tEnd;
    #ifdef debugShowBVH
    int nodesVisited;
    #endif
};

struct Hit
{
    vec3 position;
    vec3 normal;
    vec2 uv;
    bool isHit;
};

struct Vertex {
    vec3 p;
    vec3 n;
    vec2 t;
};

struct IndexedTriangle
{
   Vertex v0;
   Vertex v1;
   Vertex v2;
};

//------------------- GETTERS -----------------------

uvec4 getData(int index)
{
    int x = index % texGeometrySize.x;
    int y = index / texGeometrySize.x;
    vec2 uvPos = vec2(x / float(texGeometrySize.x), y / float(texGeometrySize.y));
    return floatBitsToUint(texture(texGeometry, uvPos));
}

/*
    const ivec4 i4_2e15 = ivec4(32768);
    const ivec4 i4_2e16 = ivec4(65536);
    dataL = (ivec4(data & uvec4(0x0000ffffu)) + i4_2e15) % i4_2e16 - i4_2e15;
    dataR = (ivec4(data >> uvec4(16))         + i4_2e15) % i4_2e16 - i4_2e15;
*/

#define NORM(x) x = x / 32767. - 1.;
Node getNode(int index)
{
    //isLeftPixel ? leftRight >> 16 : (leftRight & 0x0000ffffu);
    uvec4 data, dataL, dataR;
    data = getData(index);
    dataL = data & uvec4(0x0000ffffu);
    dataR = data >> uvec4(16);

    Node node;
    node.leftChild  = int(dataL.r);
    node.rightChild = int(dataR.r);
    node.leftChild =  (node.leftChild  + 32768) % 65536 - 32768;
    node.rightChild = (node.rightChild + 32768) % 65536 - 32768;

    node.aabbMin = vec3(dataL.gba);
    node.aabbMax = vec3(dataR.gba);
    NORM(node.aabbMin)
    NORM(node.aabbMax)

    return node;
}

IndexedTriangle getIndexedTriangle(int triIndex)
{
    uvec2 triIndices = getData(triIndex).rg;
    uint i0 = triIndices.r & 0x0000ffffu;
    uint i1 = triIndices.r >> 16;
    uint i2 = triIndices.g & 0x0000ffffu;

    uvec4 data, dataL, dataR;
    IndexedTriangle triangle;

    data = getData(int(i0));
    dataL = data & uvec4(0x0000ffffu);
    dataR = data >> uvec4(16);

    triangle.v0.p = vec3(dataL.rgb);
    triangle.v0.n = vec3(dataR.rgb);
    triangle.v0.t = vec2(dataL.a, dataR.a);
    NORM(triangle.v0.p);
    NORM(triangle.v0.n);
    NORM(triangle.v0.t);


    data = getData(int(i1));
    dataL = data & uvec4(0x0000ffffu);
    dataR = data >> uvec4(16);

    triangle.v1.p = vec3(dataL.rgb);
    triangle.v1.n = vec3(dataR.rgb);
    triangle.v1.t = vec2(dataL.a, dataR.a);
    NORM(triangle.v1.p);
    NORM(triangle.v1.n);
    NORM(triangle.v1.t);


    data = getData(int(i2));
    dataL = data & uvec4(0x0000ffffu);
    dataR = data >> uvec4(16);

    triangle.v2.p = vec3(dataL.rgb);
    triangle.v2.n = vec3(dataR.rgb);
    triangle.v2.t = vec2(dataL.a, dataR.a);
    NORM(triangle.v2.p);
    NORM(triangle.v2.n);
    NORM(triangle.v2.t);


    return triangle;
}

//------------------- INTERSECTIONS -----------------------

bool slabs(in Ray ray, in vec3 minB, in vec3 maxB, inout float localMin) {

    if(all(greaterThan(ray.origin, minB)) && all(lessThan(ray.origin, maxB)))
        return true;

    vec3 t0 = (minB - ray.origin)/ray.direction;
    vec3 t1 = (maxB - ray.origin)/ray.direction;
    vec3 tmin = min(t0, t1), tmax = max(t0, t1);
    float tminf = max(max(tmin.x, tmin.y), tmin.z);
    float tmaxf = min(min(tmax.x, tmax.y), tmax.z);

    if (tminf > tmaxf)
        return false;

    localMin = tminf;
    return tminf < ray.tEnd && tminf > ray.tStart;
}

bool isect_tri(inout Ray ray, in IndexedTriangle tri, inout Hit hit) {
        vec3 e1 = tri.v1.p - tri.v0.p;
        vec3 e2 = tri.v2.p - tri.v0.p;
    // if(dot(cross(e1,e2), ray.direction) > 0) return false; // backface culling

        vec3 P = cross(ray.direction, e2);
        float det = dot(e1, P);
        // if (abs(det) < 1e-10) // small parts can disappear here
        //     return false;

        float inv_det = 1. / det;
        vec3 T = (ray.origin - tri.v0.p);
        float u = dot(T, P) * inv_det;
        if (u < 0.0 || u > 1.0)
        return false;

        vec3 Q = cross(T, e1);
        float v = dot(ray.direction, Q) * inv_det;
        if (v < 0.0 || (v+u) > 1.0)
        return false;

        float tt = dot(e2, Q) * inv_det;

    if(ray.tEnd > tt && ray.tStart < tt )
    {
        vec3 c = vec3(u, v, 1.0 - u - v);
        countTI++;
        hit.position = (ray.origin + ray.direction * tt);
        hit.normal = normalize(tri.v0.n * c.z + tri.v1.n * c.x + tri.v2.n * c.y);
        hit.uv = tri.v0.t * c.z + tri.v1.t * c.x + tri.v2.t * c.y;

        hit.isHit = true;
        ray.tEnd = tt;
        return true;
    }
    return false;
}

//------------------- TRACE -----------------------

void traceCloseHitV2(inout Ray ray, inout Hit hit)
{
    stackClear();
    stackPush(0);
    hit.isHit = false;
    Node select;
    IndexedTriangle try;
    float tempt;

    for(int i = 0; (i < 1024) && (stackSize() > 0); i++)
    {
        #ifdef debugShowBVH
        ray.nodesVisited++;
        #endif
        select = getNode(stackPop());



        if(!slabs(ray, select.aabbMin, select.aabbMax, tempt))
            continue;

        if(select.leftChild > 0 && select.rightChild > 0)
        {
            float leftMinT = 0;
            float rightMinT = 0;
            Node right = getNode(select.rightChild);
            Node left = getNode(select.leftChild);
            bool rightI = slabs(ray, right.aabbMin, right.aabbMax,  rightMinT);
            bool leftI = slabs(ray, left.aabbMin, left.aabbMax,  leftMinT);

            if(rightI && leftI)
            {
                if (rightMinT < leftMinT)
                {
                    stackPush(select.leftChild);
                    stackPush(select.rightChild);
                }
                else
                {
                    stackPush(select.rightChild);
                    stackPush(select.leftChild);
                }
                continue;
            }
            if(rightI)
                stackPush(select.rightChild);
            else
                stackPush(select.leftChild);
            continue;
        }

        if (select.rightChild > 0)
            stackPush(select.rightChild);

        if (select.leftChild > 0)
            stackPush(select.leftChild);

        if(select.rightChild <= 0)
        {
            try = getIndexedTriangle(-select.rightChild);
            isect_tri(ray, try, hit);
        }

        if(select.leftChild <= 0)
        {
            try = getIndexedTriangle(-select.leftChild);
            isect_tri(ray, try, hit);
        }
    }
}


vec2 hash2(vec3 p3) {
        p3 = fract(p3 * vec3(5.3983, 5.4427, 6.9371));
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.xx + p3.yz) * p3.zy);
}

mat4 rotationMatrix(float angle)
{
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(c,   -s,       0,  0.0,
                s,    c,       0,  0.0,
                0,    0,  oc + c,  0.0,
                0,    0,       0,  1.0);
}

void main() {
    vec2 dither = hash2(vec3(gl_FragCoord.xy, 0));
    mat4 finalMatrix = modelWorldInv;
    // finalMatrix = rotationMatrix(dither.x * .4) * finalMatrix;

    vec3 viewDir = normalize(vs.wPos.xyz - cameraPosition);

    viewDir =         (finalMatrix * vec4(viewDir,        0.)).xyz;
    vec3 viewOrigin = (finalMatrix * vec4(cameraPosition, 1.)).xyz;

    Ray ray;
    ray.direction = viewDir;
    ray.origin = viewOrigin;
    ray.tStart = 0.0001;
    ray.tEnd = 10000;
    #ifdef debugShowBVH
    ray.nodesVisited = 0;
    #endif

    Hit hit;
    traceCloseHitV2(ray, hit);
    color = vec4(hit.normal * .5 + 0.5, 1.0);

    #ifdef debugShowBVH
    color.rgb += vec3(sqrt(ray.nodesVisited) * 0.05);
    #endif

}
