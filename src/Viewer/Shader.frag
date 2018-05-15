#version 330 core

in vec3 fColor;
in vec2 txCoord;
in float fFace;
out vec4 outColor;

uniform sampler2D TxFront;
uniform sampler2D TxBack;
uniform sampler2D TxLeft;
uniform sampler2D TxRight;
uniform sampler2D TxTop;
uniform sampler2D TxBottom;

void main() 
{
	if (fFace <= 0.2) {
			outColor = texture(TxFront, txCoord);
	} else if (fFace <= 1.2) {
			outColor = texture(TxBack, txCoord);
	} else if (fFace <= 2.2) {
		outColor = texture(TxRight, txCoord);
	} else if (fFace <= 3.2) {
		outColor = texture(TxLeft, txCoord);
	} else if (fFace <= 4.2) {
		outColor = texture(TxTop, txCoord);
	} else if (fFace <= 5.2) {
		outColor = texture(TxBottom, txCoord);
	} else {
		outColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
}