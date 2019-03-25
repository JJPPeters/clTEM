////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Generates the crystal potential (using the conventional technique, no accounting for z)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This is really where the magic happens
/// However, I feel it is not well optimised, or at least not well organised
///
/// This kernel calculates the actual potential for each slice, which the wave function is then propagated through. Most
/// of the parameterisation stuff can be found in Kirkland's "Advanced computing in electron microscopy 2nd ed." in
/// appendix C (also where the parameters are given). The rest of this function is loading the atoms, calculating
/// whether they are relevant and maybe more.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// potential - the output potential image for the slice
/// pos_x - x position of the atoms
/// pos_y - y position of the atoms
/// pos_z - z position of the atoms
/// atomic_num - atomic number of the atoms
/// params - perameterised form of the scattering factors
/// block_start_pos - the start positions (real space) of each block
/// width - width of the output potential
/// height - height of the output potential
/// current_slice - current slice of the simulation
/// total_slices - total number of slices in the simulation
/// z - current z position
/// dz - the slice thickness
/// pixelscale - pixel scale of the image in real space
/// blocks_x - total number of blocks in x direction
/// blocks_y - total number of blocks in y direction
/// max_x - max x position (including padding)
/// min_x - min x position (including padding)
/// max_y - max y position (including padding)
/// min_y - min y position (including padding)
/// block_load_x - blocks to load in x direction
/// block_load_y - blocks to load in y direction
/// slice_load_z - blocks to load in z direction
/// sigma - the interaction parameter (given by eq. 5.6 in Kirkland)
/// startx - x start position of simulation (when simulation is cropped)
/// starty - y start position of simulation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Bessel functions (Used the the projected potential calculations
/// These function's can be found in "Numerical recipes in C, 2nd ed." Chapter 6.6.
/// bessi0 - calculate the zero order modified bessel function of the first kind (using only for bessk0)
/// bessk0 - calculate the zero order modified bessel function of the second kind
/// bessi1 - calculate the zero order modified bessel function of the first kind (using only for bessk0)
/// bessk1 - calculate the zero order modified bessel function of the second kind
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Projected potential functions
/// The lobato paper (10.1107/S205327331401643X) gives a good overview of these parameters. Kirkland's book 2nd ed. has
/// a useful table too (Table C.1) to see a list of other parameterisations.
/// kirkland - See equation C.20 from Kirkland's book (2nd ed.). Parameters are stored as:
/// a1, b1, a2, b2, a3, b3, c1, d1, c2, d2, c3, d3
/// lobato - See equation 16 from their paper (10.1107/S205327331401643X). Parameter are stored as:
/// a1, a2, a3, a4, a5, b1, b2, bb, b4, b5
/// peng - See equation 47 from the lobato paper (this needs to be integrated for the projected potential. Reference
/// 10.1107/S0108767395014371. Parameters are stored as: a1, a2, a3, a4, a5, b1, b2, bb, b4, b5
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double bessi0(double x) {
	int i;
	double ax, x2, sum;
	double i0a[] = {1.0, 3.5156229, 3.0899424, 1.2067492, 0.2659732, 0.0360768, 0.0045813};
	double i0b[] = {0.39894228, 0.01328592, 0.00225319, -0.00157565, 0.00916281, -0.02057706, 0.02635537, -0.01647633, 0.00392377};

	ax = fabs(x);

	if(ax <= 3.75) {
		x2 = x / 3.75;
		x2 = x2 * x2;
		sum = i0a[6];
		for(i = 5; i >= 0; --i)
			sum = sum*x2 + i0a[i];
	} else {
		x2 = 3.75 / ax;
		sum = i0b[8];
		for(i=7; i>=0; --i)
			sum = sum*x2 + i0b[i];
		sum = native_exp(ax) * sum * native_rsqrt(ax);
	}

	return sum;
}

double bessi1(double x) {
    int i;
    double ax, x2, sum;
    double i1a[] = {0.5, 0.87890594, 0.51498869, 0.15084934, 0.02658733, 0.00301532, 0.00032411};
    double i1b[] = {0.39894228, -0.03988024, -0.00362018, 0.00163801, -0.01031555, 0.02282967, -0.02895312, 0.01787654, 0.00420059};

    ax = fabs(x);

    if(ax <= 3.75) {
        x2 = x / 3.75;
        x2 = x2 * x2;
        sum = i1a[6];
        for(i = 5; i >= 0; --i)
            sum = sum*x2 + i1a[i];
        sum *= ax;
    } else {
        x2 = 3.75 / ax;
        sum = i1b[8];
        for(i = 2; i >= 0; --i)
            sum = sum*x2 + i1b[i];
        sum = native_exp(ax) * sum * native_rsqrt(ax);
    }

    return sum;
}

double bessk0(double x) {
	int i;
	double ax, x2, sum;
	double k0a[] = {-0.57721566, 0.42278420, 0.23069756, 0.03488590, 0.00262698, 0.00010750, 0.00000740};
	double k0b[] = {1.25331414, -0.07832358, 0.02189568, -0.01062446, 0.00587872, -0.00251540, 0.00053208};

	ax = fabs(x);

	if((ax > 0.0)  && (ax <=  2.0)) {
		x2 = ax/2.0;
		x2 = x2 * x2;
		sum = k0a[6];
		for(i = 5; i >= 0; --i)
			sum = sum*x2 + k0a[i];
		sum = -log(ax/2.0) * bessi0(x) + sum;
	} else if(ax > 2.0) {
		x2 = 2.0/ax;
		sum = k0b[6];
		for(i=5; i>=0; --i)
			sum = sum*x2 + k0b[i];
		sum = native_exp(-ax) * sum * native_rsqrt(ax);
	} else
		sum = FLT_MAX;

	return sum;
}

double bessk1(double x) {
    int i;
    double ax, x2, sum;
    double k1a[] = {1.0, 0.15443144, -0.67278579, -0.18156897, -0.01919402, -0.00110404, -0.00004686};
    double k1b[] = {1.25331414, 0.23498619, -0.03655620, 0.01504268, -0.00780353, 0.00325614, -0.00068245};

    ax = fabs(x);

    if((ax > 0.0)  && (ax <=  2.0)) {
        x2 = ax/2.0;
        x2 = x2 * x2;
        sum = k1a[6];
        for(i = 5; i >= 0; --i)
            sum = sum*x2 + k1a[i];
        sum = log(ax/2.0) * bessi1(x) + sum / ax;
    } else if( ax > 2.0 ) {
        x2 = 2.0/ax;
        sum = k1b[6];
        for(i=5; i>=0; --i)
            sum = sum*x2 + k1b[i];
        sum = native_exp(-ax) * sum * native_rsqrt(ax);
    } else
        sum = FLT_MAX;

    return sum;
}

double kirkland(__constant double* params, int ZNum, double rad) {
    int i;
    double suml, sumg, x;
    suml = 0.0;
    sumg = 0.0;
    //
    // Lorentzians
    //
    x = 2.0 * M_PI * rad;

    // Loop through our parameters (a and b)
    for(i=0; i<6; i+=2) {
        double a = params[(ZNum-1)*12+i];
        double b = params[(ZNum-1)*12+i+1];
        suml += a * bessk0( x*native_sqrt(b) );
    }

    //
    // Gaussians
    //
    x = M_PI * rad;
    x = x * x;

    // Loop through our parameters (a and b)
    for(i=6; i<12; i+=2) {
        double c = params[(ZNum-1)*12+i];
        double d = params[(ZNum-1)*12+i+1];
        double d_inv = native_recip(d);
        sumg += (c * d_inv) * native_exp(-x * d_inv);
    }

    // The funny doubles are from the remaining constants in equation C.20
    // Not that they use the fundamental charge as 14.4 Volt-Angstroms
    return 300.8242834 * suml + 150.4121417 * sumg;
 }

double lobato(__constant double* params, int ZNum, double rad) {
    int i;
    double sum, x;
    sum = 0.0;

    x = 2.0 * M_PI * rad;

    for(i=0; i < 5; ++i) {
        double a = params[(ZNum-1)*10+i];
        double b = params[(ZNum-1)*10+i+5];
        double b_inv_root = native_rsqrt(b);
        sum += a * (b_inv_root * b_inv_root * b_inv_root) * (bessk0(x * b_inv_root) + rad * bessk1(x * b_inv_root));
    }

    return 945.090144399935 * sum;
}

double peng(__constant double* params, int ZNum, double rad) {
    int i;
    double sum, x;
    sum = 0.0;

    x = M_PI * rad;
    x = x * x;

    for(i=0; i<5; ++i) {
        double a = params[(ZNum-1)*10+i];
        double b = params[(ZNum-1)*10+i+5];

        sum += a * native_sqrt(b) * native_exp(-x * native_recip(b));
    }

    return 1040.79479708354 * sum;
}

__kernel void potential_projected_d( __global double2* potential,
											       __global const double* restrict pos_x,
										  		   __global const double* restrict pos_y,
										 		   __global const double* restrict pos_z,
												   __global const int* restrict atomic_num,
												   __constant double* params,
												   unsigned int param_selector,
										 		   __global const int* restrict block_start_pos,
												   unsigned int width,
												   unsigned int height,
												   int current_slice,
												   int total_slices,
												   double z,
												   double dz,
												   double pixelscale, 
												   int blocks_x,
												   int blocks_y,
												   double max_x,
												   double min_x,
												   double max_y,
												   double min_y,
												   int block_load_x,
												   int block_load_y,
												   int slice_load_z,
												   double sigma,
										  		   double startx,
												   double starty)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	int lid = get_local_id(0) + get_local_size(0)*get_local_id(1);
	int id = xid + width * yid;
	int topz = current_slice;
	int bottomz = current_slice;
	double sumz = 0.0;
	int gx = get_group_id(0);
	int gy = get_group_id(1);

	if(topz < 0 )
		topz = 0;
	if(bottomz >= total_slices )
		bottomz = total_slices-1;

	__local double atx[256];
	__local double aty[256];
	__local int atZ[256];

	// calculate the indices of the bins we will need?
	int startj = fmax(floor( (starty - min_y+  gy *    get_local_size(1) * pixelscale) * blocks_y  / (max_y-min_y)) - block_load_y, 0) ;
	int endj =   fmin( ceil( (starty - min_y + (gy+1) * get_local_size(1) * pixelscale) * blocks_y  / (max_y-min_y)) + block_load_y, blocks_y-1);
	int starti = fmax(floor( (startx - min_x +  gx *    get_local_size(0) * pixelscale) * blocks_x  / (max_x-min_x)) - block_load_x, 0) ;
	int endi =   fmin( ceil( (startx - min_x + (gx+1) * get_local_size(0) * pixelscale) * blocks_x  / (max_x-min_x)) + block_load_x, blocks_x-1);

	for(int k = topz; k <= bottomz; k++) {
		for (int j = startj ; j <= endj; j++) {
			//Need list of atoms to load, so we can load in sequence
			int start = block_start_pos[k*blocks_x*blocks_y + blocks_x*j + starti];
			int end = block_start_pos[k*blocks_x*blocks_y + blocks_x*j + endi + 1];

			int gid = start + lid;

			if(lid < end-start) {
				atx[lid] = pos_x[gid];
				aty[lid] = pos_y[gid];
				atZ[lid] = atomic_num[gid];
			}

			barrier(CLK_LOCAL_MEM_FENCE);

			for (int l = 0; l < end-start; l++) {
				// calculate the radius from the current position in space (i.e. pixel?)
				double rad = native_sqrt((startx + xid*pixelscale-atx[l])*(startx + xid*pixelscale-atx[l]) + (starty + yid*pixelscale-aty[l])*(starty + yid*pixelscale-aty[l]));

				if(rad < 0.25 * pixelscale) // is this sensible?
					rad = 0.25 * pixelscale;

				if( rad < 3.0) { // Should also make sure is not too small
					if (param_selector == 0)
                        sumz += kirkland(params, atZ[l], rad);
                    else if (param_selector == 1)
                        sumz += peng(params, atZ[l], rad);
                    else if (param_selector == 2)
                        sumz += lobato(params, atZ[l], rad);
				}
			}

			barrier(CLK_LOCAL_MEM_FENCE);
		}
	}

	if(xid < width && yid < height) {
		potential[id].x = cos(sigma*sumz);
		potential[id].y = sin(sigma*sumz);
	}
}
