////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Multiply two complex images
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Multiples two complex images element wise
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// input_a - first image to be multiplied
/// input_b - second image to be multiplied
/// output - output of multiplication
/// width - width of hte inputs
/// height - height of the outputs
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void clComplexMultiply( __global float2* input_a,
								 __global float2* input_b,
								 __global float2* output, 
								 unsigned int width,
								 unsigned int height)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	if(xid < width && yid < height) {
		int Index = xid + width*yid;
		output[Index].x = input_a[Index].x * input_b[Index].x - input_a[Index].y * input_b[Index].y;
		output[Index].y = input_a[Index].x * input_b[Index].y + input_a[Index].y * input_b[Index].x;
	}
}