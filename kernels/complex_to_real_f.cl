////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Converts a complex array to a real one
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Takes a complex input and converts to real using the methods selected
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// input - image to be translated
/// output - output translated image
/// method - how the complex number of converted (0 = real, 1 = imaginary, 2 = magnitude, 3 = phase, 4 = square abs)
/// width - width of the input/output
/// height - height of the output/output
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void complex_to_real_f( __global float2* input,
								 __global float* output,
								 int method,
								 unsigned int width,
								 unsigned int height)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
    int id = xid + width * yid;

    if (xid >= 0 && xid < width && yid >= 0 && yid < height) {

        if (method == 0)
            output[id] = input[id].x;
        else if (method == 1)
            output[id] = input[id].y;
        else if (method == 2)
            output[id] = native_sqrt(input[id].x * input[id].x + input[id].y * input[id].y);
        else if (method == 3)
            output[id] = atan2(input[id].y, input[id].x);
        else if (method == 4)
            output[id] = input[id].x * input[id].x + input[id].y * input[id].y;
        else
            output[id] = 0.0f;
    }
}