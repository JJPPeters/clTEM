////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Finite difference propagator
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Kernel used for propagating the wavefunction using the finite difference method. This can mostly be summarised in 
/// section 6.11.2 of Kirkland's "Advanced computing in electron microscopy 2nd ed.." Note that the laplacian has
/// already been calculated (see equation 6.118). The actual calculation performed here is given by equation 6.115
/// should be more "accurate to about 1 order better than the standard multislice method"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// potential - the crystal potential for the current slice step
/// laplaced - the wavefunction (psi) after the laplace operator has been applied
/// psi_minus - the previous wavefunction
/// psi - the current wavefunction
/// psi_plus - the next wave function (to be calculated here)
/// wavelength - the wavelength of the electron beam (units?)
/// sigma - the interaction parameter (given by eq. 5.6 in Kirkland)
/// width - width of input_output
/// height - height of input_output
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void clFiniteDifference( __global float2* restrict potential,
								  __global float2* restrict laplaced,
								  __global float2* restrict psi_minus,
								  __global float2* restrict psi,
								  __global float2* restrict psi_plus,
								  float FDdz,
								  float wavelength,
								  float sigma,
								  int width,
								  int height)
{
	int xid = get_global_id(0);
	int yid = get_global_id(1);
	int Index = xid + width* yid;

	float2 cMinus = {1 , -2*3.14159f*FDdz/wavelength};
	float2 cPlus = {1 , 2*3.14159f*FDdz/wavelength};

	float2 reciprocalCPlus = {cMinus.x / (cMinus.x*cMinus.x + cMinus.y*cMinus.y),cMinus.y / (cMinus.x*cMinus.x + cMinus.y*cMinus.y)};
	float2 cMinusOvercPlus = {(cPlus.x*cPlus.x - cPlus.y*cPlus.y) / (cMinus.x*cMinus.x + cMinus.y*cMinus.y),-2*(cPlus.x*cPlus.y) / (cMinus.x*cMinus.x + cMinus.y*cMinus.y)};

	if(xid < width && yid < height)
	{
		float real = reciprocalCPlus.x*(2*psi[Index].x-FDdz*FDdz*laplaced[Index].x - FDdz*FDdz*4*3.14159f*sigma*potential[Index].x*psi[Index].x/wavelength)
				-reciprocalCPlus.y*(2*psi[Index].y-FDdz*FDdz*laplaced[Index].y -  FDdz*FDdz*4*3.14159f*sigma*potential[Index].x*psi[Index].y/wavelength)
				-cMinusOvercPlus.x*(psi_minus[Index].x) + cMinusOvercPlus.y*(psi_minus[Index].y);

		float imag = reciprocalCPlus.y*(2*psi[Index].x-FDdz*FDdz*laplaced[Index].x - FDdz*FDdz*4*3.14159f*sigma*potential[Index].x*psi[Index].x/wavelength)
				+reciprocalCPlus.x*(2*psi[Index].y-FDdz*FDdz*laplaced[Index].y -  FDdz*FDdz*4*3.14159f*sigma*potential[Index].x*psi[Index].y/wavelength)
				-cMinusOvercPlus.y*(psi_minus[Index].x) - cMinusOvercPlus.x*(psi_minus[Index].y);

		psi_plus[Index].x = real;
		psi_plus[Index].y = imag;
	}
}