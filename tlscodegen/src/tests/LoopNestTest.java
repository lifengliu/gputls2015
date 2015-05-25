package tests;

import jaforloop.parser.JaForLoopLexer;
import jaforloop.parser.JaForLoopParser;

import java.util.Map;

import model.JaArray;
import model.JaForLoop;

import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.tree.ParseTree;
import org.testng.annotations.Test;

import codegen.SimpleReadnWriteAnalyzer;
import controller.JaVisitor;

public class LoopNestTest {
	
	@Test
	public void testLoopNest() throws Exception {
		System.out.println("testLoopNest() \n\n");
		JaForLoopLexer lexer = new JaForLoopLexer(new ANTLRFileStream("resources/code_example/nest1.forloop"));
		CommonTokenStream tokens = new CommonTokenStream(lexer);
		JaForLoopParser parser = new JaForLoopParser(tokens);
		ParseTree pt = parser.forloop();

		JaVisitor visitor = new JaVisitor();
		visitor.visit(pt);

		JaForLoop fl = visitor.getRootForLoop();
		SimpleReadnWriteAnalyzer ana = new SimpleReadnWriteAnalyzer();
		
		Map<String, JaArray> arrayInfoMap = ana.getArrayInfos(fl.getBlockContext());

		for (Map.Entry<String, JaArray> e : arrayInfoMap.entrySet()) {
			System.out.println(e.getValue());
		}

		System.out.println("\n\n finish testLoopNest() \n\n");
	}
	
}






