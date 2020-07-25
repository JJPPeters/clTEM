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
/// bessk0 - calculate the zero order modified bessel function of the second kind
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

#define recip(x) (1.0f / (x))

__constant float k0pi[5] = {1.0f, 2.346487949187396e-1f, 1.187082088663404e-2f, 2.150707366040937e-4f, 1.425433617130587e-6f};
__constant float k0qi[3] = {9.847324170755358e-1f, 1.518396076767770e-2f, 8.362215678646257e-5f};
__constant float k0p[5] = {1.159315156584126e-1f, 2.770731240515333e-1f, 2.066458134619875e-2f, 4.574734709978264e-4f, 3.454715527986737e-6f};
__constant float k0q[3] = {9.836249671709183e-1f, 1.627693622304549e-2f, 9.809660603621949e-5f};
__constant float k0pp[8] = {1.253314137315499f, 1.475731032429900e1f, 6.123767403223466e1f, 1.121012633939949e2f, 9.285288485892228e1f, 3.198289277679660e1f, 3.595376024148513f, 6.160228690102976e-2f};
__constant float k0qq[8] = {1.0f, 1.189963006673403e1f, 5.027773590829784e1f, 9.496513373427093e1f, 8.318077493230258e1f, 3.181399777449301e1f, 4.443672926432041f, 1.408295601966600e-1f};

__constant float k1pi[5] = {0.5f, 5.598072040178741e-2f, 1.818666382168295e-3f, 2.397509908859959e-5f, 1.239567816344855e-7f};
__constant float k1qi[3] = {9.870202601341150e-1f, 1.292092053534579e-2f, 5.881933053917096e-5f};
__constant float k1p[5] = {-3.079657578292062e-1f, -8.109417631822442e-2f, -3.477550948593604e-3f, -5.385594871975406e-5f, -3.110372465429008e-7f};
__constant float k1q[3] = {9.861813171751389e-1f, 1.375094061153160e-2f, 6.774221332947002e-5f};
__constant float k1pp[8] = {1.253314137315502f, 1.457171340220454e1f, 6.063161173098803e1f, 1.147386690867892e2f, 1.040442011439181e2f, 4.356596656837691e1f, 7.265230396353690f, 3.144418558991021e-1f};
__constant float k1qq[8] = {1.0f, 1.125154514806458e1f, 4.427488496597630e1f, 7.616113213117645e1f, 5.863377227890893e1f, 1.850303673841586e1f, 1.857244676566022f, 2.538540887654872e-2f};

float poly(__constant float* cof, int n, float x) {
    float ans = cof[n];
    for (int i = n-1; i >= 0; --i)
        ans = ans * x + cof[i];
    return ans;
}

float bessk0(float x) {
    float ax = fabs(x);
	if(ax > 0.0f && ax <= 1.0f) {
		float z = x * x;
        float term = poly(k0pi, 4, z) * native_log(x) * native_recip(poly(k0qi, 2, 1.0f-z));
		return poly(k0p, 4, z) * native_recip(poly(k0q, 2, 1.0f-z)) - term;
	} else if (ax > 1.0f) {
		float z = native_recip(x);
		return native_exp(-x) * poly(k0pp, 7, z) * native_recip(poly(k0qq, 7, z)) * native_rsqrt(x);
	} else
        return FLT_MAX;
}

float bessk1(float x) {
    float ax = fabs(x);
    if(ax > 0.0f && ax <= 1.0f) {
        float z = x * x;
        float term = poly(k1pi, 4, z) * native_log(x) * native_recip(poly(k1qi, 2, 1.0f-z));
        return x * ( poly(k1p, 4, z) * native_recip(poly(k1q, 2, 1.0f-z)) + term ) + native_recip(x);
    } else if (ax > 1.0f) {
        float z = native_recip(x);
        return native_exp(-x) * poly(k1pp, 7, z) * native_recip(poly(k1qq, 7, z)) * native_rsqrt(x);
    } else
        return FLT_MAX; 
}

float kirkland(__constant float* params, int i_lim, int ZNum, float rad) {
    int i;
    float suml, sumg, x;
    suml = 0.0f;
    sumg = 0.0f;

    int z_ofst = (ZNum - 1) * 12;

    //
    // Lorentzians
    //
    x = 2.0f * M_PI_F * rad;

    // Loop through our parameters (a and b)
    for(i = 0; i < i_lim*2; i+=2) {
        float a = params[z_ofst + i];
        float b = params[z_ofst + i + 1];
        suml += a * bessk0( x * native_sqrt(b) );
    }

    //
    // Gaussians
    //
    x = M_PI_F * rad;
    x = x * x;

    // Loop through our parameters (a and b)
    for(i = i_lim*2; i < i_lim*4; i+=2) {
        float c = params[z_ofst + i];
        float d = params[z_ofst + i + 1];
        float d_inv = native_recip(d);
        sumg += (c * d_inv) * native_exp(-x * d_inv);
    }

    // The funny floats are from the remaining constants in equation C.20
    // Not that they use the fundamental charge as 14.4 Volt-Angstroms
    return 300.8242834f * suml + 150.4121417f * sumg;
 }

float lobato(__constant float* params, int i_lim, int ZNum, float rad) {
    int i;
    float sum, x;
    sum = 0.0f;

    int z_ofst = (ZNum - 1) * 10;

    x = 2.0f * M_PI_F * rad;

    for(i=0; i < i_lim; ++i) {
        float a = params[z_ofst+i];
        float b = params[z_ofst+i+5];
        float b_inv_root = native_rsqrt(b);
        sum += a * (b_inv_root * b_inv_root * b_inv_root) * (bessk0(x * b_inv_root) + rad * bessk1(x * b_inv_root));
    }

    return 945.090144399935f * sum;
}

float peng(__constant float* params, int i_lim, int ZNum, float rad) {
    int i;
    float sum, x;
    sum = 0.0f;

    int z_ofst = (ZNum - 1) * 10;

    x = M_PI_F * rad;
    x = x * x;

    for(i=0; i < i_lim; ++i) {
        float a = params[z_ofst+i];
        float b = params[z_ofst+i+5];
        float b_inv = native_recip(b);

        sum += a * b_inv * native_exp(-x * b_inv);
    }

    return 150.4121417f * sum;
}

__kernel void transmission_potentials_projected_f( __global float2* potential,
							         __global const float* restrict pos_x,
						  		     __global const float* restrict pos_y,
						 		     __global const float* restrict pos_z,
								     __global const int* restrict atomic_num,
								     __constant float* params,
								     unsigned int param_selector,
									 unsigned int param_i_count,
						 		     __global const int* restrict block_start_pos,
								     unsigned int width,
								     unsigned int height,
								     int current_slice,
								     int total_slices,
								     float z,
								     float dz,
								     float pixelscale, 
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
								     float beam_theta,
								     float beam_phi)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	int lid = get_local_id(0) + get_local_size(0)*get_local_id(1);
	int id = xid + width * yid;
	float sumz = 0.0f;
	int gx = get_group_id(0);
	int gy = get_group_id(1);
	// convert from mrad to radians (and get beam tilt from the surface)
	beam_theta = M_PI_2_F - beam_theta * 0.001f;

	__local float atx[256];
	__local float aty[256];
	__local int atZ[256];

	// calculate the indices of the bins we will need
    // get the size of one workgroup
    float group_size_x = get_local_size(0) * pixelscale;
    float group_size_y = get_local_size(1) * pixelscale;

    // get the start and end position of the current workgroup
    float group_start_x = startx + gx * group_size_x;
    float group_end_x   = group_start_x + group_size_x;

    float group_start_y = starty + gy * group_size_y;
    float group_end_y   = group_start_y + group_size_y;

    // get the reciprocal of the full range (for efficiency)
    float recip_range_x = native_recip(max_x - min_x);
    float recip_range_y = native_recip(max_y - min_y);

    int starti = fmax(floor( blocks_x * (group_start_x - min_x) * recip_range_x) - block_load_x, 0);
    int endi   = fmin( ceil( blocks_x * (group_end_x   - min_x) * recip_range_x) + block_load_x, blocks_x - 1);
	int startj = fmax(floor( blocks_y * (group_start_y - min_y) * recip_range_y) - block_load_y, 0);
	int endj   = fmin( ceil( blocks_y * (group_end_y   - min_y) * recip_range_y) + block_load_y, blocks_y - 1);

    int k = current_slice;
    if (k < 0)
        k = 0;
    if (k >= total_slices)
        k = total_slices - 1;


    // loop through our bins (y only, x is handled using the workgroup)
	for (int j = startj ; j <= endj; j++) {
        // for this y block, get the range of indices to use (this is what the block_Start_pos is) from the x blocks
		int start = block_start_pos[k*blocks_x*blocks_y + blocks_x*j + starti  ];
		int end   = block_start_pos[k*blocks_x*blocks_y + blocks_x*j + endi + 1];

        // this gid is effectively where the atoms indices are looped through (using the local ids)
        // so we are parellelising this over the local workgroup
		int gid = start + lid;

		if(lid < end-start) {
			atx[lid] = pos_x[gid];
			aty[lid] = pos_y[gid];
			atZ[lid] = atomic_num[gid];
		}

        // this makes sure all the local threads have finished getting the atoms we need, atx, aty and atZ are complete
		barrier(CLK_LOCAL_MEM_FENCE);

        // now we parallelise over pixels, not atoms
		for (int l = 0; l < end-start; l++) {
			// calculate the radius from the current position in space
            float im_pos_x = startx + xid * pixelscale;
            float rad_x = im_pos_x - atx[l];

            float im_pos_y = starty + yid * pixelscale;
            float rad_y = im_pos_y - aty[l];

			//float rad = native_sqrt(rad_x*rad_x + rad_y*rad_y);
			float cos_beam_phi = native_cos(beam_phi);
			float sin_beam_phi = native_sin(beam_phi);
			float sin_beam_2theta = native_sin(2.0f * beam_theta);

			float z_prime = -0.5f * (rad_x * cos_beam_phi + rad_y * sin_beam_phi) * sin_beam_2theta;

			float z_by_tan_beam_theta = z_prime / native_tan(beam_theta);

			float x_prime = rad_x + z_by_tan_beam_theta * cos_beam_phi;
			float y_prime = rad_y + z_by_tan_beam_theta * sin_beam_phi;

            float rad = native_sqrt(z_prime*z_prime + x_prime*x_prime + y_prime*y_prime);

            float r_min = 0.25f * pixelscale;
            if(rad < r_min) // is this sensible?
                rad = r_min;

			if( rad <= 8.0f) { // Should also make sure is not too small
				if (param_selector == 0)
                    sumz += kirkland(params, param_i_count, atZ[l], rad);
                else if (param_selector == 1)
                    sumz += peng(params, param_i_count, atZ[l], rad);
                else if (param_selector == 2)
                    sumz += lobato(params, param_i_count, atZ[l], rad);
			}
		}

		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if(xid < width && yid < height) {
		potential[id].x = native_cos(sigma * sumz);
		potential[id].y = native_sin(sigma * sumz);
	}
}
