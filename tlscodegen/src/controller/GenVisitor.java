package controller;

import model.ForLoop;
import antlr4gen.JavaBaseVisitor;
import antlr4gen.JavaParser;

public class GenVisitor extends JavaBaseVisitor<Object> {
	public ForLoop floop = new ForLoop();
	
	@Override 
	public Object visitComments(JavaParser.CommentsContext ctx) {
		String commentText = ctx.getText();
		
		if (commentText.trim().startsWith("/*par")) { // 当前节点之后一个节点就是for loop
			return true;
		}
		
		return false;
	}
	
	@Override
	public Object visitForloop(JavaParser.ForloopContext ctx) {
		if ((Boolean) visit(ctx.comments())) {
			visit(ctx.forControl());
			floop.setLoopBody(ctx.statement().getText());
			visit(ctx.statement());
		}
		
		return null;
	}
	
	@Override
	public Object visitForControl(JavaParser.ForControlContext ctx) {
		visit(ctx.forInit());
		String boolExpression = ctx.expression().getText();
		floop.setBoolExpression(boolExpression);
		
		String forUpdate = ctx.forUpdate().getText();
		floop.setUpdate(forUpdate);
		
		return null;
	}
	
	
}
