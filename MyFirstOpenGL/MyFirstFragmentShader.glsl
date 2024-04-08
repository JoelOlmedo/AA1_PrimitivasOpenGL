#version 440 core

uniform vec2 windowSize;
uniform vec3 colorAbove; // Color para los píxeles por encima de la mitad de la ventana
uniform vec3 colorBelow; // Color para los píxeles por debajo de la mitad de la ventana

out vec4 fragColor;

void main() {
    // Obtener la coordenada Y del fragmento en el espacio de pantalla
    float fragmentY = gl_FragCoord.y;
    
    // Calcular la mitad de la altura de la ventana
    float halfScreenHeight = windowSize.y / 2.0;

    // Comprobar si el fragmento está por encima o por debajo de la mitad de la pantalla
    if (fragmentY > halfScreenHeight) {
        // Por encima de la mitad: asignar el color correspondiente
        fragColor = vec4(colorAbove, 1.0);
    } else {
        // Por debajo de la mitad: asignar el color correspondiente
        fragColor = vec4(colorBelow, 1.0);
    }
}
