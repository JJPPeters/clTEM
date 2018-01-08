////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Modify image so it's FFT will already be shifted
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Centres the zero frequency of a Fourier transformed image by modifying images before it has been transformed. This 
/// is achieved by multiplying every pixel by -1 to the power of the pixel index.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// input - image to be pre-FFT shifted
/// output - output pre FFT shifted image
/// width - width of input
/// height - height of input
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void clPreFftShift( __global const float2* input
                             __global float2* output,
                             int width,
                             int height)
{
    //Get the work items ID
    int xid = get_global_id(0);
    int yid = get_global_id(1);
    if(xid < width && yid < height)
    {
    	int Index = xid + yid*width;
    	output[Index] = pown(-1, xid+yid) * input[Index];
    }
}