package model;

public class JaStatement {
	private int id; //
	
	private int blockId; // order within a block
	
	protected String statement;
	
	public int getId() {
		return id;
	}
	
	public void setId(int id) {
		this.id = id;
	}
	
	public int getBlockId() {
		return blockId;
	}
	
	public void setBlockId(int blockId) {
		this.blockId = blockId;
	}
	
	public String getStatement() {
		return statement;
	}
	
	public void setStatement(String statement) {
		this.statement = statement;
	}
	
	
}
