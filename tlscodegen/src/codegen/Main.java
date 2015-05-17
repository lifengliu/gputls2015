package codegen;

import jaforloop.parser.JaForLoopLexer;
import jaforloop.parser.JaForLoopParser;

import java.io.IOException;

import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.RecognitionException;
import org.antlr.v4.runtime.tree.ParseTree;

import controller.JaVisitor;

public class Main {
	
    public static void main(String... args) throws NoSuchFieldException, IllegalAccessException, IOException, RecognitionException {
    	JaForLoopLexer lexer = new JaForLoopLexer(new ANTLRFileStream("resources/code_example/1.forloop"));    	
    	CommonTokenStream tokens = new CommonTokenStream(lexer);
    	JaForLoopParser parser = new JaForLoopParser(tokens);
    	ParseTree pt = parser.forloop();
    	JaVisitor visitor = new JaVisitor();
    	visitor.visit(pt);
    	visitor.jaBlockContext.printAllArrays();
    }

}

