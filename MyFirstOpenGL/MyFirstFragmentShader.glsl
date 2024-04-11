#version 440 core

uniform vec2 windowSize;
uniform vec3 objectColor;

out vec4 fragColor;

void main() {
    // Obtener la coordenada Y del fragmento en el espacio de pantalla
    float fragmentY = gl_FragCoord.y;
    
    // Calcular la mitad de la altura de la ventana
    float halfScreenHeight = windowSize.y / 2.0;

        fragColor = vec4(objectColor, 1.0);    

}
