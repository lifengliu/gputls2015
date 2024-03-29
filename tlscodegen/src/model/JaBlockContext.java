package model;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

public class JaBlockContext {
	
	//private Set<String> iterationDomainVariables = new HashSet<String> (); // interation domain of a block
	
	public JaBlockContext(JaBlockContext parent) {
		this.parent = parent;
	}
	
	/*public void setCurrentIterationDomain(String blockIterDomVar) {
		iterationDomainVariables.add(blockIterDomVar);
	}*/
	
	public JaBlockContext parent = null;
	
	public List<JaStatement> statementList = new ArrayList<JaStatement> ();
	
	private Map<String, JaArray> arrayInfo = new TreeMap<String, JaArray> ();
	
	public void addStatement(JaStatement stat) {
		if (stat == null) {
			return;
		}
		stat.setBlockId(statementList.size()+1);
		statementList.add(stat);
	}
	
	/**
	 * get JaArray corresponding to the identifier, 
	 * return null when id is not existent
	 * @param identifier
	 * @return
	 */
	public JaArray getJaArray(String identifier) {
		return arrayInfo.get(identifier);
	}
	
	public Map<String, JaArray> getArrayInfoMap() {
		return Collections.unmodifiableMap(arrayInfo);
	}
	
	/**
	 * put the jaArray netly generated to existed one
	 * or create a new one into arrayInfo
	 * @param jaArray
	 * @return
	 */
	public JaArray combineIn(JaArray jaArray) {
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
