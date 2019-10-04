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
double cModSq(double2 a)
{
	return (a.x*a.x + a.y*a.y);
}
double2 cMult(double2 a, double2 b)
{
	return (double2)(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}
double2 cConj(double2 a)
{
	return (double2)(a.x, -a.y);
}
double2 cPow(double2 a, int n)
{
	double2 temp = a;
	for (int j=1; j < n; j++)
	{
		temp = cMult(temp, a);
	}
	return temp;
}
__kernel void ctem_image_d( __global const double2* input,
							   __global double2* output,
							   unsigned int width,
							   unsigned int height,
							   __global double* k_x,
							   __global double* k_y,
						   	   double wavelength,
							   double C10, double2 C12,
							   double2 C21, double2 C23,
							   double C30, double2 C32, double2 C34,
							   double2 C41, double2 C43, double2 C45,
							   double C50, double2 C52, double2 C54, double2 C56,
							   double obj_ap,
							   double beta,
							   double delta)
{
	//Get the work items ID
    int xid = get_global_id(0);
    int yid = get_global_id(1);
	if(xid < width && yid < height)
	{
		int id = xid + yid*width;
		double obj_ap2 = (obj_ap * 0.001) / wavelength;
		double beta2 = (beta * 0.001) / wavelength;
		double k = native_sqrt((k_x[xid]*k_x[xid]) + (k_y[yid]*k_y[yid]));
		if (k < obj_ap2)
		{
			double2 w = (double2)(wavelength*k_x[xid], wavelength*k_y[yid]);
			double2 wc = cConj(w);

			// TODO: check the 0.25 factor here is correct (it was 0.5, but Kirkland 2nd ed. eq. 3.42 disagrees)
			double temporalCoh = native_exp( -0.25 * M_PI*M_PI  * delta*delta * cModSq(w)*cModSq(w) / (wavelength*wavelength) );
			double spatialCoh = native_exp( -1.0 * M_PI*M_PI * beta2*beta2 * cModSq(w) * pow((C10 + C30*cModSq(w) + C50*cModSq(w)*cModSq(w)), 2)  / (wavelength*wavelength) );
			double tC10 = 0.5 * C10 * cModSq(w);
			double2 tC12 = 0.5 * cMult(C12, cPow(wc, 2));
			double2 tC21 = cMult(C21, cMult(cPow(wc, 2), w)) / 3.0;
			double2 tC23 = cMult(C23, cPow(wc, 3)) / 3.0;
			double tC30 = 0.25 * C30 * cModSq(w)*cModSq(w);
			double2 tC32 = 0.25 * cMult(C32, cMult(cPow(wc, 3), w));
			double2 tC34 = 0.25 * cMult(C34, cPow(wc, 4));

			double2 tC41 = 0.2 * cMult(C41, cMult(cPow(wc, 3), cPow(w ,2)));
			double2 tC43 = 0.2 * cMult(C43, cMult(cPow(wc, 4), w));
			double2 tC45 = 0.2 * cMult(C45, cPow(wc, 5));
			double tC50 = C50 * cModSq(w)*cModSq(w)*cModSq(w) / 6.0;
			double2 tC52 = cMult(C52, cMult(cPow(wc, 4), cPow(w ,2))) / 6.0;
			double2 tC54 = cMult(C54, cMult(cPow(wc, 5), w)) / 6.0;
			double2 tC56 = cMult(C56, cPow(wc, 6)) / 6.0;

			double cchi = tC10 + tC12.x + tC21.x + tC23.x + tC30 + tC32.x + tC34.x + tC41.x + tC43.x + tC45.x + tC50 + tC52.x + tC54.x + tC56.x;
			double chi = 2.0 * M_PI * cchi / wavelength;
			output[id].x = temporalCoh * spatialCoh * ( input[id].x * cos(chi) + input[id].y * sin(chi) );
			output[id].y = temporalCoh * spatialCoh * ( input[id].y * cos(chi) - input[id].x * sin(chi) );
		}
		else
		{
			output[id].x = 0.0;
			output[id].y = 0.0;
		}
	}
}