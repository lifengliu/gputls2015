package controller;

import jaforloop.parser.JaForLoopBaseVisitor;
import jaforloop.parser.JaForLoopParser;
import jaforloop.parser.JaForLoopParser.ExpressionContext;
import jaforloop.parser.JaForLoopParser.StatementContext;

import java.util.ArrayDeque;
import java.util.Deque;
import java.util.List;

import model.JaArray;
import model.JaBlockContext;
import model.JaBranchStructure;
import codegen.util.JaArrayUtils;

public class JaVisitor extends JaForLoopBaseVisitor<Object> {
	private Deque<JaBlockContext> blockContextStack = new ArrayDeque<> ();
	
	public JaVisitor(JaBlockContext blockContext) {
		super();
		blockContextStack.push(blockContext);
	}
	
	private JaBlockContext getCurrentBlockContext() {
		return blockContextStack.getFirst();
	}
	
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
		} else if (ctx.block() != null && ctx.block().size() > 0) { // if
			calcFromBranchStructure(ctx);
		} else if (ctx.expression() != null) { // function call
			System.out.println(ctx.expression().getText());
		}
		
		return null;
	}

	private void calcFromBranchStructure(JaForLoopParser.StatementContext ctx) {
		visit(ctx.expression()); // if condition
		//if
		
		JaBranchStructure jbs = new JaBranchStructure();
		JaBlockContext trueBranchContext = new JaBlockContext(getCurrentBlockContext());
		blockContextStack.push(trueBranchContext);
		visit(ctx.block(0));
		blockContextStack.pop();
		jbs.trueBranch = trueBranchContext;
		
		if (ctx.block().size() > 1) { // else
			JaBlockContext falseBranchContext = new JaBlockContext(getCurrentBlockContext());
			blockContextStack.push(falseBranchContext);
			visit(ctx.block(1));
			blockContextStack.pop();
			jbs.falseBranch = falseBranchContext;
		}
		
		getCurrentBlockContext().addStatement(jbs);
		
		//TODO
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
			getCurrentBlockContext().combine(a);
			visit(ctx.expression());
		}
		
		return null;
	}
	
	@Override
	public Object visitAccessibleVar(JaForLoopParser.AccessibleVarContext ctx) {
		if (currentLayer > 0) {
			if (ctx.expression() != null) { // it is an array
				JaArray jaArray = JaArrayUtils.getJaArrayFromAccessibleVar(ctx);
				getCurrentBlockContext().combine(jaArray);
				visit(ctx.expression());
			}
		}
		
		return null;
	}
}









