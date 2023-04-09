#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

struct Light {
	vec3 position;
	vec3 color;
};

uniform HammersleyBlock {
    float n;
    float hammersley[2 * 100]; 
} hb;

// G-Buffer textures
// TODO: Use image2d eventually
uniform sampler2D g_Position;
uniform sampler2D g_Normal;
uniform sampler2D g_Diffuse;
uniform sampler2D g_RoughMetal;

// Consists of depth values from the 
// global light's POV 
uniform sampler2D shadowMap;

// Irradiance Map for IBL
uniform sampler2D irradianceMap;
uniform sampler2D skyDomeMap;

// To convert from world space to 
// shadow-coord tex space
uniform mat4 shadowMat;

uniform bool iblOn;

// global light/sun
uniform Light globalLight;

uniform float width, height;

uniform float minDepth, maxDepth;

uniform float expControl;

uniform vec3 viewPos;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

//float GeometryGGX2(float NdotV, float roughness)
//{
//    float tanthetaV = sqrt(1.0f - (NdotV * NdotV)) / NdotV;
//    float denom = 1 + sqrt(1 + roughness * roughness * tanthetaV * tanthetaV);
//    return 2.0f / denom;
//}
//
//float GeometrySmith2(vec3 N, vec3 V, vec3 L, float roughness)
//{
//    float NdotV = max(dot(N, V), 0.0);
//    float NdotL = max(dot(N, L), 0.0);
//    float ggx2 = GeometryGGX2(NdotV, roughness);
//    float ggx1 = GeometryGGX2(NdotL, roughness);
//
//    return ggx1 * ggx2;
//}


float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (vec3(1) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec2 toUV(vec3 N)
{
    return vec2(0.5 - atan(N.z, N.x) / (2 * PI), acos(N.y) / PI);
}

vec3 Cholesky (float m11, float m12, float m13, float m22, float m23, 
                float m33, float z1, float z2, float z3) {

    float a = 1.0f;

    float b = m12 / a;
    float c = m13 / a;

    float d = sqrt(m22 - (b * b));
    if (d <= 0.0f) {
        d = 0.0001f;
    }

    float e = (m23 - (b * c)) / d;
    float f = sqrt(m33 - (c * c) - (e * e));
    if (f <= 0.0f) {
        f = 0.0001f;
    }

    float c1hat = z1 / a;
    float c2hat = (z2 - (b * c1hat)) / d;
    float c3hat = (z3 - (c * c1hat) - (e * c2hat)) / f;

    float c3 = c3hat / f;
    float c2 = (c2hat - (e * c3)) / d;
    float c1 = (c1hat - (b * c2) - (c * c3)) / a;

    return vec3(c1, c2, c3);
}

void main()
{		
    vec2 uv = gl_FragCoord.xy / vec2(width, height);
	vec3 FragPos = texture(g_Position, uv).rgb;
	vec3 N = texture(g_Normal, uv).rgb;
    float isSkySphere = texture(g_Diffuse, uv).a;
	vec3 albedo = texture(g_Diffuse, uv).rgb;
	float roughness = texture(g_RoughMetal, uv).r;
	float metalness = texture(g_RoughMetal, uv).g;

    //vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);

    // Calculate reflectance at normal incidence; If non-metal use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metalness);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // calculate per-light radiance
    vec3 L = normalize(globalLight.position - FragPos);
    vec3 H = normalize(V + L);
    float distance    = length(globalLight.position - FragPos);
    vec3 radiance     = globalLight.color;

    vec3 specular = vec3(0.0);
    float NdotL = max(dot(N, L), 0.0);                
    //vec3 kD = vec3(0.0);

    if (iblOn) {
        
        if (isSkySphere < 0) {
            Lo = albedo;
            //Lo  = expControl * Lo / (expControl * Lo + vec3(1.0));
            Lo = pow(Lo, vec3(1.0/2.2));
        }
        else {
    	    vec3 kS = FresnelSchlick(dot(H, V), F0);
		    vec3 kD = 1.0 - kS;
		    kD *= 1.0 - metalness;

            vec3 irradiance = texture(irradianceMap, toUV(N)).xyz;
            //irradiance = pow(irradiance, vec3(2.2));
            vec3 diffuse = (kD * albedo / PI) * irradiance * 2.0f;

            vec3 R = 2 * dot(N, V) * N - V;

            //A = normalize (Zaxis×R) = normalize (−Ry , Rx , 0)
            vec3 A = normalize(cross(vec3(0, 0, 1), R));
            
            //B = normalize (R×A)
            vec3 B = normalize(cross(R, A));

            for(int i = 0; i < hb.n * 2; i += 2) {

                float val1 = hb.hammersley[i];
                float val2 = hb.hammersley[i + 1];

                //θ = tan−1( αg √ξ2/√1−ξ2 )
                float theta = atan(roughness * sqrt(val1) / sqrt(1 - val1));
                //float theta = acos(pow(val2, 1.0/(roughness + 1.0)));
                vec2 currUV = vec2(val1, theta/PI);

                //( cos(2π (1/2−u)) sin (π v ), sin(2π(1/2−u)) sin(π v), cos(π v ))
                vec3 currL = vec3(
                    cos(2 * PI * (0.5 - currUV.x)) * sin(PI * currUV.y),
                    sin(2 * PI * (0.5 - currUV.x)) * sin(PI * currUV.y), 
                    cos(PI * currUV.y)
                );

                //ωk = normalize (L. x A + L. y B + L. z R)
                vec3 wK = normalize(currL.x * A + currL.y * B + currL.z * R);
                H = normalize(wK + V);

                float level = 0.5 * log2(width * height / hb.n) - 0.5 * log2(DistributionGGX(H, N, roughness));

                vec3 Li = textureLod(skyDomeMap, toUV(wK), level).xyz;

                specular += cos(theta) * Li * GeometrySmith(N, V, wK, roughness) 
                * FresnelSchlick(max(dot(wK,H), 0.0), kS) / (4.0 * max(dot(N, V), 0.0) * max(dot(N, wK), 0.0) + 0.0001);
            }
            specular /= hb.n;

            // Tone-mapping
            //specular = expControl * specular / (expControl * specular + vec3(1.0));
            //specular = pow(specular, vec3(1.0/2.2));

            Lo += (diffuse + specular);
        }
    }
    else {
        
        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);       
    
        // ks
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metalness;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        specular     = numerator / denominator;  

        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }


    //Lo += (kD * albedo / PI) * radiance * NdotL; 
    //Lo += specular * radiance * NdotL; 

    vec3 ambient = vec3(0.03) * albedo;

    vec4 shadowCoord = shadowMat * vec4(FragPos, 1.0f);
    vec2 shadowIndex = shadowCoord.xy / shadowCoord.w;

//    VSM
//    float amtLit = 1.0f;
//
//    float relativePixelDepth;
//
//    if (shadowCoord.w > 0 &&
//    shadowIndex.x >= 0.0f && shadowIndex.x <= 1.0f &&
//    shadowIndex.y >= 0.0f && shadowIndex.y <= 1.0f) {
//
//         relativePixelDepth = (shadowCoord.w - minDepth) / (maxDepth - minDepth);
//         vec4 moments = texture(shadowMap, shadowIndex);
//         float relativeLightDepth = moments[0];
//
//        if (relativePixelDepth > relativeLightDepth) {
//            //shadow = 1.0f;
//            float mean = moments[0];
//            float variance = moments[1] - (mean * mean);
//
//            float diff = relativePixelDepth - mean;
//            amtLit = variance / (variance + (diff * diff));
//        }
//    }
//
//    amtLit = clamp(amtLit, 0, 1);
//    amtLit = amtLit * amtLit * amtLit;
    
    //4-msm

    float G = 0.0f;

    if (shadowCoord.w > 0 &&
    shadowIndex.x >= 0.0f && shadowIndex.x <= 1.0f &&
    shadowIndex.y >= 0.0f && shadowIndex.y <= 1.0f) {

        float relativePixelDepth = (shadowCoord.w - minDepth) / (maxDepth - minDepth);

        float alpha = 0.001f; 
        vec4 b = texture(shadowMap, shadowIndex);

        vec4 bHat = ((1 - alpha) * b) + (alpha * vec4(0.5));

        float m11 = 1.0f;
        float m12 = bHat[0]; 
        float m13 = bHat[1];
        float m22 = bHat[1];
        float m23 = bHat[2];
        float m33 = bHat[3];

        float z1 = 1.0f;
        float z2 = relativePixelDepth;
        float z3 = relativePixelDepth * relativePixelDepth;
    
        vec3 c = Cholesky(m11, m12, m13, m22, m23, m33, z1, z2, z3);
        
        // Sqrt discr. = b^2 - 4ac
        float sqrtDiscr = sqrt((c[1] * c[1]) - (4.0f * c[2] * c[0]));

        z2 = 0.0f;
        z3 = 0.0f;

        z2 = (-c[1] - sqrtDiscr) / (2.0f * c[2]);

        // z2 should always be the smaller root
        if (c[2] < 0.0f) {
            z3 = z2;
            z2 = (-c[1] + sqrtDiscr) / (2.0f * c[2]);
        }
        else { z3 = (-c[1] + sqrtDiscr) / (2.0f * c[2]); }

        if (relativePixelDepth <= z2) { G = 0.0f; }
        
        else if (relativePixelDepth <= z3) {
            G = ((relativePixelDepth * z3) - (bHat[0] * (relativePixelDepth + z3)) + (bHat[1]))
                / ((z3 - z2) * (relativePixelDepth - z2));
        }
        else { 
            G = 1 - (((z2 * z3) - (bHat[0] * (z2 + z3)) + (bHat[1]))
                 / ((relativePixelDepth - z2) * (relativePixelDepth - z3)));
        }
    }

    float amtLit = (1 - G) * (1 - G) * (1 - G);
    vec3 color = ambient;
    //vec3 color = ambient + (Lo * amtLit);		
    if (iblOn) { 
        color += Lo;
    }
    else { 
        color += (Lo * amtLit);  
    }		
    //color = expControl * color / (expControl * color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}