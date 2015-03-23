package codegen;

import java.io.IOException;

import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.RecognitionException;
import org.antlr.v4.runtime.tree.ParseTree;

import antlr4gen.JavaLexer;
import antlr4gen.JavaParser;
import controller.GenVisitor;

public class Main {
	
    public static void main(String... args) throws NoSuchFieldException, IllegalAccessException, IOException, RecognitionException {
    	JavaLexer lexer = new JavaLexer(new ANTLRFileStream("resources/code_example/VectorAdd.java"));    	
    	CommonTokenStream tokens = new CommonTokenStream(lexer);
    	JavaParser parser = new JavaParser(tokens);
    	ParseTree pt = parser.compilationUnit();
    	GenVisitor visitor = new GenVisitor();
    	visitor.visit(pt);
    	System.out.println(visitor.floop);
    }

}

