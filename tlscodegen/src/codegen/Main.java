package codegen;

import jaforloop.parser.JaForLoopLexer;
import jaforloop.parser.JaForLoopParser;

import java.io.IOException;

import model.JaBlockContext;
import model.JaBranchStructure;
import model.JaStatement;

import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.RecognitionException;
import org.antlr.v4.runtime.tree.ParseTree;

import controller.JaVisitor;

public class Main {
	
    public static void main(String... args) throws NoSuchFieldException, IllegalAccessException, IOException, RecognitionException {
    	JaForLoopLexer lexer = new JaForLoopLexer(new ANTLRFileStream("resources/code_example/2.forloop"));    	
    	CommonTokenStream tokens = new CommonTokenStream(lexer);
    	JaForLoopParser parser = new JaForLoopParser(tokens);
    	ParseTree pt = parser.forloop();
    	
    	JaBlockContext ctx = new JaBlockContext(null); //root
    	JaVisitor visitor = new JaVisitor(ctx);
    	visitor.visit(pt);
    	
    	ctx.printAllArrays();
    	
    	System.out.println(" ------------------------------ ");
    	
    	for (JaStatement s : ctx.statementList) {
    		if (s instanceof JaBranchStructure) {
    			((JaBranchStructure) s).trueBranch.printAllArrays();
    			System.out.println(" f ");
    			((JaBranchStructure) s).falseBranch.printAllArrays();
    		}
    	}
    }

}

