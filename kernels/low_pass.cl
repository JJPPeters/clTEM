////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Apply a low pass filter
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Applies a low pass filter to an image. Uses the k vector values defined by k_x and k_y to define the zero value.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// input_output - buffer to apply low pass to (applied in place)
/// width - width of input_output
/// height - height of input_output
/// k_max - maximum k value to allow through the filter
/// k_x - k values for x axis of input_output (size needs to equal width)
/// k_y - k values for y axis of input_output (size needs to equal height)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void clBandLimit( __global float2* input_output, 
						   unsigned int width,
						   unsigned int height,
						   float k_max,
						   __global float* k_x,
						   __global float* k_y)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	if(xid < width && yid < height)
	{
		int Index = xid + width*yid;
		float k = sqrt( k_x[xid]*k_x[xid] + k_y[yid]*k_y[yid] );
		input_output[Index].x *= (k<=k_max);
		input_output[Index].y *= (k<=k_max);
	}
}