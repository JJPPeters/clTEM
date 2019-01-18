////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Apply a band pass filter
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Applies a band pass filter to an image. Currently assumes the image (or FFT) is zero centered.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// output - empty buffer to be filled with the band pass output
/// input - buffer containing the image to apply hte band pass to
/// width - width of output
/// height - height of output
/// inner - minimum radius to allow through (pixels)
/// outer - maximum radius to allow through (pixels)
/// x_centre - centre x shift of the band pass ring
/// y_centre - centre y shift of the band pass ring
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void clFloatBandPass( __global const float2* restrict input,
                               __global float* restrict output,
							   unsigned int width,
							   unsigned int height,
							   float inner,
							   float outer,
							   float x_centre,
							   float y_centre)
{
	//Get the work items ID
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	if(xid<width && yid<height) {
		int Index = xid + yid * width;
    	float centX = width / 2.0f + x_centre;
    	float centY = height / 2.0f + y_centre;
    	float radius = native_sqrt( (xid-centX) * (xid-centX) + (yid-centY) * (yid-centY) );
    	if (radius < outer && radius > inner) {
		    output[Index] = input[Index].x*input[Index].x + input[Index].y*input[Index].y;
		} else {
		    output[Index] = 0.0f;
		}
	}
}