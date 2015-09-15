package tests;

import org.stringtemplate.v4.ST;
import org.stringtemplate.v4.STGroup;
import org.stringtemplate.v4.STGroupDir;
import org.testng.annotations.Test;

public class STTest {

	@Test
	public void testStringTemplate() {
		ST hello = new ST("Hello, <name>");
		hello.add("name", "World");
		System.out.println(hello.render());
	}

	@Test
	public void testST2() {
		STGroup group = new STGroupDir("resources/codegen_template");
		ST st = group.getInstanceOf("decl");
		st.add("type", "int");
		st.add("name", "x");
		st.add("value", 0);
		String result = st.render();
		System.out.println(result);
	}

	@Test
	public void genMulti() {
		int[] num = new int[] { 3, 9, 20, 2, 1, 4, 6, 32, 5, 6, 77, 888, 2, 1, 6, 32, 5, 6, 77, 4, 9, 20, 2, 1, 4, 63,
				9, 20, 2, 1, 4, 6, 32, 5, 6, 77, 6, 32, 5, 6, 77, 3, 9, 20, 2, 1, 4, 6, 32, 5, 6, 77, 888, 1, 6, 32, 5 };

		String t = ST.format(30, "int <%1>[] = { <%2; wrap, anchor, separator=\", \"> };", "a", num);
		System.out.println(t);
	}

	
	@Test
	public void genMulti2() {
		System.out.println("test multi 2");
		String[] gege = new String[] {"ruozhi","nuecai","zenmeban","gegege","aoao" };

		STGroup group = new STGroupDir("resources/codegen_template");
		ST st = group.getInstanceOf("testStrArr");
		
		st.add("v", "gegege");
		st.add("cont", gege);
		
		System.out.println(st.render());
		
		System.out.println("test multi 2 end");
		
	}

}
