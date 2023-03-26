#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

struct Light {
	vec3 position;
	vec3 color;
};

// G-Buffer textures
// TODO: Use image2d eventually
uniform sampler2D g_Position;
uniform sampler2D g_Normal;
uniform sampler2D g_Diffuse;
uniform sampler2D g_RoughMetal;

// Consists of depth values from the 
// global light's POV 
uniform sampler2D shadowMap;

// To convert from world space to 
// shadow-coord tex space
uniform mat4 shadowMat;

// global light/sun
uniform Light globalLight;

uniform float width, height;

uniform float minDepth, maxDepth;

uniform vec3 viewPos;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 Cholesky (mat3 m, float z1, float z2 , float z3) {
    float a = sqrt(m[0][0]);
    if (a <= 0) a = 0.0001f;

    float b = m[0][1] / a;
    float c = m[0][2] / a;
    float d = sqrt(m[1][1] - (b * b));
    if (d <= 0) d = 0.0001f;
    
    float e = (m[1][2] - (b * c)) / d;
    
    float f = sqrt(m[2][2] - (c * c) - (e * e));
    if (f <= 0) f = 0.0001f;

    float cHat1 = z1 / a;
    float cHat2 = (z2 - (b * cHat1)) / d;
    float cHat3 = (z3 - (c * cHat1) - (e * cHat2)) / f;
    
    float c3 = cHat3 / f;
    float c2 = (cHat2 - (e * c3)) / d;
    float c1 = (cHat1 - (b * c2) - (c * c3)) / d;

    return vec3(c1, c2, c3);
}

void main()
{		
    vec2 uv = gl_FragCoord.xy / vec2(width, height);
	vec3 FragPos = texture(g_Position, uv).rgb;
	vec3 N = texture(g_Normal, uv).rgb;
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
        
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);        
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
    
    // ks
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metalness;	  
        
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;  
            
    float NdotL = max(dot(N, L), 0.0);                
    Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    //Lo += (kD * albedo / PI) * radiance * NdotL; 
    //Lo += specular * radiance * NdotL; 

    vec3 ambient = vec3(0.03) * albedo;

    vec4 shadowCoord = shadowMat * vec4(FragPos, 1.0f);
    vec2 shadowIndex = shadowCoord.xy / shadowCoord.w;

    float amtLit = 1.0f;
    float shadow = 0.0f;

    float relativePixelDepth;

    if (shadowCoord.w > 0 &&
    shadowIndex.x >= 0.0f && shadowIndex.x <= 1.0f &&
    shadowIndex.y >= 0.0f && shadowIndex.y <= 1.0f) {

         relativePixelDepth = (shadowCoord.w - minDepth) / (maxDepth - minDepth);
         vec4 moments = texture(shadowMap, shadowIndex);
         float relativeLightDepth = moments[0];

        if (relativePixelDepth > relativeLightDepth) {
            //shadow = 1.0f;
            float mean = moments[0];
            float variance = moments[1] - (mean * mean);

            float diff = relativePixelDepth - mean;
            amtLit = variance / (variance + (diff * diff));
        }
    }

    amtLit = clamp(amtLit, 0, 1);
    //4-msm

//    if (shadowCoord.w > 0 &&
//    shadowIndex.x >= 0.0f && shadowIndex.x <= 1.0f &&
//    shadowIndex.y >= 0.0f && shadowIndex.y <= 1.0f) {
//
//        float zf = shadowCoord.w; // frag depth
//        float alpha = 0.002f; 
//        vec4 b = texture(shadowMap, shadowIndex);
//
//        b = ((1 - alpha) * b) + (alpha * vec4(0.5, 0.5, 0.5, 0.5));
//    
//        vec3 column0 = vec3(1.0, b[0], b[1]);
//        vec3 column1 = vec3(b[0], b[1], b[2]);
//        vec3 column2 = vec3(b[1], b[2], b[3]);
//        mat3 m = mat3(column0, column1, column2); 
//
//        // (c1, c2, c3)
//        // or (c, b, a)
//        vec3 c = Cholesky(m, 1, zf, zf * zf);
//
//        // discr. = b^2 - 4ac
//        float discriminant = (c[1] * c[1]) - (4 * c[2] * c[0]);
//        float z2 = (-c[1] - sqrt(discriminant)) / (2 * c[2]);
//        float z3 = 0.0f;
//
//        if (c[2] < 0.0f) {
//            z3 = z2;
//            z2 = (-c[1] + sqrt(discriminant)) / (2 * c[2]);
//        }
//        else { z3 = (-c[1] + sqrt(discriminant)) / (2 * c[2]); }
//
//        if (zf <= z2) shadow = 0.0f;
//        else if (zf <= z3) { 
//            shadow = ((zf * z3) - (b[0] * (zf + z3)) + b[1]) / ((z3 - z2) * (zf - z2));
//        }
//        else {
//            shadow = 1 - ( ((z2 * z3) - (b[0] * (z2 + z3)) + b[1]) / ((zf - z2) * (zf - z3)) );
//        }
//    }


//    float outlineFactor = dot(V, N);
//
//    if(outlineFactor > 0.3f) { outlineFactor = 1.0f; }
//    else { outlineFactor = 0.0f; }

    //vec3 color = ambient + (Lo * (1 - shadow));	
    //vec3 color = vec3(relativePixelDepth);
    vec3 color = ambient + (Lo * amtLit);	
    //vec3 color = ambient + Lo;	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    FragColor = vec4(color, 1.0);
   
}