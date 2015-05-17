package controller;

import jaforloop.parser.JaForLoopBaseVisitor;
import jaforloop.parser.JaForLoopParser;
import jaforloop.parser.JaForLoopParser.ExpressionContext;
import jaforloop.parser.JaForLoopParser.StatementContext;

import java.util.List;

import model.JaArray;
import model.JaBlockContext;
import model.JaBranchStructure;
import codegen.util.JaArrayUtils;

public class JaVisitor extends JaForLoopBaseVisitor<Object> {
	public JaBlockContext jaBlockContext = new JaBlockContext();
	
	private int currentLayer = 0;
	
	@Override 
	public Object visitForloop(JaForLoopParser.ForloopContext ctx) {
		currentLayer ++;
		visit(ctx.block());
		currentLayer --;
		return null;
	}
	
	@Override
	public Object visitBlock(JaForLoopParser.BlockContext ctx) {
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
		} else if (ctx.expression() != null) { // function call
		} else if (ctx.block() != null) { // if
			
			visit(ctx.expression()); // if condition
			
			//visit(ctx.block(0));
			
			//visit(ctx.block(1));
		}
		
		return null;
	}
	
	@Override 
	public Object visitExpression(JaForLoopParser.ExpressionContext ctx) {
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
		if (ctx.accessibleVar() != null) {
			visit(ctx.accessibleVar());
		} else if (ctx.expression() != null) {
			visit(ctx.expression());
		}
		return null;
	}
	
	@Override
	public Object visitLeftvalue(JaForLoopParser.LeftvalueContext ctx) {
		if (ctx.expression() != null) { // it means it is an array
			JaArray a = JaArrayUtils.getJaArrayFromLeftvalue(ctx);
			jaBlockContext.combine(a);
			visit(ctx.expression());
		}
		
		return null;
	}
	
	@Override
	public Object visitAccessibleVar(JaForLoopParser.AccessibleVarContext ctx) {
		if (currentLayer > 0) {
			if (ctx.expression() != null) { // it is an array
				JaArray jaArray = JaArrayUtils.getJaArrayFromAccessibleVar(ctx);
				jaBlockContext.combine(jaArray);
				visit(ctx.expression());
			}
		}
		
		return null;
	}
}









