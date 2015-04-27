// Generated from JaForLoop.g4 by ANTLR 4.5
package jaforloop.parser;
import org.antlr.v4.runtime.misc.NotNull;
import org.antlr.v4.runtime.tree.ParseTreeVisitor;

/**
 * This interface defines a complete generic visitor for a parse tree produced
 * by {@link JaForLoopParser}.
 *
 * @param <T> The return type of the visit operation. Use {@link Void} for
 * operations with no return type.
 */
public interface JaForLoopVisitor<T> extends ParseTreeVisitor<T> {
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#forloop}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitForloop(JaForLoopParser.ForloopContext ctx);
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#forControl}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitForControl(JaForLoopParser.ForControlContext ctx);
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#forInit}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitForInit(JaForLoopParser.ForInitContext ctx);
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#variableDeclarator}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitVariableDeclarator(JaForLoopParser.VariableDeclaratorContext ctx);
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#block}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitBlock(JaForLoopParser.BlockContext ctx);
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#statement}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitStatement(JaForLoopParser.StatementContext ctx);
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#expression}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitExpression(JaForLoopParser.ExpressionContext ctx);
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#expressionList}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitExpressionList(JaForLoopParser.ExpressionListContext ctx);
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#primary}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPrimary(JaForLoopParser.PrimaryContext ctx);
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#leftvalue}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitLeftvalue(JaForLoopParser.LeftvalueContext ctx);
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#primitiveType}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitPrimitiveType(JaForLoopParser.PrimitiveTypeContext ctx);
	/**
	 * Visit a parse tree produced by {@link JaForLoopParser#literal}.
	 * @param ctx the parse tree
	 * @return the visitor result
	 */
	T visitLiteral(JaForLoopParser.LiteralContext ctx);
}