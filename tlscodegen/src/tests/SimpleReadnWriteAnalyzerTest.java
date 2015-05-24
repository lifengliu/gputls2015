package tests;

import jaforloop.parser.JaForLoopLexer;
import jaforloop.parser.JaForLoopParser;

import java.util.Map;

import model.JaArray;
import model.JaBlockContext;

import org.antlr.v4.runtime.ANTLRFileStream;
import org.antlr.v4.runtime.CommonTokenStream;
import org.antlr.v4.runtime.tree.ParseTree;
import org.testng.annotations.Test;

import codegen.SimpleReadnWriteAnalyzer;
import controller.JaVisitor;

public class SimpleReadnWriteAnalyzerTest {

	@Test
	public void simpleRnWTest1() throws Exception {
		JaForLoopLexer lexer = new JaForLoopLexer(new ANTLRFileStream("resources/code_example/1.forloop"));
		CommonTokenStream tokens = new CommonTokenStream(lexer);
		JaForLoopParser parser = new JaForLoopParser(tokens);
		ParseTree pt = parser.forloop();

		JaBlockContext ctx = new JaBlockContext(null); // root
		JaVisitor visitor = new JaVisitor(ctx);
		visitor.visit(pt);

		SimpleReadnWriteAnalyzer ana = new SimpleReadnWriteAnalyzer();

		Map<String, JaArray> arrayInfoMap = ana.getArrayInfos(ctx);

		for (Map.Entry<String, JaArray> e : arrayInfoMap.entrySet()) {
			System.out.println(e.getValue());
		}

	}

	@Test
	public void simpleRnWTest2() throws Exception {
		JaForLoopLexer lexer = new JaForLoopLexer(new ANTLRFileStream("resources/code_example/2.forloop"));
		CommonTokenStream tokens = new CommonTokenStream(lexer);
		JaForLoopParser parser = new JaForLoopParser(tokens);
		ParseTree pt = parser.forloop();

		JaBlockContext ctx = new JaBlockContext(null); // root
		JaVisitor visitor = new JaVisitor(ctx);
		visitor.visit(pt);

		SimpleReadnWriteAnalyzer ana = new SimpleReadnWriteAnalyzer();

		Map<String, JaArray> arrayInfoMap = ana.getArrayInfos(ctx);

		for (Map.Entry<String, JaArray> e : arrayInfoMap.entrySet()) {
			System.out.println(e.getValue());
		}

	}



}
