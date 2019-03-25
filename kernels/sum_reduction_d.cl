////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Calculate the partial sum of an image
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Takes an image and performs initial calculations to get the sum of the image. Final output is then more efficient to
/// calculate the final sum on a CPU
/// TODO: why is the ouput complex if the complex part is always 0
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// input - image to be summed
/// output - output of partially summed image
/// size - total size of the input image
/// buffer - tempprary buffer to store part of the input to then be summed independently
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void sum_reduction_d( __global const double* input,
							  __global double* output,
							  const unsigned int size,
							  __local double* buffer)
{
	// Get the work items ID
	size_t idx = get_local_id(0);
	size_t stride = get_global_size(0);
	buffer[idx] = 0.0;

	for(size_t pos = get_global_id(0); pos < size; pos += stride )
		buffer[idx] += input[pos];

	barrier(CLK_LOCAL_MEM_FENCE);

	double sum = 0.0;
	if(!idx) {
		for(size_t i = 0; i < get_local_size(0); ++i)
			sum += fabs(buffer[i]);

		output[get_group_id(0)] = sum * M_SQRT2;
	}
}