#version 330 core
	
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VOUT {
	vec3 vColor;
	vec3 vOffset;
	flat highp int vFace;
	flat highp int vDepth;
} vin[];

out vec2 txCoord;
out vec3 fColor;	
flat out highp int fFace;


uniform mat4 MVP;
uniform bool Debug;

// For use in calculating ST coordinates
int wl = (0x01 << (3 - vin[0].vDepth));
float segl = 1.0 / (wl * 1.0);

vec2 getST(vec4 pos, int face)
{
	if (Debug) {
		segl = 1.0;
	}

	pos = (pos + vec4(0.5, 0.5, 0.5, 1.0)) * segl;

	switch (face) {
		case 0:
			pos.y = (1.0 * segl) - pos.y;
			return vec2(pos.x, pos.y);
		case 1:
			pos.x = (1.0 * segl) - pos.x;
			pos.y = (1.0 * segl) - pos.y;
			return vec2(pos.x, pos.y);
		case 2:
			pos.y = (1.0 * segl) - pos.y;
			return vec2(pos.z, pos.y);
		case 3:
			pos.y = (1.0 * segl) - pos.y;
			pos.z = (1.0 * segl) - pos.z;
			return vec2(pos.z, pos.y);
		case 4:
			pos.z = (1.0 * segl) - pos.z;
		case 5:
			return vec2(pos.x, pos.z);
	}
}

vec2 getTxS(vec4 pos, int face)
{
	//pos = pos + vec4(0.5, 0.5, 0.5, 1.0);
	float s0 = mod(pos.x, wl) * segl + (1.0 * segl);
	float s1 = mod(pos.x, wl) * segl;
	// For left/right we need to use the y coordinate instead
	float y0 = mod(pos.y, wl) * segl + (1.0 * segl);
	float y1 = mod(pos.y, wl) * segl;

	if (Debug) {
		s0 = 1.0;
		s1 = 0.0;
		y0 = s0;
		y1 = s1;
	}

	switch (face) {
		case 0:
			return vec2(s1, s0);
		case 1:
			return vec2(s0, s1);
		case 2:
			return vec2(y1, y0);
		case 3:
			return vec2(y0, y1);
		case 4:
		case 5:
			return vec2(s1, s0);
	}

}

vec2 getTxT(vec4 pos, int face)
{
	pos = pos + vec4(0.5, 0.5, 0.5, 1.0);
	float t0 = mod(pos.y, wl) * segl + (1.0 * segl);
	float t1 = mod(pos.y, wl) * segl;
	// For top/bottom/left/right we need to use the z coordinate instead
	float z0 = mod(pos.z, wl) * segl + (1.0 * segl);
	float z1 = mod(pos.z, wl) * segl;

	if (Debug) {
		t0 = 1.0;
		t1 = 0.0;
		z0 = t0;
		z1 = t1;
	}

	switch (face) {
		case 0:
		case 1:
			return vec2(t0, t1);
		case 2:
		case 3:
			return vec2(z0, z1);
		case 4:
			return vec2(z0, z1);
		case 5:
			return vec2(z1, z0);
	}
}

void main()
{
	// Color is unnecessary but useful for debugging, can be removed in the final product
	fColor = vin[0].vColor;
	//fColor = vec3(1.0, 0.5, 0.5)
	
	float x = vin[0].vOffset.x;
	float y = vin[0].vOffset.y;
	float z = vin[0].vOffset.z;

	// Calculate our ST coordinates bound to our tile depth size
	//vec2 Scoord = getTxS(gl_in[0].gl_Position, vin[0].vFace);
	//vec2 Tcoord = getTxT(gl_in[0].gl_Position, vin[0].vFace);

	vec4 position;
	// TODO: I'm sure there's a better way to check which coordinates to use
	if (y > 0.0) {

		

		// Calculate the position of the vetex to be emitted
		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);
		vec2 Scoord = getTxS(position, vin[0].vFace);
		vec2 Tcoord = getTxT(position, vin[0].vFace);
		// Translate against MVP matrix
		gl_Position = MVP * position;
		// Set our ST coords (x/y just refer to vector position 0/1, not literally x/y coords)
		//txCoord = vec2(Scoord.x, Tcoord.x);
		txCoord = getST(position, vin[0].vFace);
		// Tell the frag shader what face we are (and what texture to use)
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		// Rinse, repeat
		position = gl_in[0].gl_Position + vec4( x, -y, z, 0.0);
		Scoord = getTxS(position, vin[0].vFace);
		Tcoord = getTxT(position, vin[0].vFace);
		gl_Position = MVP * position;
		//txCoord = vec2(Scoord.y, Tcoord.x);
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, -z, 0.0);
		Scoord = getTxS(position, vin[0].vFace);
		Tcoord = getTxT(position, vin[0].vFace);
		gl_Position = MVP * position;
		//txCoord = vec2(Scoord.x, Tcoord.y);
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		Scoord = getTxS(position, vin[0].vFace);
		Tcoord = getTxT(position, vin[0].vFace);
		gl_Position = MVP * position;
		//txCoord = vec2(Scoord.y, Tcoord.y);
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex();

	} else {

		//TODO: The only difference in this branch is we have -z -z z z instead of -z z -z z in our offsets
		// Can we consolidate this somehow?

		position = gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0);
		vec2 Scoord = getTxS(position, vin[0].vFace);
		vec2 Tcoord = getTxT(position, vin[0].vFace);
		gl_Position = MVP * position;
		//txCoord = vec2(Scoord.x, Tcoord.x);
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4( x, -y, -z, 0.0);
		Scoord = getTxS(position, vin[0].vFace);
		Tcoord = getTxT(position, vin[0].vFace);
		gl_Position = MVP * position;
		//txCoord = vec2(Scoord.y, Tcoord.x);
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
					  			 
		position = gl_in[0].gl_Position + vec4(-x,  y, z, 0.0);
		Scoord = getTxS(position, vin[0].vFace);
		Tcoord = getTxT(position, vin[0].vFace);
		gl_Position = MVP * position;
		//txCoord = vec2(Scoord.x, Tcoord.y);
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex(); 			 
								 
		position = gl_in[0].gl_Position + vec4( x,  y, z, 0.0);
		Scoord = getTxS(position, vin[0].vFace);
		Tcoord = getTxT(position, vin[0].vFace);
		gl_Position = MVP * position;
		//txCoord = vec2(Scoord.y, Tcoord.y);
		txCoord = getST(position, vin[0].vFace);
		fFace = vin[0].vFace;
		EmitVertex();
	}

	EndPrimitive();
}