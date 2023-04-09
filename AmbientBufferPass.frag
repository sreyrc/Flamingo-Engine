#version 430

layout (location = 0) out vec4 g_AOBuffer;

uniform int width, height;
uniform vec3 eyePos;
uniform float s, k, R;

uniform sampler2D g_Position;
uniform sampler2D g_Normal;

const float PI = 3.141592;

void main()
{
	int n = 10;
	float c = 0.1 * R;
	float delta = 0.001;

	ivec2 integerCoeff = ivec2(gl_FragCoord.xy);
	vec2 floatingCoords = gl_FragCoord.xy / vec2(width, height);
	vec3 N = texture2D(g_Normal, floatingCoords).xyz;
    vec3 P = texture2D(g_Position, floatingCoords).xyz;
	float d = length(eyePos-P.xyz);
	
	float phi = (30 * integerCoeff.x) ^ integerCoeff.y + 10 * integerCoeff.x * integerCoeff.y;
	float sum = 0;
	for(int i = 0; i < n; ++i)
	{
		float alpha = (i + 0.5f)/n;
		float h = alpha * R/d;
		float theta = 2 * PI * alpha * (7.0 * n / 9.0) + phi;

		vec3 Pi = texture2D(g_Position, floatingCoords + h * vec2(cos(theta), sin(theta))).xyz;
		vec3 wi = Pi - P;
		float di = length(eyePos-Pi.xyz);
		float H = 1.0f;
		if (R-length(wi) < 0.0f)
			H = 0.0f;

		sum += max(0.0, dot(N,wi) - delta * di) * H / max(c*c, dot(wi,wi));
	}

	sum *= 2 * PI * c/n;
	float result = pow((1.0 - s * sum), k);

	//if (s * sum > 1.0) result = 0.0;

	g_AOBuffer = vec4(vec3(result), 1.0f);
	//g_AOBuffer = vec4(1.0f);
}