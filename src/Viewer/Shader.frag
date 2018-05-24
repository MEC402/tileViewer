#version 330 core

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
}