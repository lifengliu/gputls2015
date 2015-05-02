package model;

import java.util.Map;
import java.util.TreeMap;

public class JaBlockContext {
	private Map<String, JaArray> arrayInfo = new TreeMap<String, JaArray> ();
	
	public JaArray getJaArray(String identifier) {
		return arrayInfo.get(identifier);
	}
	
	/**
	 * put the jaArray netly generated to existed one
	 * or create a new one into arrayInfo
	 * @param jaArray
	 * @return
	 */
	public JaArray combine(JaArray jaArray) {
		if (arrayInfo.containsKey(jaArray.identifier)) { //combine
			JaArray jaOld = arrayInfo.get(jaArray.identifier);
			if (jaOld.dimension != jaArray.dimension ) {
				throw new RuntimeException("array " + jaOld.identifier + " dimension is inconsistent");
			}
			
			jaOld.accessType = jaOld.accessType | jaArray.accessType;
			
			jaOld.writeExpressions.addAll(jaArray.writeExpressions); 
			jaOld.readExpressions.addAll(jaArray.readExpressions); 
			
		} else { // add new
			arrayInfo.put(jaArray.identifier, jaArray);
		}
		
		return arrayInfo.get(jaArray.identifier);
	}
	
	
	public void printAllArrays() {
		for (Map.Entry<String, JaArray> entry : arrayInfo.entrySet()) {
		    System.out.println(entry.getValue());
		}
		
	}
	
}
