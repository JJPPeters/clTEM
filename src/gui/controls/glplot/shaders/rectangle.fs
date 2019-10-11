#version 150

uniform vec4 RectCol;
uniform vec3 RectMins;
uniform vec3 RectMaxs;
uniform float pixel_size;
in vec4 PosPass;

out vec4 fragColour;

void main() {
    // set the colour of the rect
    fragColour = RectCol;

    float norm_x = (PosPass.x - RectMins.x) / (RectMaxs.x - RectMins.x);
    float norm_y = (PosPass.y - RectMins.y) / (RectMaxs.y - RectMins.y);
    float norm_z = (PosPass.z - RectMins.z) / (RectMaxs.z - RectMins.z);

    float line_w = 3.0f * pixel_size;
    float line_x = line_w / (RectMaxs.x - RectMins.x);
    float line_y = line_w / (RectMaxs.y - RectMins.y);
    float line_z = line_w / (RectMaxs.z - RectMins.z);

    if(norm_x > 1-line_x || norm_x < line_x || norm_y > 1-line_y || norm_y < line_y || norm_z > 1-line_z || norm_z < line_z )
        fragColour.w = 0.8f;
}