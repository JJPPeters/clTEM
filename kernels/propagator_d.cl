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
__kernel void propogator_d( __global double2* propagator,
									__global double* k_x,
									__global double* k_y,
									unsigned int width,
									unsigned int height,
									double dz,
									double wavelength,
									double k_max)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);

	if(xid < width && yid < height) {
		int id = xid + width * yid;
		double k0x = k_x[xid];
		double k0y = k_y[yid];

		k0x *= k0x;
		k0y *= k0y;

		propagator[id].x = cos(M_PI * dz * wavelength * (k0x + k0y));
		propagator[id].y = -1.0 * sin(M_PI * dz * wavelength * (k0x + k0y));
	}
}

