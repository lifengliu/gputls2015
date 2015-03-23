package model;

public class ForLoop {
	String initializer; // like int i = 0;
	String boolExpression;  // like i < 50; 
	String update; // like i++;
	String loopBody;
	public String getInitializer() {
		return initializer;
	}
	public void setInitializer(String initializer) {
		this.initializer = initializer;
	}
	public String getBoolExpression() {
		return boolExpression;
	}
	public void setBoolExpression(String boolExpression) {
		this.boolExpression = boolExpression;
	}
	public String getUpdate() {
		return update;
	}
	public void setUpdate(String update) {
		this.update = update;
	}
	public String getLoopBody() {
		return loopBody;
	}
	public void setLoopBody(String loopBody) {
		this.loopBody = loopBody;
	}
	@Override
	public String toString() {
		return "ForLoop [initializer=" + initializer + ", boolExpression=" + boolExpression + ", update=" + update
				+ ", loopBody=" + loopBody + "]";
	}
	
}
