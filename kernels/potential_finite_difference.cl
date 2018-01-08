////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Generates the crystal potential (using the finite difference method)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This is really where the magic happens
/// However, I feel it is not well optimised, or at least not well organised
///
/// This kernel calculates the actual potential for each slice, which the wave function is then propagated through. Most
/// of the parameterisation stuff can be found in Kirkland's "Advanced computing in electron microscopy 2nd ed." in
/// appendix C (also where the parameters are given). Particularly equation C.19 gives the actual equation to generate
/// the potential. The rest of this function is loading the atoms, calculating whether they are relevant and maybe more.
/// Note that there are lot's of funny indices when accessing the parameters. This is because they are kept in one array
/// grouped into twelve for each atomic number, these values represent a1, b1, a2, b2, a3, b3, c1, d1, c2, d2, c3, d3.
/// so accessing all the values is a bit clumsy.
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void clBinnedAtomicpotentialOptFD( __global float2* potential,
											__global const float* restrict pos_x,
											__global const float* restrict pos_y,
											__global const float* restrict pos_z,
											__global const int* restrict atomic_num,
											__constant float* params,
											__global const int* restrict block_start_pos,
											int width,
											int height,
											int current_slices, 
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
											float starty)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	int lid = get_local_id(0) + get_local_size(0)*get_local_id(1);
	int Index = xid + width*yid;
	int topz = current_slices - slice_load_z;
	int bottomz = current_slices + slice_load_z;
	float sumz = 0.0f;
	int gx = get_group_id(0);
	int gy = get_group_id(1);

	if(topz < 0 )
		topz = 0;
	if(bottomz >= total_slices )
		bottomz = total_slices-1;

	__local float atx[256];
	__local float aty[256];
	__local float atz[256];
	__local int atZ[256];

	int startj = fmax(floor(((starty + gy * get_local_size(1) * pixel_scale) * blocks_y ) / (max_y-min_y)) - block_load_y,0) ;
	int endj = fmin(ceil(((starty + (gy+1) * get_local_size(1) * pixel_scale) * blocks_y) / (max_y-min_y)) + block_load_y,blocks_y-1);
	int starti = fmax(floor(((startx + gx * get_local_size(0) * pixel_scale) * blocks_x)  / (max_x-min_x)) - block_load_x,0) ;
	int endi = fmin(ceil(((startx + (gx+1) * get_local_size(0) * pixel_scale) * blocks_x) / (max_x-min_x)) + block_load_x,blocks_x-1);

	for(int k = topz; k <= bottomz; k++)
	{
		for (int j = startj ; j <= endj; j++)
		{
			//Need list of atoms to load, so we can load in sequence

			int start = block_start_pos[k*blocks_x*blocks_y + blocks_x*j + starti];
			int end = block_start_pos[k*blocks_x*blocks_y + blocks_x*j + endi + 1];

			int gid = start + lid;

			if(lid < end-start)
			{
				atx[lid] = pos_x[gid];
				aty[lid] = pos_y[gid];
				atz[lid] = pos_z[gid];
				atZ[lid] = atomic_num[gid];
			}

			barrier(CLK_LOCAL_MEM_FENCE);

			for (int l = 0; l < end-start; l++)
			{
				float xyrad2 = (startx + xid*pixel_scale-atx[l])*(startx + xid*pixel_scale-atx[l]) + (starty + yid*pixel_scale-aty[l])*(starty + yid*pixel_scale-aty[l]);

				int ZNum = atZ[l];

				float rad = native_sqrt(xyrad2 + (z-atz[l])*(z-atz[l]));

				if(rad < 0.25f * pixel_scale)
					rad = 0.25f * pixel_scale;

				float p1 = 0;

				if( rad < 3.0f) // Should also make sure is not too small
				{
					p1 += (150.4121417f * native_recip(rad) * params[(ZNum-1)*12  ]* native_exp( -2.0f*3.141592f*rad*native_sqrt(params[(ZNum-1)*12+1  ])));
					p1 += (150.4121417f * native_recip(rad) * params[(ZNum-1)*12+2]* native_exp( -2.0f*3.141592f*rad*native_sqrt(params[(ZNum-1)*12+2+1])));
					p1 += (150.4121417f * native_recip(rad) * params[(ZNum-1)*12+4]* native_exp( -2.0f*3.141592f*rad*native_sqrt(params[(ZNum-1)*12+4+1])));
					p1 += (266.5157269f * params[(ZNum-1)*12+6] * native_exp (-3.141592f*rad*3.141592f*rad/params[(ZNum-1)*12+6+1]) * native_powr(params[(ZNum-1)*12+6+1],-1.5f));
					p1 += (266.5157269f * params[(ZNum-1)*12+8] * native_exp (-3.141592f*rad*3.141592f*rad/params[(ZNum-1)*12+8+1]) * native_powr(params[(ZNum-1)*12+8+1],-1.5f));
					p1 += (266.5157269f * params[(ZNum-1)*12+10] * native_exp (-3.141592f*rad*3.141592f*rad/params[(ZNum-1)*12+10+1]) * native_powr(params[(ZNum-1)*12+10+1],-1.5f));

					sumz +=p1;
				}
			}

			barrier(CLK_LOCAL_MEM_FENCE);
		}
	}

	if(xid < width && yid < height)
	{
		potential[Index].x = sumz;
		potential[Index].y = 0;
	}
}
