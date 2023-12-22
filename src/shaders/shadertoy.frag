#version 330 core
#define PI 3.1415926535
uniform vec4 iMouse;
uniform vec2 iResolution;

uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;

// iChannelN can be easily added with GLFrameBuffer-s textures

const vec2 poisson16[] = vec2[](vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725), vec2(-0.094184101, -0.92938870), vec2(0.34495938, 0.29387760), vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464), vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379), vec2(0.44323325, -0.97511554), vec2(0.53742981, -0.47373420), vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188), vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590), vec2(0.19984126, 0.78641367), vec2(0.14383161, -0.14100790));

vec2 poisson64[] = vec2[](vec2(-0.613392, 0.617481), vec2(0.170019, -0.040254), vec2(-0.299417, 0.791925), vec2(0.645680, 0.493210), vec2(-0.651784, 0.717887), vec2(0.421003, 0.027070), vec2(-0.817194, -0.271096), vec2(-0.705374, -0.668203), vec2(0.977050, -0.108615), vec2(0.063326, 0.142369), vec2(0.203528, 0.214331), vec2(-0.667531, 0.326090), vec2(-0.098422, -0.295755), vec2(-0.885922, 0.215369), vec2(0.566637, 0.605213), vec2(0.039766, -0.396100), vec2(0.751946, 0.453352), vec2(0.078707, -0.715323), vec2(-0.075838, -0.529344), vec2(0.724479, -0.580798), vec2(0.222999, -0.215125), vec2(-0.467574, -0.405438), vec2(-0.248268, -0.814753), vec2(0.354411, -0.887570), vec2(0.175817, 0.382366), vec2(0.487472, -0.063082), vec2(-0.084078, 0.898312), vec2(0.488876, -0.783441), vec2(0.470016, 0.217933), vec2(-0.696890, -0.549791), vec2(-0.149693, 0.605762),
    vec2(0.034211, 0.979980), vec2(0.503098, -0.308878), vec2(-0.016205, -0.872921), vec2(0.385784, -0.393902), vec2(-0.146886, -0.859249), vec2(0.643361, 0.164098), vec2(0.634388, -0.049471), vec2(-0.688894, 0.007843), vec2(0.464034, -0.188818), vec2(-0.440840, 0.137486), vec2(0.364483, 0.511704), vec2(0.034028, 0.325968), vec2(0.099094, -0.308023), vec2(0.693960, -0.366253), vec2(0.678884, -0.204688), vec2(0.001801, 0.780328), vec2(0.145177, -0.898984), vec2(0.062655, -0.611866), vec2(0.315226, -0.604297), vec2(-0.780145, 0.486251), vec2(-0.371868, 0.882138), vec2(0.200476, 0.494430), vec2(-0.494552, -0.711051), vec2(0.612476, 0.705252), vec2(-0.578845, -0.768792), vec2(-0.772454, -0.090976), vec2(0.504440, 0.372295), vec2(0.155736, 0.065157), vec2(0.391522, 0.849605), vec2(-0.620106, -0.328104), vec2(0.789239, -0.419965), vec2(-0.545396, 0.538133), vec2(-0.178564, -0.596057));

float sdBox(vec2 p)
{
    float scale = 0.78;
    float radius = 64;
    vec2 halfRes = iResolution / 2;
    vec2 d = abs(p - halfRes) - halfRes * scale + radius;
    return clamp(length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - radius, 0, 1);
}

// SHADERTOY CODE STARTS HERE
// Shader created by Inigo Quilez, 2020 - https://iquilezles.org/
// https://www.shadertoy.com/view/3tsyzl

#define TRAPS
#define CUT

const int SAMPLES = 1;
const int kNumIte = 200;
const float kPrecis = 0.00025;
const vec4 kC = vec4(-2, 6, 15, -6) / 22.0;
const float kFocLen = 2.5;

const float kBSRad = 2.0;

#define kNumBounces 3

// --------------------------------------
// oldschool rand() from Visual Studio
// --------------------------------------
int seed = 1;
int rand(void)
{
    seed = seed * 0x343fd + 0x269ec3;
    return (seed >> 16) & 32767;
}
float frandNotUpdate() { return float(seed & 32767) / 32767.0; }
float frand(void) { return float(rand()) / 32767.0; }

void srand(ivec2 p, int frame)
{
    int n = frame;
    n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589; // by Hugo Elias
    n += p.y;
    n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589;
    n += p.x;
    n = (n << 13) ^ n;
    n = n * (n * n * 15731 + 789221) + 1376312589;
    seed = n;
}

//--------------------------------------------------------------------------------
// http://amietia.com/lambertnotangent.html
//--------------------------------------------------------------------------------
vec3 cosineDirection(in vec3 nor)
{
    float u = frand() * 2.0 - 1.0;
    float a = frand() * 6.28318531;
    return normalize(nor + vec3(sqrt(1.0 - u * u) * vec2(cos(a), sin(a)), u));
}
//--------------------------------------------------------------------------------
// quaternion manipulation
//--------------------------------------------------------------------------------
vec4 qSquare(in vec4 q)
{
    q = q.yxzw;
    return vec4(q.x * q.x - q.y * q.y - q.z * q.z - q.w * q.w, 2.0 * q.x * q.yzw);
}

mat2 rotate(float angle)
{

    vec2 cs = vec2(cos(angle), sin(angle));
    return mat2(cs.x, -cs.y, cs.y, cs.x);
}

vec4 qCube(in vec4 q)
{
    // q = q.yxzw;
    float an = (iTime + frandNotUpdate() * iTimeDelta) / 3 * (float(PI) * 2);
    an = mod(an, 2 * PI);
    an /= PI; // 0...2

    bool flip = an > 1;
    an = flip ? 2 - an : an;
    an = pow(an, 6);
    an = flip ? 2 - an : an;
    an *= PI;

    q.xz *= rotate(an);
    vec4 q2 = q * q;
    return vec4(q.x * (q2.x - 3.0 * q2.y - 3.0 * q2.z - 3.0 * q2.w),
        q.yzw * (3.0 * q2.x - q2.y - q2.z - q2.w));
}
float qLength2(in vec4 q) { return dot(q, q); }

vec2 iSphere(in vec3 ro, in vec3 rd, in float rad)
{
    float b = dot(ro, rd);
    float c = dot(ro, ro) - rad * rad;
    float h = b * b - c;
    if (h < 0.0)
        return vec2(-1.0);
    h = sqrt(h);
    return vec2(-b - h, -b + h);
}
//--------------------------------------------------------------------------------
// build camera rotation matrix
//--------------------------------------------------------------------------------
mat3 setCamera(in vec3 ro, in vec3 ta, float cr)
{
    vec3 cw = normalize(ta - ro);
    vec3 cp = vec3(sin(cr), cos(cr), 0.0);
    vec3 cu = normalize(cross(cw, cp));
    vec3 cv = normalize(cross(cu, cw));
    return mat3(cu, cv, cw);
}
//--------------------------------------------------------------------------------
// SDF of the Julia set z³+c
// https://iquilezles.org/articles/distancefractals
//--------------------------------------------------------------------------------
vec2 map(in vec3 p)
{
    vec4 z = vec4(p, 0.0);
    float dz2 = 1.0;
    float m2 = 0.0;
    float n = 0.0;
#ifdef TRAPS
    float o = 1e10;
#endif

    for (int i = 0; i < kNumIte; i++) {
        // z' = 3z² -> |z'|² = 9|z²|²
        dz2 *= 9.0 * qLength2(qSquare(z));

        // z = z³ + c
        z = qCube(z) + kC;

        // stop under divergence
        m2 = qLength2(z);
        // orbit trapping : https://iquilezles.org/articles/orbittraps3d
        o = min(o, length(z.xz - vec2(0.45, 0.55)) - 0.1);

        if (m2 > 256.0)
            break;
        n += 1.0;
    }

    // sdf(z) = log|z|·|z|/|dz| : https://iquilezles.org/articles/distancefractals
    float d = 0.25 * log(m2) * sqrt(m2 / dz2);

    d = min(o, d); // traps

    d = max(d, p.y - o * 0.1); // cut

    return vec2(d, n);
}
#if 1
// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal(in vec3 pos)
{
    const vec2 e = vec2(1.0, -1.0) * 0.5773 * kPrecis;
    return normalize(e.xyy * map(pos + e.xyy).x + e.yyx * map(pos + e.yyx).x + e.yxy * map(pos + e.yxy).x + e.xxx * map(pos + e.xxx).x);
}
#else
// https://iquilezles.org/articles/juliasets3d
vec3 calcNormal(in vec3 p)
{
#ifdef TRAPS
    // the code below only works for the actual Julia set, not the traps
#endif

    vec4 z = vec4(p, 0.0);
    z.xyzw = z.yxzw;
    // identity derivative
    mat4x4 J = mat4x4(1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1);

    for (int i = 0; i < kNumIte; i++) {

        float k1 = 6.0 * z.x * z.y, k2 = 6.0 * z.x * z.z;
        float k3 = 6.0 * z.x * z.w, k4 = 2.0 * z.y * z.z;
        float k5 = 2.0 * z.y * z.w, k6 = 2.0 * z.z * z.w;
        float sx = z.x * z.x, sy = z.y * z.y;
        float sz = z.z * z.z, sw = z.w * z.w;
        float mx = 3.0 * sx - 3.0 * sy - 3.0 * sz - 3.0 * sw;
        float my = 3.0 * sx - 3.0 * sy - sz - sw;
        float mz = 3.0 * sx - sy - 3.0 * sz - sw;
        float mw = 3.0 * sx - sy - sz - 3.0 * sw;

        // chain rule of jacobians
        J = J * mat4x4(mx, -k1, -k2, -k3, k1, my, -k4, -k5, k2, -k4, mz, -k6, k3, -k5, -k6, mw);
        // q = q³ + c
        z = qCube(z) + kC;

        // exit condition
        if (dot2(z) > 256.0)
            break;
    }

    return (p.y > 0.0) ? vec3(0.0, 1.0, 0.0) : normalize((J * z).xyz);
}
#endif

vec2 raycast(in vec3 ro, in vec3 rd)
{
    float tmax = 7.0;
    float tmin = kPrecis;

// intersect clipping plane
#ifdef CUT
    // const float kSplit = 0.01;
    // float tpS = (kSplit - ro.y) / rd.y;
    // if (tpS > 0.0) {
    //     if (ro.y > kSplit)
    //         tmin = max(tmin, tpS);
    //     else
    //         tmax = min(tmax, tpS);
    // }
#endif

// intersect lower clipping plane
#if 0
    {
        float tpF = (-0.8 - ro.y) / rd.y;
        if (tpF > 0.0)
            tmax = min(tmax, tpF);
    }
#endif

// intersect bounding sphere
#if 1
    vec2 bv = iSphere(ro, rd, kBSRad);
    if (bv.y < 0.0)
        return vec2(-2.0, 0.0);
    tmin = max(tmin, bv.x);
    tmax = min(tmax, bv.y);
#endif

    // raymarch
    vec2 res = vec2(-1.0);
    float t = tmin;
    float lt = 0.0;
    float lh = 0.0;
    for (int i = 0; i < 1024; i++) {
        res = map(ro + rd * t);
        if (res.x < kPrecis)
            break;
        lt = t;
        lh = res.x;
#ifndef TRAPS
        t += min(res.x, 0.2);
#else
        t += min(res.x, 0.01) * (0.5 + 0.5 * frand());
#endif
        if (t > tmax)
            break;
    }
    // linear interpolation for better isosurface
    if (lt > 0.0001 && res.x < 0.0)
        t = lt - lh * (t - lt) / (res.x - lh);

    res.x = (t < tmax) ? t : -1.0;

    return res;
}

//--------------------------------------------------------------------------------
// color of the surface
//--------------------------------------------------------------------------------
vec3 colorSurface(in vec3 pos, in vec3 nor, in vec2 tn)
{
    vec3 col = 0.5 + 0.5 * cos(log2(tn.y) * 0.9 + 3.5 + vec3(0.0, 0.6, 1.0));
    if (pos.y > 0.0)
        col = mix(col, vec3(1.0), 0.2);
    float inside = smoothstep(14.0, 15.0, tn.y);
    col *= vec3(0.45, 0.42, 0.40) + vec3(0.55, 0.58, 0.60) * inside;
    col = mix(col * col * (3.0 - 2.0 * col), col, inside);
    col = mix(mix(col, vec3(dot(col, vec3(0.3333))), -0.4),
        col, inside);
    return clamp(col * 0.65, 0.0, 1.0);
}

//--------------------------------------------------------------------------------
// Render the scene through super simplified path-tracing
//--------------------------------------------------------------------------------
vec4 render(in vec2 fragCoord, in vec3 ro, in vec3 rd,
    out vec3 resPos, out float resT)
{
    vec3 colorMask = vec3(1.0);
    resT = 1e20;

    float alpha = 1 - sdBox(fragCoord);
    // path-tracing
    for (int bounce = 0; bounce < kNumBounces; bounce++) {
        vec2 tn = raycast(ro, rd);
        float t = tn.x;
        if (t < 0.0) {
            vec4 result;
            if (bounce > 0) {
                result = vec4(colorMask * 1.65 * step(0.0, rd.y), 1);
            } else {
                result = vec4(vec3(clamp(0.02 + 0.021 * rd.y, 0.0, 1.0)), 1);
                result.a *= alpha;
            }
            return result;
        } else {
            vec3 pos = ro + t * rd;
            vec3 nor = calcNormal(pos);

            if (bounce == 0) {
                resT = t;
                resPos = pos;
            }

            colorMask *= colorSurface(pos, nor, tn);
            rd = cosineDirection(nor);
            ro = pos + nor * kPrecis;
        }
    }

    return vec4(0, 0, 0, 1);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{

    vec3 ta = vec3(0.0, -0.3, 0.0);

    vec4 col = vec4(0);
    for (int i = 0; i < SAMPLES; ++i) {

        // float an = iTime / 3 * (PI * 2) / 3 + frand() * iTimeDelta;

        float an = 0;
        vec3 ro = 2.0 * vec3(sin(an), 0.8, cos(an));

        mat3x3 cam = setCamera(ro, ta, 0.0);
        vec2 p = (2.0 * fragCoord + poisson64[i % SAMPLES] - iResolution.xy) / iResolution.y;
        vec3 rd = normalize(cam * vec3(p.xy, kFocLen));

        ivec2 q = ivec2(fragCoord);
        srand(q, iFrame * SAMPLES + i);
        vec3 pos;
        float resT;
        col += render(fragCoord, ro, rd, pos, resT);
    }
    col /= float(SAMPLES);

    // color grade
    col.rgb = col.rgb * 2.0 / (1.0 + col.rgb);
    col.rgb = pow(col.rgb, vec3(0.4545));
    col.rgb = pow(col.rgb, vec3(0.85, 0.97, 1.0));
    col.rgb = col.rgb * 0.5 + 0.5 * col.rgb * col.rgb * (3.0 - 2.0 * col.rgb);

    fragColor = col;
}

// SHADERTOY CODE ENDS HERE

out vec4 FragColor;
void main()
{
    vec4 fragColor;
    vec2 fragCoord = vec2(gl_FragCoord);

    mainImage(fragColor, fragCoord);

    FragColor = fragColor;
}
