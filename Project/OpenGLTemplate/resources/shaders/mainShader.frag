#version 400 core
// Input from vertex shader
in vec3 vColour;
in vec2 vTexCoord;
in vec3 worldPosition;
in vec3 eyePosition;
in vec3 eyeNormal;

// Output
out vec4 vOutputColour;

// Uniforms
uniform sampler2D sampler0;
uniform samplerCube CubeMapTex;
uniform bool bUseTexture;
uniform bool renderSkybox;

// Light structure with parameters for spotlights
struct LightInfo {
    vec4 position;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
    vec3 direction; //Direction of spotlight
    float exponent; //Spotlight falloff
    float cutoff; //Spotlight cone angle
};

// Material properties
struct MaterialInfo {
    vec3 Ma;
    vec3 Md;
    vec3 Ms;
    float shininess;
};

// Uniform light and material
uniform LightInfo light1;
uniform MaterialInfo material1;
uniform int numActiveLights;
uniform LightInfo trackLights[16];

// Convert lighting into toon shaders by applying bands of colours
float toonify(float intensity) {
    if (intensity > 0.95) return 1.2;       // Brightest areas
    else if (intensity > 0.75) return 0.95;  // Bright areas
    else if (intensity > 0.5) return 0.7;   // Medium-lit areas
    else if (intensity > 0.25) return 0.45;  // Shadowy areas
    else return 0.25;                       // Dark areas
}

// Color enhancement 
vec3 enhanceColors(vec3 color) {
    // Boost saturation 
    float luminance = 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;
    return mix(vec3(luminance), color, 1.2); //Mix between luminance and color with boosted saturation
}

vec3 ApplySpotlight(LightInfo light, vec3 position, vec3 normal) {
    vec3 lightDir = light.position.xyz - position;
    float distance = length(lightDir);
    vec3 s = normalize(lightDir); //Normalize light direction vector

    vec3 ambient = light.La * material1.Ma * 0.7; //*0.7 to dim
    float cosAngle = dot(-s, normalize(light.direction)); //Get angle between light and its direction

    // Convert spot angle light into bands
    float spotEffect = pow(max(cosAngle, 0.0), light.exponent);
    float bandedSpot = floor(spotEffect * 5.0) / 5.0; // 5 bands in the cone

    // Quantize distance attenuation into bands
    float attenuation = 1.0;
    float maxRange = 80.0;
    if (distance > 20.0) {
        attenuation = 1.0 - min(1.0, (distance - 20.0) / (maxRange - 20.0));
    }
    float bandedAttenuation = floor(attenuation * 4.0) / 4.0; // Convert into 4 bands of light

    //Calculate diffuse lighting factor and apply toon effect to it
    float sDotN = max(dot(s, normal), 0.0);
    float toonDiffuse = toonify(sDotN);

    vec3 diffuse = light.Ld * material1.Md * toonDiffuse * 0.8;

    vec3 v = normalize(-position); //View direction 
    vec3 h = normalize(v + s);  // Half vector for Blinn-Phong specular calculation
    float specDot = max(dot(h, normal), 0.0);
    float toonSpec = (pow(specDot, material1.shininess) > 0.6) ? 0.7 : 0.0; //Toonify specular angle factor
    vec3 specular = light.Ls * material1.Ms * toonSpec * 0.7; //Specular contribution to lighting

    return ambient + (bandedAttenuation * bandedSpot * (diffuse + specular)); //Return total light contribution

}

void main() {
    if (renderSkybox) {
        // render skybox with a slight toon effect
        vec4 skyColor = texture(CubeMapTex, worldPosition);
        skyColor.rgb = enhanceColors(skyColor.rgb * 0.8); // Brighter skybox
        vOutputColour = skyColor;
    } else {
        // Increse base global lighting
        vec3 lightSum = light1.La * material1.Ma * 2.5; // Significant boost to overall scene brightness
        
        vec3 normal = normalize(eyeNormal);
        
        // Add spotlight contributions from all track lights
        for (int i = 0; i < numActiveLights && i < 16; i++) {
            lightSum += ApplySpotlight(trackLights[i], eyePosition, normal);
        }
        
        // Apply toon edge detection for outlines
        vec3 v = normalize(-eyePosition);
        float edgeFactor = smoothstep(0.2, 0.4, dot(v, normal));
        vec3 toonColour = mix(lightSum * 0.4, lightSum, edgeFactor);
        
        // Apply texture with toon banding
        vec4 texColor = texture(sampler0, vTexCoord);
            
        // Apply toon shader banding to texture
        texColor.r = mix(texColor.r, toonify(texColor.r), 0.7);
        texColor.g = mix(texColor.g, toonify(texColor.g), 0.7);
        texColor.b = mix(texColor.b, toonify(texColor.b), 0.7);
            
        // Boost overall brightness
        vOutputColour = texColor * vec4(toonColour, 1.0) * 1.4; // Increased multiplier for brighter scene
    }
}
