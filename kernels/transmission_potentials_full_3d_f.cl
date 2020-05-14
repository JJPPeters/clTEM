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
/// pixel_scale - pixel scale of the image in real space
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

float kirkland(__constant float* params, int ZNum, float rad) {
    int i;
    float suml, sumg, x;
    suml = 0.0f;
    sumg = 0.0f;
    //
    // Lorentzians
    //
    x = 2.0f * M_PI_F * rad;

    // Loop through our parameters (a and b)
    for(i=0; i<6; i+=2) {
        float a = params[(ZNum-1)*12+i];
        float b = params[(ZNum-1)*12+i+1];
        suml += a * native_exp(-x * native_sqrt(b) );
    }

    //
    // Gaussians
    //
    x = M_PI_F * rad;
    x = x * x;

    // Loop through our parameters (a and b)
    for(i=6; i<12; i+=2) {
        float c = params[(ZNum-1)*12+i];
        float d = params[(ZNum-1)*12+i+1];
        float d_inv_root = native_rsqrt(d);
        sumg += c * (d_inv_root*d_inv_root*d_inv_root) * native_exp(-x * native_recip(d));
    }

    // The funny floats are from the remaining constants in equation C.20
    // Not that they use the fundamental charge as 14.4 Volt-Angstroms
    return 150.4121417f * native_recip(rad) * suml + 266.5157269f * sumg;
 }

float lobato(__constant float* params, int ZNum, float rad) {
    int i;
    float sum, x;
    sum = 0.0f;

    x = M_PI_F * rad;

    for(i=0; i < 5; ++i) {
        float a = params[(ZNum-1)*10+i];
        float b = params[(ZNum-1)*10+i+5];
        float b_inv_root = native_rsqrt(b);
        sum += a * (b_inv_root*b_inv_root*b_inv_root) * native_exp(-2.0f * x * b_inv_root) * (native_sqrt(b) * native_recip(x) + 1.0f);
    }

    return 472.545072199968f * sum;
}

float peng(__constant float* params, int ZNum, float rad) {
    int i;
    float sum, x;
    sum = 0.0f;

    x = M_PI_F * rad;
    x = x * x;

    for(i=0; i<5; ++i) {
        float a = params[(ZNum-1)*10+i];
        float b = params[(ZNum-1)*10+i+5];
        float b_inv_root = native_rsqrt(b);

        sum += a * (b_inv_root*b_inv_root*b_inv_root) * native_exp(-x * native_recip(b));
    }

    return 266.5157269f * sum;
}

__kernel void transmission_potentials_full_3d_f( __global float2* potential,
										  __global const float* restrict pos_x,
										  __global const float* restrict pos_y,
										  __global const float* restrict pos_z,
										  __global const int* restrict atomic_num,
										  __constant float* params,
                                          unsigned int param_selector,
										  __global const int* restrict block_start_pos,
										  unsigned int width,
										  unsigned int height,
										  int current_slice,
										  int total_slices,
										  float z,
										  float dz,
										  float pixel_scale,
										  int blocks_x,
										  int blocks_y,
										  float max_x,
										  float min_x,
										  float max_y,
										  float min_y,
										  int block_load_x,
										  int block_load_y,
										  int slice_load_z,
										  float sigma,
										  float startx,
										  float starty,
                                          float slice_shift_x,
                                          float slice_shift_y,
										  int integrals)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	int lid = get_local_id(0) + get_local_size(0)*get_local_id(1);
	int id = xid + width * yid;
	int topz = current_slice - slice_load_z;
	int bottomz = current_slice + slice_load_z;
	float sumz = 0.0f;
	int gx = get_group_id(0);
	int gy = get_group_id(1);
	float int_r = native_recip(integrals);

	if(topz < 0 )
		topz = 0;
	if(bottomz >= total_slices )
		bottomz = total_slices-1;

	__local float atx[256];
	__local float aty[256];
	__local float atz[256];
	__local int atZ[256];

	// calculate the indices of the bins we will need
    // get the size of one workgroup
    float group_size_x = get_local_size(0) * pixel_scale;
    float group_size_y = get_local_size(1) * pixel_scale;

    // get the start and end position of the current workgroup
    float group_start_x = startx +  gx      * group_size_x;
    float group_end_x   = startx + (gx + 1) * group_size_x;

    float group_start_y = starty +  gy      * group_size_y;
    float group_end_y   = starty + (gy + 1) * group_size_y;

    // get the reciprocal of the full range (for efficiency)
    float recip_range_x = native_recip(max_x - min_x);
    float recip_range_y = native_recip(max_y - min_y);

    int starti = fmax(floor( blocks_x * (group_start_x - min_x) * recip_range_x) - block_load_x, 0);
    int endi   = fmin( ceil( blocks_x * (group_end_x   - min_x) * recip_range_x) + block_load_x, blocks_x - 1);
    int startj = fmax(floor( blocks_y * (group_start_y - min_y) * recip_range_y) - block_load_y, 0);
    int endj   = fmin( ceil( blocks_y * (group_end_y   - min_y) * recip_range_y) + block_load_y, blocks_y - 1);

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

			float p2=0.0f;
			for (int l = 0; l < end-start; l++) {
				// calculate the radius from the current position in space (i.e. pixel?)
                float im_pos_x = startx + xid * pixel_scale;
                float rad_x = im_pos_x - atx[l];

                float im_pos_y = starty + yid * pixel_scale;
                float rad_y = im_pos_y - aty[l];

				for (int h = 0; h < integrals; h++) {
					// not sure how the integrals work here (integrals = integrals)
					// I think we are generating multiple subslices for each slice (nut not propagating through them,
					// just building our single slice potential from them

                    // account for shift due to beam tilt
                    rad_x -= slice_shift_x;
                    rad_y -= slice_shift_y;

                    float xyrad2 = rad_x*rad_x + rad_y*rad_y;

					// z is the slice position, h is the 'sub' integral, dz is the slice thickness and int_r is 1/integrals
					// so basically this gets our exact z position...
					float im_pos_z = z - h * dz * int_r;
					float rad_z = im_pos_z - atz[l];

					float rad = native_sqrt(xyrad2 + rad_z*rad_z);

					float r_min = 0.25f * pixel_scale;
                    if(rad < r_min) // avoid singularity at 0 (value used by kirkland)
                        rad = r_min;

					float p1 = 0.0f;

					if(xyrad2 <= 64.0f && rad_z <= 3.0f) {
    					float p1;
    					if (param_selector == 0)
    					    p1 = kirkland(params, atZ[l], rad);
                        else if (param_selector == 1)
                            p1 = peng(params, atZ[l], rad);
                        else if (param_selector == 2)
                            p1 = lobato(params, atZ[l], rad);

    					// why make sure h!=0 when we can just remove it from the loop?
    					// surely h == 0 will be in the previous slice??
    					// because p1 is used in the next iteration (why it is set to p2)
    					sumz += (h!=0) * (p1+p2)*0.5f;
    					p2 = p1;
					}
				}
			}

			barrier(CLK_LOCAL_MEM_FENCE);
		}
	}
	if(xid < width && yid < height) {
		potential[id].x = native_cos((dz * int_r) * sigma * sumz);
		potential[id].y = native_sin((dz * int_r) * sigma * sumz);
	}
}
