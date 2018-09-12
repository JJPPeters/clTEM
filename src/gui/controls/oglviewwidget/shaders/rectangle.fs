#version 150

uniform vec4 RectCol;
uniform vec4 RectLims;
in vec4 PosPass;

void main() {
    gl_FragColor = RectCol;

    float norm_x = (PosPass.x - RectLims.x) / (RectLims.z - RectLims.x);
    float norm_y = (PosPass.y - RectLims.y) / (RectLims.w - RectLims.y);

    float line_w = 0.1;
    float line_x = line_w / (RectLims.z - RectLims.x);
    float line_y = line_w / (RectLims.w - RectLims.y);


    if(norm_x > 1-line_x || norm_x < line_x || norm_y > 1-line_y || norm_y < line_y)
    {
        gl_FragColor.w = 1.0f;
    }
}