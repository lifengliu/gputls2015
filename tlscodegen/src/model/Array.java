package model;

import java.util.List;

public class Array {
	private int dimension;
	private String name;
	private List<Expression> readIndexExprs;
	private List<Expression> writeIndexExprs;
	private List<Statement> refferedStatement;
	
	public int getDimension() {
		return dimension;
	}
	
	public void setDimension(int dimension) {
		this.dimension = dimension;
	}
	
	public String getName() {
		return name;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	public List<Expression> getReadIndexExprs() {
		return readIndexExprs;
	}
	
	public void setReadIndexExprs(List<Expression> readIndexExprs) {
		this.readIndexExprs = readIndexExprs;
	}
	
	public List<Expression> getWriteIndexExprs() {
		return writeIndexExprs;
	}
	
	public void setWriteIndexExprs(List<Expression> writeIndexExprs) {
		this.writeIndexExprs = writeIndexExprs;
	}
	
	public List<Statement> getRefferedStatement() {
		return refferedStatement;
	}
	
	public void setRefferedStatement(List<Statement> refferedStatement) {
		this.refferedStatement = refferedStatement;
	}
	
	
}
