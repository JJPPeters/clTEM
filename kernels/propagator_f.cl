////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Generate the multislice propagator
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Generates the propagator function for the multislice steps. This is taken from Kirkland's "Advanced computing in 
/// electron microscopy 2nd ed." equation 6.100 (or derivation of 6.71). Small (< 1 degree) specimen tilt can be 
/// included here if desired, but I beleive these are best simulated by tilting the physical structure. This propagator
/// is calculated once before the actual multislice steps are computed.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// propagator - the output propagator function
/// k_x - k values for x axis of propagator (size needs to equal width)
/// k_y - k values for y axis of propagator (size needs to equal height)
/// width - width of propagator
/// height - height of propagator
/// dz - the thickness of the slices
/// wavelength - wavelength of the electron beam (units...)
/// k_max - maximum k value to allow through the low pass filter
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void propagator_f( __global float2* propagator,
                            __global float* k_x,
                            __global float* k_y,
                            unsigned int width,
                            unsigned int height,
                            float dz,
                            float beam_k,
                            float beam_k_x,
                            float beam_k_y,
                            float beam_k_z,
                            float k_max)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);

	if(xid < width && yid < height) {
		int id = xid + width * yid;
		float k0x = k_x[xid] * k_x[xid];
		float k0y = k_y[yid] * k_y[yid];

        double k_max_2 = k_max * k_max;

        if (k0x+k0y < k_max_2) {
		    float f = beam_k_z / beam_k;
		    float s_u = beam_k*beam_k - beam_k_z*beam_k_z - (beam_k_x + k_x[xid])*(beam_k_x + k_x[xid]) - (beam_k_y + k_y[yid])*(beam_k_y + k_y[yid]);
		    s_u = s_u / (beam_k_z);
		    propagator[id].x = native_cos(M_PI_F * s_u * dz);
		    propagator[id].y = native_sin(M_PI_F * s_u * dz);
		} else {
		    propagator[id].x = 0.0f;
		    propagator[id].y = 0.0f;
		}
	}
}

