package codegen;

import java.util.Map;

import model.JaArray;
import model.JaBlockContext;
import model.JaBranchStructure;
import model.JaStatement;

public class SimpleReadnWriteAnalyzer {
	
	private JaBlockContext allArrayBlock = new JaBlockContext(null);
	
	private void traverseBlocks(JaBlockContext currentBlock) {
		for (Map.Entry<String, JaArray> e : currentBlock.getArrayInfoMap().entrySet()) {
			allArrayBlock.combineIn(e.getValue());
		}
		
		for (JaStatement js : currentBlock.statementList) {
			if (js instanceof JaBranchStructure) {
				JaBranchStructure jbs = (JaBranchStructure) js;
				traverseBlocks(jbs.trueBranch);
				traverseBlocks(jbs.falseBranch);
			}
		}
	}
	
	public Map<String, JaArray> getArrayInfos(JaBlockContext jbc) {
		traverseBlocks(jbc);
		return allArrayBlock.getArrayInfoMap();	
	}
	
}
