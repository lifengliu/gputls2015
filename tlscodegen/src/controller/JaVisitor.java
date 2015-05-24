package controller;

import jaforloop.parser.JaForLoopBaseVisitor;
import jaforloop.parser.JaForLoopParser;
import jaforloop.parser.JaForLoopParser.ExpressionContext;
import jaforloop.parser.JaForLoopParser.StatementContext;

import java.util.ArrayDeque;
import java.util.Deque;
import java.util.List;

import model.JaArray;
import model.JaAssignStatement;
import model.JaBlockContext;
import model.JaBranchStructure;
import model.JaFunctionCall;
import model.JaStatement;
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
	private int globalStatementId = 0;
	
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
		globalStatementId++;
		
		if (ctx.leftvalue() != null) { // an assign statement
			JaStatement stat = new JaAssignStatement();
			stat.setId(globalStatementId);
			stat.setStatement(ctx.getText());
			getCurrentBlockContext().addStatement(stat);
			
			visit(ctx.leftvalue());
			visit(ctx.expression());
		} else if (ctx.block() != null && ctx.block().size() > 0) { // if
			JaBranchStructure branchStructureStat = new JaBranchStructure();
			branchStructureStat.setId(globalStatementId);
			branchStructureStat.setStatement(ctx.getText());
			calcFromBranchStructure(ctx, branchStructureStat);
		} else if (ctx.expression() != null) { // function call
			JaStatement stat = new JaFunctionCall();
			stat.setId(globalStatementId);
			stat.setStatement(ctx.expression().getText());
			getCurrentBlockContext().addStatement(stat);
		}
		
		return null;
	}

	private void calcFromBranchStructure(JaForLoopParser.StatementContext ctx, JaBranchStructure jbs) {
		visit(ctx.expression()); // if condition

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
			getCurrentBlockContext().combineIn(a);
			visit(ctx.expression());
		}
		
		return null;
	}
	
	@Override
	public Object visitAccessibleVar(JaForLoopParser.AccessibleVarContext ctx) {
		if (currentLayer > 0) {
			if (ctx.expression() != null) { // it is an array
				JaArray jaArray = JaArrayUtils.getJaArrayFromAccessibleVar(ctx);
				getCurrentBlockContext().combineIn(jaArray);
				visit(ctx.expression());
			}
		}
		
		return null;
	}
}









