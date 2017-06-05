package xtuml_load.interfaces;

import org.xtuml.bp.core.ComponentInstance_c;

public interface ICharacter_StreamToProvider {
	public void Text(ComponentInstance_c c, String Value);

	public void File(ComponentInstance_c c, String Filename);
}
