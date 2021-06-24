  
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void vecMean( __global double* x)

{

	double a,y;

	int gid = get_global_id(0);
	
	y += x[gid]
	a = y/sizeof(x)
}

__kernel void vecDev( __global double* x)

{
	double b,y,z,i,j;
	int gid = get_global_id(0);
	
	y += x[gid]
	z = y/sizeof(x)
	i += (x[gid]*x[gid])
	j = i/sizeof(x)
	b = j**(1/2)
}