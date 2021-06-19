  
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void vecMean(__global double* x)

{

	double y,z;

	int gid = get_global_id(0);
	
	y += x[gid]
	z = y/sizeof(x)
}

__kernel void vecDev(__global double* x)

{
	double y,z,i,j,k;
	int gid = get_global_id(0);
	
	y += x[gid]
	z = y/sizeof(x)
	i += (x[gid]*x[gid])
	j = i/sizeof(x)
	k = j**(1/2)
}