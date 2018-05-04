////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Apply the DQE to the simulated image
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Takes an input ctem image and apllies the detector quantum efficiency described by the dqe input and with any
/// binning. See doi 10.1016/j.jsb.2013.05.008 for more details (Eq. 7 in particular)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// input - image to apply the DQE to
/// dqe - array describing the dqe
/// width - width of the inputs
/// height - height of the outputs
/// binning - the binning of the CCD
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void clDQE(__global const float2* input,
					__global float* dqe_data,
					int width,
					int height,
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
		int dqe = floor(rad/binning);
		int dqe2 = dqe+1;
		float dqeval = dqe_data[min(dqe,724)];
		float dqeval2 = dqe_data[min(dqe2,724)];
		float interp = rad/binning - floor(rad/binning);
		float finaldqe = interp * dqeval2 + (1-interp)*dqeval;
		float real = input[Index].x;
		float imag = input[Index].y;
		input[Index].x = real*sqrt(finaldqe);
		input[Index].y = imag*sqrt(finaldqe);
	}
}
