package codegen;

import java.io.IOException;

import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.ParserRuleContext;
import org.antlr.v4.runtime.RecognitionException;
import org.antlr.v4.runtime.misc.NotNull;
import org.antlr.v4.runtime.tree.ParseTree;
import org.antlr.v4.runtime.tree.ParseTreeWalker;

import antlr4gen.JavaBaseListener;
import antlr4gen.JavaLexer;
import antlr4gen.JavaParser;

public class Main {
	
    public static void main(String... args) throws NoSuchFieldException, IllegalAccessException, IOException, RecognitionException {
    	JavaLexer lexer = new JavaLexer(new ANTLRFileStream("resources/code_example/VectorAdd.java"));    	
    	CommonTokenStream tokens = new CommonTokenStream(lexer);
    	JavaParser parser = new JavaParser(tokens);
    	
    	ParserRuleContext tree = parser.compilationUnit(); // parse
    	ParseTreeWalker walker = new ParseTreeWalker(); // create standard walker
    	
    	JavaBaseListener listener = new JavaBaseListener() {

    		@Override public void enterForUpdate(@NotNull JavaParser.ForUpdateContext ctx) { 
    			System.out.println("enter for update");
    		}
    		
    		@Override public void enterComments(@NotNull JavaParser.CommentsContext ctx) {
    			String commentText = ctx.getText();
				System.out.println("enter comments" + commentText);
    			
				
				if (commentText.trim().startsWith("/*par")) { // 当前节点之后一个节点就是for loop
					
					ParserRuleContext parent = ctx.getParent();
					
					int cnt = parent.getChildCount();
					
	    			for (int i = 0;i < cnt; i++) {
	    				ParseTree child = parent.getChild(i);
	    				
						System.out.println(i + " " + child.getText());
	    			}
    			}
    			
    			
    			
    		}
    		
    	};
    	
    	walker.walk(listener, tree);
    	
    	
    	 
    }
    
    

}

