#version 330 core

in vec3 fColor;
in vec2 txCoord;
flat in highp int fFace;
out vec4 outColor;

uniform sampler2D TxFront;
uniform sampler2D TxBack;
uniform sampler2D TxRight;
uniform sampler2D TxLeft;
uniform sampler2D TxTop;
uniform sampler2D TxBottom;

void main() 
{
	switch (fFace) {
		case 0:
			outColor = texture(TxFront, txCoord);
			break;
		case 1:
			outColor = texture(TxBack, txCoord);
			break;
		case 2:
			outColor = texture(TxRight, txCoord);
			break;
		case 3:
			outColor = texture(TxLeft, txCoord);
			break;
		case 4:
			outColor = texture(TxTop, txCoord);
			break;
		case 5:
			outColor = texture(TxBottom, txCoord);
			break;
	}
	//if (fFace <= 0.2) {
	//	outColor = texture(TxFront, txCoord)	* vec4(fColor, 1.0);
	//} else if (fFace <= 1.2) {					
	//	outColor = texture(TxBack, txCoord)		* vec4(fColor, 1.0);
	//} else if (fFace <= 2.2) {					
	//	outColor = texture(TxRight, txCoord)	* vec4(fColor, 1.0);
	//} else if (fFace <= 3.2) {					
	//	outColor = texture(TxLeft, txCoord)		* vec4(fColor, 1.0);
	//} else if (fFace <= 4.2) {					
	//	outColor = texture(TxTop, txCoord)		* vec4(fColor, 1.0);
	//} else if (fFace <= 5.2) {					
	//	outColor = texture(TxBottom, txCoord)	* vec4(fColor, 1.0);
	//} else {
	//	outColor = vec4(1.0, 0.0, 0.0, 1.0);
	//}
}