/*package controller;

import model.ForLoop;
import antlr4gen.JavaBaseVisitor;
import antlr4gen.JavaParser;

public class GenVisitor extends JavaBaseVisitor<Object> {
	public ForLoop floop = new ForLoop();
	
	int loopLayer = 0;
	@Override 
	public Object visitComments(JavaParser.CommentsContext ctx) {
		System.out.println("visit Comments");
		String commentText = ctx.getText();
		
		if (commentText.trim().startsWith("/*par")) { // 当前节点之后一个节点就是for loop
			
			return true;
		}
		return false;
	}
	
	@Override
	public Object visitForloop(JavaParser.ForloopContext ctx) {
		System.out.println("visitForLoop");
		if ((Boolean) visit(ctx.comments())) {
			loopLayer = 1;
			visit(ctx.forControl());
			floop.setLoopBody(ctx.block().getText());
			visit(ctx.block());
			loopLayer = 0;
		}
		return null;
	}
	
	@Override
	public Object visitForControl(JavaParser.ForControlContext ctx) {
		System.out.println("visitForControl");
		//visit(ctx.forInit());
		String boolExpression = ctx.expression().getText();
		floop.setBoolExpression(boolExpression);
		
		String forUpdate = ctx.forUpdate().getText();
		floop.setUpdate(forUpdate);
		
		return null;
	}
	
	@Override 
	public Object visitBlock(JavaParser.BlockContext ctx) {
		System.out.println("visitBlock");
		if (loopLayer > 0) {
			System.out.println(ctx.getText());
		}
		
		return visitChildren(ctx); 
	}
	
	
}
*/