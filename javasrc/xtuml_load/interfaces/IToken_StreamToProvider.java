package xtuml_load.interfaces;

import org.xtuml.bp.core.ComponentInstance_c;

public interface IToken_StreamToProvider {
	public void Value(ComponentInstance_c c, String Value);

	public void Identifier(ComponentInstance_c c, String Value);

	public void Comment(ComponentInstance_c c, String Value);

	public void End_Of_Stream(ComponentInstance_c c);

	public void Semicolon(ComponentInstance_c c);

	public void Comma(ComponentInstance_c c);

	public void Left_Parenthesis(ComponentInstance_c c);

	public void Right_Parenthesis(ComponentInstance_c c);
}
