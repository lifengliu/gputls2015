package model;

public class JaForLoop extends JaStatement {
	
	private JaForLoop parent;
	
	public String initializer; // like int i = 0;

	public String boolExpression;  // like i < 50; 
	
	public String update; // like i++;
	
	public String loopBody; // block 
	
	private JaBlockContext loopBlockContext; 
	
	public String domainVariable;

	public void initEmptyBlock() {
		loopBlockContext = new JaBlockContext(null);
	}
	
	public JaBlockContext getBlockContext() {
		if (loopBlockContext == null) {
			throw new RuntimeException("loop block context has not been initiated");
		} else {
			return loopBlockContext;
		}
	}
	
	public JaForLoop() {
		
	}
	
	public JaForLoop(JaForLoop parent) {
		this.parent = parent;
	}

	public JaForLoop getParent() {
		return parent;
	}

	public void setParent(JaForLoop parent) {
		this.parent = parent;
	}

	public JaBlockContext getLoopBlockContext() {
		return loopBlockContext;
	}

	public void setLoopBlockContext(JaBlockContext loopBlockContext) {
		this.loopBlockContext = loopBlockContext;
	}
	
	
}


