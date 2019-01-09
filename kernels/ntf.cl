////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Apply the NTF to the simulated image
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Takes an input ctem image and apllies the noise transfer fucntion described by the ntf input and with any
/// binning. See doi 10.1016/j.jsb.2013.05.008 for more details (Eq. 7 in particular)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// input - image to apply the DQE to
/// ntf - array describing the ntf
/// width - width of the inputs
/// height - height of the outputs
/// binning - the binning of the CCD
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void clNTF(__global const float2* input,
					__global float* ntf_data,
					unsigned int width,
					unsigned int height,
					int binning)
{
	//Get the work items ID
	int xid = get_global_id(0);
	int yid = get_global_id(1);

	if(xid < width && yid < height)
	{
		int Index = xid + yid*width;
		int midx;
		int midy;

		if(xid < width/2)
			midx=0;
		else
			midx=width;
		if(yid < height/2)
			midy=0;
		else
			midy=height;
			
		float xp = xid - midx;
		float yp = yid - midy;
		float rad = sqrt(xp*xp + yp*yp);
		int ntf = floor(rad/binning);
		int ntf2 = ntf+1;
		float ntfval = ntf_data[min(ntf,724)];
		float ntfval2 = ntf_data[min(ntf2,724)];
		float interp = rad/binning - floor(rad/binning);
		float finalntf = interp * ntfval2 + (1-interp)*ntfval;
		float real = input[Index].x;
		float imag = input[Index].y;
		input[Index].x = real*finalntf;
		input[Index].y = imag*finalntf;
	}
}
