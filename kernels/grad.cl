////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Laplacian intermediate step kernel
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Part of the process for applying the Laplacian operator to a wave function (used in the finite difference method). 
/// The wave function must first be Fourier transformed, this kernel applied and then inverse Fourier transformed. More
/// details can be found in Kirkland's "Advanced computing in electron microscopy 2nd ed." equation 6.118. Note that the
/// input and the frequency data must have the same 0 frequency position (i.e. both or neither are FFT shifted).
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// input_output - FFT of wavefeunction to apply laplacian to (applied in place)
/// k_x - k values for x axis of input_output (size needs to equal width)
/// k_y - k values for y axis of input_output (size needs to equal height)
/// width - width of input_output
/// height - height of input_output
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void clGrad( __global float2* input_output,
					  __global float* restrict k_x,
					  __global float* restrict k_y,
					  int width,
					  int height)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	if(xid < width && yid < height)
	{
		int Index = xid + width * yid;
		input_output[Index] *= -4.0f * M_PI_F * M_PI_F * (k_x[xid]*k_x[xid] + k_y[yid]*k_y[yid]);
	}
}