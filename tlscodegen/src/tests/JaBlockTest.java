package tests;

import jaforloop.parser.JaForLoopLexer;
import jaforloop.parser.JaForLoopParser;
import model.JaAssignStatement;
import model.JaBlockContext;
import model.JaBranchStructure;
import model.JaFunctionCall;
import model.JaStatement;

import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.tree.ParseTree;
import org.testng.annotations.Test;

import controller.JaVisitor;

public class JaBlockTest {
	@Test
	public void testArrayRefs() throws Exception {
    	JaForLoopLexer lexer = new JaForLoopLexer(new ANTLRFileStream("resources/code_example/2.forloop"));    	
    	CommonTokenStream tokens = new CommonTokenStream(lexer);
    	JaForLoopParser parser = new JaForLoopParser(tokens);
    	ParseTree pt = parser.forloop();
    	
    	JaBlockContext ctx = new JaBlockContext(null); //root
    	JaVisitor visitor = new JaVisitor(ctx);
    	visitor.visit(pt);
    	
    	ctx.printAllArrays();
    	
    	
    	for (JaStatement s : ctx.statementList) {
    		if (s instanceof JaBranchStructure) {
    			JaBranchStructure jbs = (JaBranchStructure) s;
    			jbs.trueBranch.printAllArrays();
    			jbs.falseBranch.printAllArrays();
    		} else if (s instanceof JaFunctionCall) {
    			
    		} else if (s instanceof JaAssignStatement) {
    			
    		}
    	}
	}
	
	private void traverseStatementInBlock(JaBlockContext jbc) {
		for (JaStatement s : jbc.statementList) {
    		System.out.println(s.getId() + "  " + s.getBlockId());
    		System.out.println(s.getStatement());
    		
			if (s instanceof JaBranchStructure) {
				traverseStatementInBlock(((JaBranchStructure) s).trueBranch);
				traverseStatementInBlock(((JaBranchStructure) s).falseBranch);
			}
		}
	}
	
	@Test
	public void testRecordedStatements() throws Exception {
    	JaForLoopLexer lexer = new JaForLoopLexer(new ANTLRFileStream("resources/code_example/2.forloop"));    	
    	CommonTokenStream tokens = new CommonTokenStream(lexer);
    	JaForLoopParser parser = new JaForLoopParser(tokens);
    	ParseTree pt = parser.forloop();
    	
    	JaBlockContext ctx = new JaBlockContext(null); //root
    	JaVisitor visitor = new JaVisitor(ctx);
    	visitor.visit(pt);
    	
    	traverseStatementInBlock(ctx);
	}
	
	
}



