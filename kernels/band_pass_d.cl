////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Apply a band pass filter
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Applies a band pass filter to an image. Currently assumes the image (or FFT) is zero centered.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// output - empty buffer to be filled with the band pass output
/// input - buffer containing the image to apply hte band pass to
/// width - width of output
/// height - height of output
/// inner - minimum radius to allow through (pixels)
/// outer - maximum radius to allow through (pixels)
/// x_centre - centre x shift of the band pass ring
/// y_centre - centre y shift of the band pass ring
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void band_pass_d( __global const double* restrict input,
                               __global double* restrict output,
							   unsigned int width,
							   unsigned int height,
							   double inner,
							   double outer,
							   double x_centre,
							   double y_centre)
{
	//Get the work items ID
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	if (xid < width && yid < height) {
		int id = xid + yid * width;
    	double centX = width / 2.0 + x_centre;
    	double centY = height / 2.0 + y_centre;
    	double radius = native_sqrt( (xid-centX) * (xid-centX) + (yid-centY) * (yid-centY) );
    	if (radius < outer && radius > inner) {
		    output[id] = input[id];
		} else {
		    output[id] = 0.0;
		}
	}
}