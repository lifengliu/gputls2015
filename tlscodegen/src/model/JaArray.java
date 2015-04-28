package model;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;


public class JaArray {

	public int dimension;
	
	public String identifier;
	
	public int accessType;
	
	public List<Map<Integer, String>> writeExpressions = new ArrayList<Map<Integer,String>> ();
	public List<Map<Integer, String>> readExpressions = new ArrayList<> ();
	
	public static final int READ_ONLY = 1, WRITE_ONLY = 2, READ_AND_WRITE = 3;

	@Override
	public String toString() {
		return "JaArray [dimension=" + dimension + ", identifier=" + identifier + ", accessType=" + accessType
				+ ", writeExpressions=" + writeExpressions + ", readExpressions=" + readExpressions + "]";
	}
	
}



