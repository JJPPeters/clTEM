////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Generates the crystal potential (using the full 3d method)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This is really where the magic happens
/// However, I feel it is not well optimised, or at least not well organised
///
/// Note that the main difference from the 'conventional' method is the use of equation C.19 instead of C.20
/// This actually seems to go one further and calculate the potential for a range of sub slices to form the slice
/// potential.
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
/// integrals - the number of sub-slices used to build the full 3d potential
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
        suml += a * native_exp(-x * native_sqrt(b) );
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
        double d_inv_root = native_rsqrt(d);
        sumg += c * (d_inv_root*d_inv_root*d_inv_root) * native_exp(-x * native_recip(d));
    }

    // The funny doubles are from the remaining constants in equation C.20
    // Not that they use the fundamental charge as 14.4 Volt-Angstroms
    return 150.4121417 * native_recip(rad) * suml + 266.5157269 * sumg;
 }

double lobato(__constant double* params, int ZNum, double rad) {
    int i;
    double sum, x;
    sum = 0.0;

    x = M_PI * rad;

    for(i=0; i < 5; ++i) {
        double a = params[(ZNum-1)*10+i];
        double b = params[(ZNum-1)*10+i+5];
        double b_inv_root = native_rsqrt(b);
        sum += a * (b_inv_root*b_inv_root*b_inv_root) * native_exp(-2.0 * x * b_inv_root) * (native_sqrt(b) * native_recip(x) + 1.0);
    }

    return 472.545072199968 * sum;
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

        sum += a * native_exp(-x * native_recip(b));
    }

    return 1844.76074609315 * sum;
}

__kernel void potential_full_3d_d( __global double2* potential,
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
										  double pixel_scale,
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
										  double starty,
										  int integrals)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	int lid = get_local_id(0) + get_local_size(0)*get_local_id(1);
	int id = xid + width * yid;
	int topz = current_slice - slice_load_z;
	int bottomz = current_slice + slice_load_z;
	double sumz = 0.0;
	int gx = get_group_id(0);
	int gy = get_group_id(1);
	double int_r = native_recip(integrals);

	if(topz < 0 )
		topz = 0;
	if(bottomz >= total_slices )
		bottomz = total_slices-1;

	__local double atx[256];
	__local double aty[256];
	__local double atz[256];
	__local int atZ[256];

	int startj = fmax(floor( (starty - min_y +  gy    * get_local_size(1) * pixel_scale) * blocks_y * native_recip(max_y-min_y)) - block_load_y, 0) ;
	int endj =   fmin( ceil( (starty - min_y + (gy+1) * get_local_size(1) * pixel_scale) * blocks_y * native_recip(max_y-min_y)) + block_load_y, blocks_y-1);
	int starti = fmax(floor( (startx - min_x +  gx    * get_local_size(0) * pixel_scale) * blocks_x * native_recip(max_x-min_x)) - block_load_x, 0) ;
	int endi =   fmin( ceil( (startx - min_x + (gx+1) * get_local_size(0) * pixel_scale) * blocks_x * native_recip(max_x-min_x)) + block_load_x, blocks_x-1);

	for(int k = topz; k <= bottomz; k++) {
		for (int j = startj ; j <= endj; j++) {
			//Need list of atoms to load, so we can load in sequence
			int start = block_start_pos[k*blocks_x*blocks_y + blocks_x*j + starti];
			int end = block_start_pos[k*blocks_x*blocks_y + blocks_x*j + endi + 1];

			int gid = start + lid;

			if(lid < end-start) {
				atx[lid] = pos_x[gid];
				aty[lid] = pos_y[gid];
				atz[lid] = pos_z[gid];
				atZ[lid] = atomic_num[gid];
			}

			barrier(CLK_LOCAL_MEM_FENCE);

			double p2=0.0;
			for (int l = 0; l < end-start; l++) {
				double xyrad2 = (startx + xid*pixel_scale-atx[l])*(startx + xid*pixel_scale-atx[l]) + (starty + yid*pixel_scale-aty[l])*(starty + yid*pixel_scale-aty[l]);

				for (int h = 0; h <= integrals; h++) {
					// not sure how the integrals work here (integrals = integrals)
					// I think we are generating multiple subslices for each slice (nut not propagating through them,
					// just building our single slice potential from them
					double rad = native_sqrt(xyrad2 + (z - h * dz * int_r - atz[l])*(z - h * dz * int_r - atz[l]));

					if(rad < 0.25 * pixel_scale)
						rad = 0.25 * pixel_scale;

					double p1 = 0.0;

					if( rad < 3.0) { // Should also make sure is not too small
						// note that all the funny numbers are just pre-calculated groups of constants
						double p1;
						if (param_selector == 0)
						    p1 = kirkland(params, atZ[l], rad);
                        else if (param_selector == 1)
                            p1 = peng(params, atZ[l], rad);
                        else if (param_selector == 2)
                            p1 = lobato(params, atZ[l], rad);

						// why make sure h!=0 when we can just remove it from the loop?
						// surely h == 0 will be in the previous slice??
						// because p1 is used in the next iteration (why it is set to p2)
						sumz += (h!=0) * (p1+p2)*0.5;
						p2 = p1;
					}
				}
			}

			barrier(CLK_LOCAL_MEM_FENCE);
		}
	}
	if(xid < width && yid < height) {
		potential[id].x = cos((dz * int_r) * sigma * sumz);
		potential[id].y = sin((dz * int_r) * sigma * sumz);
	}
}
