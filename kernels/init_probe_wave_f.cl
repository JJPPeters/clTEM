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
__kernel void init_probe_wave_f( __global float2* output,
											unsigned int width,
											unsigned int height,
											__global const float* k_x,
											__global const float* k_y,
											float pos_x,
											float pos_y,
											float pixel_scale,
											float wavelength,
											float C10, float2 C12,
											float2 C21, float2 C23,
											float C30, float2 C32, float2 C34,
											float2 C41, float2 C43, float2 C45,
											float C50, float2 C52, float2 C54, float2 C56,
											float cond_ap,
											float ap_smooth)
{
	// Get the work items ID
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	if(xid < width && yid < height)
	{
		int id = xid + yid*width;
		float cond_ap2 = (cond_ap * 0.001f) / wavelength;
        float k = native_sqrt( (k_x[xid]*k_x[xid]) + (k_y[yid]*k_y[yid]) );

        float ap_smooth_radius = (ap_smooth * 0.001f) / wavelength; // radius in mrad

		if (k < cond_ap2 + ap_smooth_radius)
		{
			// this term is easier to calculate once before it is put into the exponential
			float posTerm = 2.0f * M_PI_F * (k_x[xid]*pos_x*pixel_scale + k_y[yid]*pos_y*pixel_scale);
			float2 w = (float2)(wavelength*k_x[xid], wavelength*k_y[yid]);
			float2 wc = cConj(w);
			// all the aberration terms, calculated in w (omega)
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

            // smooth the aperture edge
            float edge_factor = 1.0f;

            if (fabs(k-cond_ap2) < ap_smooth_radius)
                edge_factor = 1.0f - smoothstep(cond_ap2 - ap_smooth_radius, cond_ap2 + ap_smooth_radius, k);

			output[id].x = native_cos(posTerm - chi) * edge_factor;
            output[id].y = native_sin(posTerm - chi) * edge_factor;
		}
		else
		{
			output[id].x = 0.0f;
			output[id].y = 0.0f;
		}
	}
}
