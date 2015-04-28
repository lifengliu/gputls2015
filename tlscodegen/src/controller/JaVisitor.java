package controller;

import jaforloop.parser.JaForLoopBaseVisitor;
import jaforloop.parser.JaForLoopParser;
import jaforloop.parser.JaForLoopParser.ExpressionContext;
import jaforloop.parser.JaForLoopParser.StatementContext;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import model.JaArray;

public class JaVisitor extends JaForLoopBaseVisitor<Object> {
	
	Map<String, JaArray> arrayInfo;
	
	
	{
		arrayInfo = new HashMap<String, JaArray> ();
	}
	
	private int currentLayer = 0;
	
	@Override 
	public Object visitForloop(JaForLoopParser.ForloopContext ctx) {
		System.out.println("visit for loop");
		//visit(ctx.forControl());
		currentLayer ++;
		visit(ctx.block());
		currentLayer --;
		return null;
	}
	
	@Override
	public Object visitBlock(JaForLoopParser.BlockContext ctx) {
		System.out.println("for block" + ctx.getText());
		List<StatementContext> statements = ctx.statement();
		for (StatementContext sCtx : statements) {
			visit(sCtx);
		}
		return null;
	}
	
	@Override
	public Object visitStatement(JaForLoopParser.StatementContext ctx) {
		if (ctx.leftvalue() != null) { // an assign statement
			visit(ctx.leftvalue());
			visit(ctx.expression());
			//System.out.println(ctx.expression().getText());
		} else if (ctx.expression() != null) { // function call
			//visit(ctx.expression());
		}
		
		return null;
	}
	
	@Override 
	public Object visitExpression(JaForLoopParser.ExpressionContext ctx) {
		//System.out.println("expression = " + ctx.getText());
		if (ctx.primary() != null) {
			visit(ctx.primary());
		} else {
			List<ExpressionContext> expressions = ctx.expression();
			for (ExpressionContext expCtx : expressions) {
				visit(expCtx);
			}
		}
		
		return null;
	}
	
	@Override
	public Object visitPrimary(JaForLoopParser.PrimaryContext ctx) {
		ctx.accessibleVar();
		if (ctx.accessibleVar() != null) {
			visit(ctx.accessibleVar());
		} else if (ctx.expression() != null) {
			visit(ctx.expression());
		}
		return null;
	}
	
	@Override
	public Object visitLeftvalue(JaForLoopParser.LeftvalueContext ctx) {
		//System.out.println("this is a left value");
		
		if (ctx.expression() != null) { // it means it is an array
			
			JaArray a = getJaArrayFromLeftvalue(ctx);
			System.out.println(loopUpnCombine(a));
			
		}
		
		return null;
	}
	
	@Override
	public Object visitAccessibleVar(JaForLoopParser.AccessibleVarContext ctx) {
		if (currentLayer > 0) {
			System.out.println("accessible var   " + ctx.getText());

			if (ctx.expression() != null) { // it is an array
				JaArray jaArray = getJaArrayFromAccessibleVar(ctx);
				System.out.println(loopUpnCombine(jaArray));
				
			}
		}
		
		
		return null;
	}
	
	
	private JaArray getJaArrayFromAccessibleVar(JaForLoopParser.AccessibleVarContext ctx) {
		
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

	private JaArray loopUpnCombine(JaArray jaArray) {
		if (arrayInfo.containsKey(jaArray.identifier)) {
			JaArray jaOld = arrayInfo.get(jaArray.identifier);
			if (jaOld.dimension != jaArray.dimension ) {
				throw new RuntimeException("array " + jaOld.identifier + " dimension is inconsistent");
			}
			
			jaOld.accessType = jaOld.accessType | jaArray.accessType;
			
			jaOld.writeExpressions.addAll(jaArray.writeExpressions);
			jaOld.readExpressions.addAll(jaArray.readExpressions);
			
		} // combine
		else {
			arrayInfo.put(jaArray.identifier, jaArray);
		}
		
		return arrayInfo.get(jaArray.identifier);
	}
	
	private JaArray getJaArrayFromLeftvalue(JaForLoopParser.LeftvalueContext ctx) {
		JaArray jaArray = new JaArray();
		
		JaForLoopParser.LeftvalueContext tmpLeftvalueCtx = ctx;
		List<String> hierarchyList = new ArrayList<String> ();
		List<String> dimensionExpressions = new ArrayList<String> ();
		
		while (tmpLeftvalueCtx != null) {
			hierarchyList.add(tmpLeftvalueCtx.getText());
			
			if (tmpLeftvalueCtx.expression() != null) {
				//System.out.println(tmpLeftvalueCtx.expression().getText());
				dimensionExpressions.add(tmpLeftvalueCtx.expression().getText());
			}
			
			//System.out.println(tmpLeftvalueCtx.getText());
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









