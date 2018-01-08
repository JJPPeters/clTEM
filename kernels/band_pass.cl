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
__kernel void clFloatBandPass( __global float* restrict output,
							   __global const float* restrict input,
							   int width,
							   int height,
							   float inner,
							   float outer,
							   float x_centre,
							   float y_centre)
{
	//Get the work items ID
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	if(xid<width && yid<height)
	{
		int Index = xid + yid*width;
    	float centX = width/2 + x_centre;
    	float centY = height/2 + y_centre;
    	float radius = hypot(xid-centX,yid-centY);
		output[Index] = (radius < outer && radius > inner) * input[Index];
	}
}