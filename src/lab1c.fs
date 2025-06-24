#version 400

/*
 * Simple fragment shader for point light source.
 * Light computations performed in world coordinates.
 */
in vec3 normal;
in vec4 position;

uniform vec3 eye;
uniform vec3 objectColor; // Receive the color uniform

out vec4 colour;

void main() {
    vec3 N;
    vec3 Lposition = vec3(500.0, 500.0, 800.0); // Light position
    vec4 Mcolour = vec4(objectColor, 1.0); // Object colour
    vec4 Lcolour = vec4(1.0, 1.0, 1.0, 1.0); // Light colour
    vec3 H;
    float diffuse;
    float specular;
    float n = 100.0;
    vec3 L;

    N = normalize(normal);
    L = Lposition - position.xyz;
    L = normalize(L);
    H = normalize(L + eye);

    diffuse = dot(N, L);

    if (diffuse < 0.0) {
        diffuse = 0.0;
        specular = 0.0;
    } else {
        specular = pow(max(0.0, dot(N, H)), n);
    }

    colour = min(0.3 * Mcolour + diffuse * Mcolour * Lcolour + Lcolour * specular, vec4(1.0));
    colour.a = Mcolour.a;
}
