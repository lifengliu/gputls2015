// Generated from JaForLoop.g4 by ANTLR 4.5
package jaforloop.parser;
import org.antlr.v4.runtime.misc.NotNull;
import org.antlr.v4.runtime.tree.ParseTreeListener;

/**
 * This interface defines a complete listener for a parse tree produced by
 * {@link JaForLoopParser}.
 */
public interface JaForLoopListener extends ParseTreeListener {
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#forloop}.
	 * @param ctx the parse tree
	 */
	void enterForloop(JaForLoopParser.ForloopContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#forloop}.
	 * @param ctx the parse tree
	 */
	void exitForloop(JaForLoopParser.ForloopContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#forControl}.
	 * @param ctx the parse tree
	 */
	void enterForControl(JaForLoopParser.ForControlContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#forControl}.
	 * @param ctx the parse tree
	 */
	void exitForControl(JaForLoopParser.ForControlContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#forInit}.
	 * @param ctx the parse tree
	 */
	void enterForInit(JaForLoopParser.ForInitContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#forInit}.
	 * @param ctx the parse tree
	 */
	void exitForInit(JaForLoopParser.ForInitContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#variableDeclarator}.
	 * @param ctx the parse tree
	 */
	void enterVariableDeclarator(JaForLoopParser.VariableDeclaratorContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#variableDeclarator}.
	 * @param ctx the parse tree
	 */
	void exitVariableDeclarator(JaForLoopParser.VariableDeclaratorContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#block}.
	 * @param ctx the parse tree
	 */
	void enterBlock(JaForLoopParser.BlockContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#block}.
	 * @param ctx the parse tree
	 */
	void exitBlock(JaForLoopParser.BlockContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#statement}.
	 * @param ctx the parse tree
	 */
	void enterStatement(JaForLoopParser.StatementContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#statement}.
	 * @param ctx the parse tree
	 */
	void exitStatement(JaForLoopParser.StatementContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#expression}.
	 * @param ctx the parse tree
	 */
	void enterExpression(JaForLoopParser.ExpressionContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#expression}.
	 * @param ctx the parse tree
	 */
	void exitExpression(JaForLoopParser.ExpressionContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#expressionList}.
	 * @param ctx the parse tree
	 */
	void enterExpressionList(JaForLoopParser.ExpressionListContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#expressionList}.
	 * @param ctx the parse tree
	 */
	void exitExpressionList(JaForLoopParser.ExpressionListContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#primary}.
	 * @param ctx the parse tree
	 */
	void enterPrimary(JaForLoopParser.PrimaryContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#primary}.
	 * @param ctx the parse tree
	 */
	void exitPrimary(JaForLoopParser.PrimaryContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#accessibleVar}.
	 * @param ctx the parse tree
	 */
	void enterAccessibleVar(JaForLoopParser.AccessibleVarContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#accessibleVar}.
	 * @param ctx the parse tree
	 */
	void exitAccessibleVar(JaForLoopParser.AccessibleVarContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#leftvalue}.
	 * @param ctx the parse tree
	 */
	void enterLeftvalue(JaForLoopParser.LeftvalueContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#leftvalue}.
	 * @param ctx the parse tree
	 */
	void exitLeftvalue(JaForLoopParser.LeftvalueContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#primitiveType}.
	 * @param ctx the parse tree
	 */
	void enterPrimitiveType(JaForLoopParser.PrimitiveTypeContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#primitiveType}.
	 * @param ctx the parse tree
	 */
	void exitPrimitiveType(JaForLoopParser.PrimitiveTypeContext ctx);
	/**
	 * Enter a parse tree produced by {@link JaForLoopParser#literal}.
	 * @param ctx the parse tree
	 */
	void enterLiteral(JaForLoopParser.LiteralContext ctx);
	/**
	 * Exit a parse tree produced by {@link JaForLoopParser#literal}.
	 * @param ctx the parse tree
	 */
	void exitLiteral(JaForLoopParser.LiteralContext ctx);
}