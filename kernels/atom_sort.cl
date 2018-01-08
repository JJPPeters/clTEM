////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sort atom positions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// This kernel takes the position of all the atoms and labels them with two ids: the block id (essentially grouped by
/// the x, y position) and the z id (essentially determines what slice the atom is in). This is to allow more efficient
/// access of the atom positions/potentials in later kernels. This is only run once at the start of the simulation.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// x_input - x positions of the input atoms
/// y_input - y positions of the input atoms
/// z_input - z positions of the input atoms
/// n_atoms - number of atoms
/// min_x - minimum x position (including padding) // TODO: does this need to include the padding...
/// max_x - maximum x position (including padding)
/// min_y - minimum y position (including padding)
/// max_y - maximum y position (including padding)
/// min_z - minimum z position (including padding)
/// max_z - maximum z position (including padding)
/// blocks_x - number of x blocks
/// blocks_y - number of y blocks
/// block_ids - output block ids for each atom
/// z_ids - output z ids for each atom
/// dz - slice thickness
/// n_slices - number of slices
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void clAtomSort( __global const float* x_input,
						  __global const float* y_input,
						  __global const float* z_input,
						  int n_atoms,
						  float min_x,
						  float max_x,
						  float min_y,
						  float max_y,
						  float min_z,
						  float max_z,
						  int blocks_x,
						  int blocks_y,
						  __global int* block_ids,
						  __global int* z_ids,
						  float dz,
						  int n_slices) 
{		
	int xid = get_global_id(0);	
	if(xid < n_atoms) 
	{	
		// get the fractional position of the atoms (in the structure), times by the number of blocks and floor
		int bidx = floor( (x_input[xid] - min_x) / (max_x - min_x) * blocks_x); 
		int bidy = floor( (y_input[xid] - min_y) / (max_y - min_y) * blocks_y); 
		// I think this assumes that the z is always from 0?
		int zid  = floor( (max_z - z_input[xid]) / dz); 

		// account for any edge cases that are exactly on the limit of z
		zid -= (zid==n_slices); 
		bidx -= (bidx==blocks_x); 
		bidy -= (bidy==blocks_y);

		//calculate the actual block id and return
		int bid = bidx + blocks_x*bidy; 
		block_ids[xid] = bid; 
		z_ids[xid] = zid; 
	}		
}