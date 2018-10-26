#version 150

uniform vec4 RectCol;
uniform vec3 RectMins;
uniform vec3 RectMaxs;
in vec4 PosPass;

void main() {
    // set the colour of the rect
    gl_FragColor = RectCol;

    //
    float norm_x = (PosPass.x - RectMins.x) / (RectMaxs.x - RectMins.x);
    float norm_y = (PosPass.y - RectMins.y) / (RectMaxs.y - RectMins.y);
    float norm_z = (PosPass.z - RectMins.z) / (RectMaxs.z - RectMins.z);

    float line_w = 0.1;
    float line_x = line_w / (RectMaxs.x - RectMins.x);
    float line_y = line_w / (RectMaxs.y - RectMins.y);
    float line_z = line_w / (RectMaxs.z - RectMins.z);

    if(norm_x > 1-line_x || norm_x < line_x || norm_y > 1-line_y || norm_y < line_y || norm_z > 1-line_z || norm_z < line_z )
        gl_FragColor.w = 1.0f;
}