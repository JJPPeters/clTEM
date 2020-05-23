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

#define recip(x) (1.0 / (x))

__kernel void atom_sort_d( __global const double* x_input,
						  __global const double* y_input,
						  __global const double* z_input,
						  int n_atoms,
						  double min_x,
						  double max_x,
						  double min_y,
						  double max_y,
						  double min_z,
						  double max_z,
						  int blocks_x,
						  int blocks_y,
						  __global int* block_ids,
						  __global int* z_ids,
						  double dz,
						  int n_slices)
{
	int xid = get_global_id(0);
	if(xid < n_atoms)
	{
	    if (x_input[xid] >= min_x && x_input[xid] <= max_x && y_input[xid] >= min_y && y_input[xid] <= max_y) {
            // get the fractional position of the atoms (in the structure), times by the number of blocks and floor
            int bidx = floor( (x_input[xid] - min_x) * native_recip(max_x - min_x) * blocks_x);
            int bidy = floor( (y_input[xid] - min_y) * native_recip(max_y - min_y) * blocks_y);
            // This sorts the top atoms (largest z) to be the frist atoms (i.e. we simulate top down)
            int zid  = floor( (max_z - z_input[xid]) * native_recip(dz));

            // account for any edge cases that are exactly on the limit of z
            zid -= (zid==n_slices);
            bidx -= (bidx==blocks_x);
            bidy -= (bidy==blocks_y);

            // calculate the actual block id and return
            int bid = bidx + blocks_x * bidy;
            block_ids[xid] = bid;
            z_ids[xid] = zid;
        } else {
            block_ids[xid] = -1;
            z_ids[xid] = -1;
        }
	}
}