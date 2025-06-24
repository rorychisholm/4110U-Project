#version 420 core

in vec3 normal;   // Interpolated normal from vertex shader
in vec4 position; // World-space position of the fragment


uniform vec3 eye;       // Camera/view position
uniform float displace;

out vec4 fragColor;     // Output fragment colour

void main() {
    // Material and light colours
   
    vec3 colour;
    float height = position.y;  // Access the z component of the position
    vec3 Lposition = vec3(500.0, 500.0, 800.0); // Light position

    /*
    if (height < 0.3 * displace) {
        // Water (low height): Blue
        colour = mix(vec3(0.0, 0.0, 0.4), vec3(0.0, 0.0, 0.5), height / 0.3);
    } else if (height < 0.6 * displace) {
        // Land (medium height): Green
        colour = mix(vec3(0.0, 0.1, 0.0), vec3(0.0, 0.15, 0.0), (height - 0.3) / 0.3);
    } else if (height < 0.9 * displace) {
        // Mountains (high height): Grey
        colour = mix(vec3(0.2, 0.2, 0.2), vec3(0.225, 0.225, 0.225), (height - 0.6) / 0.3);
    } else {
        // Snow (very high height): White
        colour = mix(vec3(0.9, 0.9, 0.9), vec3(1.0, 1.0, 1.0), (height - 0.9) / 0.1);
    }
    */
    // Meadow Green Gradient
    colour = mix(vec3(0.1, 0.4, 0.1), vec3(0.3, 0.6, 0.2), height / displace);



    vec4 Lcolour = vec4(1.0, 1.0, 1.0, 1.0);  // Light colour (white)
    vec4 Mcolour = vec4(colour, 1.0);  // Object colour (greenish)

    vec3 N = normalize(normal);                 // Normalize the interpolated normal
    vec3 L = normalize(Lposition - position.xyz); // Direction from fragment to light
    vec3 V = normalize(eye - position.xyz);      // Direction from fragment to camera/view

    // Halfway vector for Blinn-Phong specular calculation
    vec3 H = normalize(L + V);  

    // Diffuse lighting calculation (Lambertian reflectance)
    float diffuse = max(dot(N, L), 1.0);

    // Specular lighting calculation (Blinn-Phong reflection model)
    float shininess = 500.0;
    float specular = pow(max(dot(N, H), 0.0), shininess);

    // Combine lighting results (ambient + diffuse + specular)
    vec4 ambient = 0.3 * Mcolour;  // Ambient light contribution
    vec4 diffuseComponent = diffuse * Mcolour * Lcolour;  // Diffuse component
    vec4 specularComponent = specular * Lcolour;  // Specular component

    // Final colour (clamp to 1.0 to avoid over-brightness)
    fragColor = min(ambient + diffuseComponent + specularComponent, vec4(1.0));
}
