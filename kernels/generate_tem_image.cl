////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Generates a CTEM image from the exit wave function
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// After a full CTEM simulation, the result is still not an image as you would see it, instead it is the exit wave 
/// function without any aberrations. This function calculates this image from the exit wave function. This is best
/// summarised in Kirkland's "Advanced computing in electron microscopy 2nd ed." in section 3.2 (for the coherence 
/// terms), particularly equation 3.42. The initial transfer function is discussed around equation 3.10. Note that this 
/// uses the complex omega (w) instead of the k vectors, it's a simple conversion w = wavelength*(k_x + i*k_y)
/// TODO: Kirkland has an epsilon term with partial coherence that is just pi * beta * delta (it is typically small, but
/// it is so easy to include anyway.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// input - the exit wave function from a simulation
/// output - empty buffer to be filled with the generated image
/// width - width of input/output
/// height - height of input/output
/// k_x - k values for x axis of output (size needs to equal width)
/// k_y - k values for y axis of output (size needs to equal height)
/// wavelength - wavelength of the electron beam (units?)
/// C10 - C56 - aberration coefficients (units?)
/// obj_ap - size/convergence of the objective aperture (units?, semiangle?)
/// beta - size/convergence of the condenser aperture semiangle (units)
/// delta - the defocus spread (a term incorporating the chromatic aberrations, see Kirkland 2nd ed., equation 3.41)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// cModSq - takes the square modulus of a complex number
/// cMult - multiply two complex numbers
/// cConj - take the conjugate of a complex number
/// cPow - calculate the value of a complex number to the power of n
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float cModSq(float2 a)
{
	return (a.x*a.x + a.y*a.y);
}
float2 cMult(float2 a, float2 b)
{
	return (float2)(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}
float2 cConj(float2 a)
{
	return (float2)(a.x, -a.y);
}
float2 cPow(float2 a, int n)
{
	float2 temp = a;
	for (int j=1; j < n; j++)
	{
		temp = cMult(temp, a);
	}
	return temp;
}
__kernel void clImagingKernel( __global const float2* input,
							   __global float2* output,
							   int width,
							   int height,
							   __global float* k_x,
							   __global float* k_y,
						   	   float wavelength,
							   float C10, float2 C12,
							   float2 C21, float2 C23,
							   float C30, float2 C32, float2 C34,
							   float2 C41, float2 C43, float2 C45,
							   float C50, float2 C52, float2 C54, float2 C56,
							   float obj_ap,
							   float beta,
							   float delta)
{
	//Get the work items ID
    int xid = get_global_id(0);
    int yid = get_global_id(1);
	if(xid < width && yid < height)
	{
		int Index = xid + yid*width;
		float obj_ap2 = (obj_ap * 0.001f) / wavelength;
		float beta2 = (beta * 0.001f) / wavelength;
		float k = sqrt((k_x[xid]*k_x[xid]) + (k_y[yid]*k_y[yid]));
		if (k < obj_ap2)
		{
			float2 w = (float2)(wavelength*k_x[xid], wavelength*k_y[yid]);
			float2 wc = cConj(w);

			// TODO: check the 0.25 factor here is correct (it was 0.5, but Kirkland 2nd ed. eq. 3.42 disagrees)
			float temporalCoh = exp( -0.25f * M_PI_F*M_PI_F  * delta*delta * cModSq(w)*cModSq(w) / (wavelength*wavelength) );
			float spatialCoh = exp( -1.0f * M_PI_F*M_PI_F * beta2*beta2 * cModSq(w) * pow((C10 + C30*cModSq(w) + C50*cModSq(w)*cModSq(w)), 2)  / (wavelength*wavelength) );
			float tC10 = 0.5f * C10 * cModSq(w);
			float2 tC12 = 0.5f * cMult(C12, cPow(wc, 2));
			float2 tC21 = cMult(C21, cMult(cPow(wc, 2), w)) / 3.0f;
			float2 tC23 = cMult(C23, cPow(wc, 3)) / 3.0f;
			float tC30 = 0.25f * C30 * cModSq(w)*cModSq(w);
			float2 tC32 = 0.25f * cMult(C32, cMult(cPow(wc, 3), w));
			float2 tC34 = 0.25f * cMult(C34, cPow(wc, 4));

			float2 tC41 = 0.2f * cMult(C41, cMult(cPow(wc, 3), cPow(w ,2)));
			float2 tC43 = 0.2f * cMult(C43, cMult(cPow(wc, 4), w));
			float2 tC45 = 0.2f * cMult(C45, cPow(wc, 5));
			float tC50 = C50 * cModSq(w)*cModSq(w)*cModSq(w) / 6.0f;
			float2 tC52 = cMult(C52, cMult(cPow(wc, 4), cPow(w ,2))) / 6.0f;
			float2 tC54 = cMult(C54, cMult(cPow(wc, 5), w)) / 6.0f;
			float2 tC56 = cMult(C56, cPow(wc, 6)) / 6.0f;

			float cchi = tC10 + tC12.x + tC21.x + tC23.x + tC30 + tC32.x + tC34.x + tC41.x + tC43.x + tC45.x + tC50 + tC52.x + tC54.x + tC56.x;
			float chi = 2.0f * M_PI_F * cchi / wavelength;
			output[Index].x = temporalCoh * spatialCoh * ( input[Index].x * cos(chi) + input[Index].y * sin(chi) );
			output[Index].y = temporalCoh * spatialCoh * ( input[Index].y * cos(chi) - input[Index].x * sin(chi) );
		}
		else
		{
			output[Index].x = 0.0f;
			output[Index].y = 0.0f;
		}
	}
}