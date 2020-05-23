////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Generates a probe wave function
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Generates a probe wave function for use in CBED and STEM simulations using up to 5th order aberrations. Uses multiple
/// helper functions to simplify the maths parts. The probe is generated in reciprocal space to then be Fourier 
/// transformed to real space. This is described in Kirkland's "Advanced computing in electron microscopy 2nd ed." by
/// equation 5.47.
/// TODO: can this include spatial/temporal coherence terms (similar to CTEM). Look at Eq. 3.74 and 3.75 in Kirkland's
/// book
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// output - empty buffer to be filled with the generated wave function
/// width - width of output
/// height - height of output
/// k_x - k values for x axis of output (size needs to equal width)
/// k_y - k values for y axis of output (size needs to equal height)
/// pos_x - x position of the generated probe (in pixels)
/// pos_y - y position of the generate probe (in pixels)
/// pixel_scale - real space pixel scale // TODO: can this be replaced with just the raw position values
/// wavelength - wavelength of the electron beam (units?)
/// C10 - C56 - aberration coefficients (units?)
/// cond_ap - size/convergence of the condenser aperture (units?)
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
__kernel void init_probe_wave_d( __global double2* output,
											unsigned int width,
											unsigned int height,
											__global const double* k_x,
											__global const double* k_y,
											double pos_x,
											double pos_y,
											double pixel_scale,
											double wavelength,
											double C10, double2 C12,
											double2 C21, double2 C23,
											double C30, double2 C32, double2 C34,
											double2 C41, double2 C43, double2 C45,
											double C50, double2 C52, double2 C54, double2 C56,
											double cond_ap,
											double ap_smooth)
{
	// Get the work items ID
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	if(xid < width && yid < height)
	{
		int id = xid + yid*width;
		double cond_ap2 = (cond_ap * 0.001) / wavelength;
        double k = native_sqrt( (k_x[xid]*k_x[xid]) + (k_y[yid]*k_y[yid]) );

        double ap_smooth_radius = (ap_smooth * 0.001) / wavelength; // radius in mrad

		if (k < cond_ap2 + ap_smooth_radius)
		{
			// this term is easier to calculate once before it is put into the exponential
			double posTerm = 2.0 * M_PI * (k_x[xid]*pos_x*pixel_scale + k_y[yid]*pos_y*pixel_scale);
			double2 w = (double2)(wavelength*k_x[xid], wavelength*k_y[yid]);
			double2 wc = cConj(w);
			// all the aberration terms, calculated in w (omega)
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

            // note because of the conjugates we only have real components left
			double cchi = tC10 + tC12.x + tC21.x + tC23.x + tC30 + tC32.x + tC34.x + tC41.x + tC43.x + tC45.x + tC50 + tC52.x + tC54.x + tC56.x;
			double chi = 2.0 * M_PI * cchi / wavelength;

            // smooth the aperture edge
            double edge_factor = 1.0f;

            if (fabs(k-cond_ap2) < ap_smooth_radius)
                edge_factor = 1.0f - smoothstep(cond_ap2 - ap_smooth_radius, cond_ap2 + ap_smooth_radius, k);

			output[id].x = native_cos(posTerm - chi);
            output[id].y = native_sin(posTerm - chi);
		}
		else
		{
			output[id].x = 0.0;
			output[id].y = 0.0;
		}
	}
}
