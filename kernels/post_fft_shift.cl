////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// FFT shift an image
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Centres the zero frequency of a Fourier transformed image. This is the 'simple' method that simply shifts the 
/// quandrants around and can be applied after the Fourier transform has been completed.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// input - image to be FFT shifted
/// output - output FFT shifted image
/// width - width of input
/// height - height of input
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void clfftShift( __global const float2* input,
                          __global float2* output,
                          int width,
                          int height)
{
    //Get the work items ID
    int xid = get_global_id(0);
    int yid = get_global_id(1);
    if(xid < width && yid < height)
    {
        int Index = xid + yid * width;
        int Yshift = width * height / 2;
        int Xshift = width / 2;
        int Xmid = width / 2;
        int Ymid = height / 2;
        if( xid < Xmid && yid < Ymid )
        {
            output[Index+Yshift+Xshift].x = input[Index].x;
            output[Index+Yshift+Xshift].y = input[Index].y;
        }
        else if( xid >= Xmid && yid < Ymid )
        {
            output[Index+Yshift-Xshift].x = input[Index].x;
            output[Index+Yshift-Xshift].y = input[Index].y;
        }
        else if( xid < Xmid && yid >= Ymid )
        {
            output[Index-Yshift+Xshift].x = input[Index].x;
            output[Index-Yshift+Xshift].y = input[Index].y;
        }
        else if( xid >= Xmid && yid >= Ymid )
        {
            output[Index-Yshift-Xshift].x = input[Index].x;
            output[Index-Yshift-Xshift].y = input[Index].y;
        }
    }
}