#version 330 core

out vec4 FragColor;

struct Light {
	vec3 position;
	vec3 color;
    float r;
};

// G-Buffer textures
// TODO: Use image2d eventually
uniform sampler2D g_Position;
uniform sampler2D g_Normal;
uniform sampler2D g_Diffuse;
uniform sampler2D g_RoughMetal;

uniform Light localLight;

uniform float width, height;

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


void main()
{	
    vec2 uv = gl_FragCoord.xy / vec2(width, height);
    vec3 FragPos = texture(g_Position, uv).rgb;

    float distFromLight = length(localLight.position - FragPos);

    // If outside of range of light
    if (distFromLight > localLight.r) 
    {
        FragColor = vec4(0);
    }
    else {
        // Grab necessary vlaues for lighting calculation 
	    vec3 N = texture(g_Normal, uv).rgb;
	    vec3 albedo = texture(g_Diffuse, uv).rgb;
	    float roughness = texture(g_RoughMetal, uv).r;
	    float metalness = texture(g_RoughMetal, uv).g;

        vec3 V = normalize(viewPos - FragPos);

        vec3 F0 = vec3(0.04); 
        F0 = mix(F0, albedo, metalness);

        // Reflectance equation
        vec3 Lo = vec3(0.0);

        // Calculate per-light radiance
        vec3 L = normalize(localLight.position - FragPos);
        vec3 H = normalize(V + L);
        float attenuation = (1.0 / (distFromLight * distFromLight)) - 
        (1.0 / (localLight.r * localLight.r));
        vec3 radiance = localLight.color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metalness;	  

        float NdotL = max(dot(N, L), 0.0);        
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;  

        vec3 color = Lo;

        // HDR tonemapping
        color = color / (color + vec3(1.0));

        FragColor = vec4(color, 1.0);
    }
}