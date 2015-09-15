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
import model.JaForLoop;
import model.JaFunctionCall;
import model.JaStatement;
import codegen.util.JaArrayUtils;

public class JaVisitor extends JaForLoopBaseVisitor<Object> {
	
	private Deque<JaBlockContext> blockContextStack = new ArrayDeque<> ();
	private Deque<JaForLoop> forloopStack = new ArrayDeque<> ();
	
	public JaVisitor() {
		super();
		JaForLoop fl = new JaForLoop();
		fl.initEmptyBlock();
		forloopStack.push(fl);
		blockContextStack.push(fl.getBlockContext());
	}
	
	private JaBlockContext getCurrentBlockContext() {
		return blockContextStack.getFirst();
	}
	
	private JaForLoop getCurrentForLoop() {
		return forloopStack.getFirst();
	}
	
	public JaForLoop getRootForLoop() {
		if (forloopStack.size() != 1) {
			throw new RuntimeException("inconsistent state");
		} else {
			return forloopStack.getFirst();
		}
	}
	
	private int currentLayer = 0;
	
	private int globalStatementId = 0;
	
	@Override 
	public Object visitForloop(JaForLoopParser.ForloopContext ctx) {
		currentLayer ++;
		visit(ctx.forControl());
		visit(ctx.block());
		currentLayer --;
		return null;
	}

	
	@Override
	public Object visitForControl(JaForLoopParser.ForControlContext ctx) {
		
		String forInitStr = ctx.forInit().getText();
		String forCondStr = ctx.expression(0).getText(); 
		String forUpdateStr = ctx.expression(1).getText();
		
		getCurrentForLoop().initializer = forInitStr;
		getCurrentForLoop().boolExpression = forCondStr;
		getCurrentForLoop().update = forUpdateStr;
		
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
		
		} else if (ctx.forloop() != null) { //for loop
		
			JaForLoop fl = new JaForLoop(getCurrentForLoop());
			getCurrentBlockContext().addStatement(fl);
			
			JaBlockContext forblockContext = new JaBlockContext(getCurrentBlockContext());
			fl.setLoopBlockContext(forblockContext);
			
			forloopStack.push(fl);
			blockContextStack.push(fl.getBlockContext());
			
			visit(ctx.forloop());
			
			forloopStack.pop();
			blockContextStack.pop();
			
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









