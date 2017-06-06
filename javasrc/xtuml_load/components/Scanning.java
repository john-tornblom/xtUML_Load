package xtuml_load.components;

import org.xtuml.bp.core.ComponentInstance_c;
import org.xtuml.bp.core.CorePlugin;

import xtuml_load.interfaces.ICharacter_StreamFromProvider;
import xtuml_load.interfaces.ICharacter_StreamToProvider;
import xtuml_load.interfaces.IToken_StreamFromProvider;
import xtuml_load.interfaces.IToken_StreamToProvider;
import lib.LOG;

import java.io.ByteArrayInputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

public class Scanning implements ICharacter_StreamToProvider, IToken_StreamFromProvider {

	private IToken_StreamToProvider output = null;
	private InputStream in = null;
	private int LA = ' ';

	public Scanning(ICharacter_StreamFromProvider Input, IToken_StreamToProvider Output) {
		this.output = Output;
	}

	@Override
	public void Text(ComponentInstance_c senderReceiver, String Value) {
		LOG.LogInfo("Tokenizing text...");

		in = new ByteArrayInputStream(Value.getBytes());
		process();
	}

	@Override
	public void File(ComponentInstance_c senderReceiver, String Filename) {
		LOG.LogInfo("Tokenizing file...");

		try {
			in = new FileInputStream(Filename);
			process();
		} catch (FileNotFoundException e) {
			e.printStackTrace(CorePlugin.err);
		}

	}

	private void process() {
		LA = ' ';
		while (LA >= 0) {
			next();
		}

		try {
			in.close();
		} catch (IOException e) {
			e.printStackTrace(CorePlugin.err);
		}
	}

	private char scan() {
		char ch = (char) LA;

		try {
			LA = in.read();
		} catch (IOException e) {
			e.printStackTrace(CorePlugin.err);
			LA = -1;
		}

		return ch;
	}

	private boolean next() {
		String value = "";

		if (LA < 0) {
			return false;
		}

		while (Character.isWhitespace(LA)) {
			scan();
		}

		while (LA == '\'') {
			do {
				value += scan();
			} while (LA != '\'');
			value += scan();
		}

		if (value.length() > 0) {
			// String value
			output.Value(null, value);
			return true;
		}

		while (LA == '"') {
			do {
				value += scan();
			} while (LA != '"');
			value += scan();
		}

		if (value.length() > 0) {
			// UUID value
			output.Value(null, value);
			return true;
		}

		if (LA == ',') {
			value += scan();
			output.Comma(null);
			return true;
		}

		if (LA == '(') {
			value += scan();
			output.Left_Parenthesis(null);
			return true;
		}

		if (LA == ')') {
			value += scan();
			output.Right_Parenthesis(null);
			return true;
		}

		if (LA == ';') {
			value += scan();
			output.Semicolon(null);
			return true;
		}

		if (LA == '-') {
			value += scan();
			if (LA == '-') {
				while (LA != '\n' && LA != '\r') {
					value += scan();
				}
				output.Comment(null, value);
				return true;
			}
			if (!Character.isDigit(LA)) {
				return false;
			}
			value += "-";
		}

		if (Character.isDigit(LA)) {
			while (Character.isDigit(LA) || LA == '.') {
				value += scan();
			}
			if (value.contains(".")) {
				// Real value
				output.Value(null, value);
			} else {
				// Integer value
				output.Value(null, value);
			}
		}

		if (Character.isLetter(LA) || LA == '_') {
			while (Character.isLetterOrDigit(LA) || LA == '_') {
				value += scan();
			}
			output.Identifier(null, value);
		}

		return value.length() > 0;
	}
}
