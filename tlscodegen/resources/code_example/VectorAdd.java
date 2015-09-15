public class VectorAdd {
	public static void VectorAdd_AutoPar(int N,float[] VectorA,float[] VectorB,float[] VectorC)
	{
		int i;
		/*par */
		for(i=0;i<N;i++)
		{
			VectorC[i]=VectorA[i]+VectorB[i];
		}
	}
	
	public static void main(String args[])
	{
		System.out.println("Japonica+ Vector Add Sample");
		int N=100000;
		float a[]=new float[N];
		float b[]=new float[N];
		float c[]=new float[N];
		
		for(int i=0;i<N;i++)
		{
			a[i]=i;
			b[i]=i;
		}
		VectorAdd_AutoPar(N,a,b,c);
		
		int flag=1;
		for(int i=0;i<N;i++)
		{
			if(Math.abs(c[i]-(a[i]+b[i]))>0.000001)
			{
				flag=0;
				System.out.println("Test Faild!!!");
				break;
			}
		}
		if(flag==1) System.out.println("Test Successful!!!");
	}
}
