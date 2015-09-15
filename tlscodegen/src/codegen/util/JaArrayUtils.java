package codegen.util;

import jaforloop.parser.JaForLoopParser;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import model.JaArray;

public class JaArrayUtils {
	public static JaArray getJaArrayFromAccessibleVar(JaForLoopParser.AccessibleVarContext ctx) {
		
		JaArray jaArray = new JaArray();
		
		JaForLoopParser.AccessibleVarContext tmpAccessibleVarContext = ctx;
		List<String> hierarchyList = new ArrayList<String> ();
		List<String> dimensionExpressions = new ArrayList<String> ();
		while (tmpAccessibleVarContext != null) {
			hierarchyList.add(tmpAccessibleVarContext.getText());
			if (tmpAccessibleVarContext.expression() != null) {
				dimensionExpressions.add(tmpAccessibleVarContext.expression().getText());
			}
			tmpAccessibleVarContext = tmpAccessibleVarContext.accessibleVar();
		}
		
		String arrayName = hierarchyList.get(hierarchyList.size() - 1);
		int dimension = dimensionExpressions.size();
		
		if (jaArray.dimension != 0 && dimension != jaArray.dimension) {
			throw new RuntimeException("array " + jaArray.identifier + " dimentsion is inconsistent");
		}
		
		jaArray.identifier = arrayName;
		jaArray.accessType = JaArray.READ_ONLY;
		jaArray.dimension = dimension;
		
		Map<Integer, String> readExpression = new HashMap<> ();
		
		for (int i = 0; i < dimensionExpressions.size(); i++) {
			String exp = dimensionExpressions.get(i);
			int dim = dimensionExpressions.size() - i;
			readExpression.put(dim, exp);
		}
		
		jaArray.readExpressions.add(readExpression);		
		
		return jaArray;
	}


	public static JaArray getJaArrayFromLeftvalue(JaForLoopParser.LeftvalueContext ctx) {
		JaArray jaArray = new JaArray();
		
		JaForLoopParser.LeftvalueContext tmpLeftvalueCtx = ctx;
		List<String> hierarchyList = new ArrayList<String> ();
		List<String> dimensionExpressions = new ArrayList<String> ();
		
		while (tmpLeftvalueCtx != null) {
			hierarchyList.add(tmpLeftvalueCtx.getText());
			
			if (tmpLeftvalueCtx.expression() != null) {
				dimensionExpressions.add(tmpLeftvalueCtx.expression().getText());
			}
			
			tmpLeftvalueCtx = tmpLeftvalueCtx.leftvalue();
		}
	
		
		String arrayName = hierarchyList.get(hierarchyList.size() - 1);
		int dimension = dimensionExpressions.size();
		
		if (jaArray.dimension != 0 && dimension != jaArray.dimension) {
			throw new RuntimeException("array " + jaArray.identifier + " dimentsion is inconsistent");
		}
		
		jaArray.identifier = arrayName;
		jaArray.accessType = JaArray.WRITE_ONLY;
		jaArray.dimension = dimension;
		
		Map<Integer, String> writeExpression = new HashMap<> ();
		
		for (int i = 0; i < dimensionExpressions.size(); i++) {
			String exp = dimensionExpressions.get(i);
			int dim = dimensionExpressions.size() - i;
			
			writeExpression.put(dim, exp);
		}
		
		jaArray.writeExpressions.add(writeExpression);		
		
		return jaArray;
	}
	
}
