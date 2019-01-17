////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Generates the crystal potential (using the conventional technique, no accounting for z)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This is really where the magic happens
/// However, I feel it is not well optimised, or at least not well organised
///
/// This kernel calculates the actual potential for each slice, which the wave function is then propagated through. Most
/// of the parameterisation stuff can be found in Kirkland's "Advanced computing in electron microscopy 2nd ed." in
/// appendix C (also where the parameters are given). Particularly equation C.20 gives the actual equation to generate
/// the potential. The rest of this function is loading the atoms, calculating whether they are relevant and maybe more.
/// Note that there are lot's of funny indices when accessing the parameters. This is because they are kept in one array
/// grouped into twelve for each atomic number, these values represent a1, b1, a2, b2, a3, b3, c1, d1, c2, d2, c3, d3.
/// so accessing all the values is a bit clumsy.
///
/// TODO: the finite difference method incorporates the z position (but is otherwise the same) what reason is there that
/// this one cannot use that?
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
/// Bessel function
/// These function's can be found in the numerical recipes books (at least the C one)
/// bessi0 - calculate the zero order modified bessel function of the first kind (using only for bessk0)
/// bessk0 - calculate the zero order modified bessel function of the second kind
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float bessi0(float x)
{
	int i;
	float ax, sum, t;

	float i0a[] = { 1.0f, 3.5156229f, 3.0899424f, 1.2067492f, 0.2659732f, 0.0360768f, 0.0045813f };
	float i0b[] = { 0.39894228f, 0.01328592f, 0.00225319f, -0.00157565f, 0.00916281f, -0.02057706f, 0.02635537f, -0.01647633f, 0.00392377f};

	ax = fabs( x );

	if( ax <= 3.75f )
	{
		t = x / 3.75f;
		t = t * t;
		sum = i0a[6];

		for( i=5; i>=0; i--)
			sum = sum*t + i0a[i];
	}
	else
	{
		t = 3.75f / ax;
		sum = i0b[8];
		for( i=7; i>=0; i--)
			sum = sum*t + i0b[i];

		sum = native_exp( ax ) * sum / native_sqrt( ax );
	}

	return sum;
}

float bessk0(float x)
{
	int i;
	float ax, x2, sum;
	float k0a[] = { -0.57721566f, 0.42278420f, 0.23069756f, 0.03488590f, 0.00262698f, 0.00010750f, 0.00000740f };
	float k0b[] = { 1.25331414f, -0.07832358f, 0.02189568f, -0.01062446f, 0.00587872f, -0.00251540f, 0.00053208f };

	ax = fabs( x );

	if( (ax > 0.0f)  && ( ax <=  2.0f ) )
	{
		x2 = ax/2.0f;
		x2 = x2 * x2;
		sum = k0a[6];
		for( i=5; i>=0; i--)
			sum = sum*x2 + k0a[i];
		sum = -log(ax/2.0f) * bessi0(x) + sum;
	}
	else if( ax > 2.0f )
	{
		x2 = 2.0f/ax;
		sum = k0b[6];
		for( i=5; i>=0; i--)
			sum = sum*x2 + k0b[i];

		sum = native_exp( -ax ) * sum / native_sqrt( ax );
	}
	else
		sum = 1.0e20;

	return sum;
}

__kernel void clBinnedAtomicPotentialConventional( __global float2* potential,
											       __global const float* restrict pos_x,
										  		   __global const float* restrict pos_y,
										 		   __global const float* restrict pos_z,
												   __global const int* restrict atomic_num,
												   __constant float* params,
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
												   float starty)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	int lid = get_local_id(0) + get_local_size(0)*get_local_id(1);
	int Index = xid + width * yid;
	int topz = current_slice;
	int bottomz = current_slice;
	float sumz = 0.0f;
	int gx = get_group_id(0);
	int gy = get_group_id(1);

	if(topz < 0 )
		topz = 0;
	if(bottomz >= total_slices )
		bottomz = total_slices-1;

	__local float atx[256];
	__local float aty[256];
	__local int atZ[256];

	// calculate the indices of the bins we will need?
	int startj = fmax(floor( (starty - min_y+  gy *    get_local_size(1) * pixelscale) * blocks_y  / (max_y-min_y)) - block_load_y, 0) ;
	int endj =   fmin( ceil( (starty - min_y + (gy+1) * get_local_size(1) * pixelscale) * blocks_y  / (max_y-min_y)) + block_load_y, blocks_y-1);
	int starti = fmax(floor( (startx - min_x +  gx *    get_local_size(0) * pixelscale) * blocks_x  / (max_x-min_x)) - block_load_x, 0) ;
	int endi =   fmin( ceil( (startx - min_x + (gx+1) * get_local_size(0) * pixelscale) * blocks_x  / (max_x-min_x)) + block_load_x, blocks_x-1);

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
				atZ[lid] = atomic_num[gid];
			}

			barrier(CLK_LOCAL_MEM_FENCE);

			for (int l = 0; l < end-start; l++)
			{
				int ZNum = atZ[l];

				// calculate the radius from the current position in space (i.e. pixel?)
				float rad = native_sqrt((startx + xid*pixelscale-atx[l])*(startx + xid*pixelscale-atx[l]) + (starty + yid*pixelscale-aty[l])*(starty + yid*pixelscale-aty[l]));

				if(rad < 0.25f * pixelscale) // is this sensible?
					rad = 0.25f * pixelscale;

				if( rad < 3.0f) // Should also make sure is not too small
				{
					int i;
					float suml, sumg, x;

					/* avoid singularity at r=0 */
					suml = sumg = 0.0f;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This part is given by equation C.20 in Kirkland (i.e. actually transforms the paremeterisation into the potential)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////					
					/* Lorenztians */
					x = 2.0f*3.141592654f*rad;

					for( i=0; i<2*3; i+=2 )
						suml += params[(ZNum-1)*12+i]* bessk0( x*native_sqrt(params[(ZNum-1)*12+i+1]) );

					/* Gaussians */
					x = 3.141592654f*rad;
					x = x*x;

					for( i=2*3; i<2*(3+3); i+=2 )
						sumg += params[(ZNum-1)*12+i] * native_exp (-x/params[(ZNum-1)*12+i+1]) / params[(ZNum-1)*12+i+1];

					// I'm assuming that the funny floats are from the remaining constants in equation C.20
					sumz += 300.8242834f*suml + 150.4121417f*sumg;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				}
			}

			barrier(CLK_LOCAL_MEM_FENCE);
		}
	}

	if(xid < width && yid < height)
	{
		potential[Index].x = native_cos(sigma*sumz);
		potential[Index].y = native_sin(sigma*sumz);
	}
}
