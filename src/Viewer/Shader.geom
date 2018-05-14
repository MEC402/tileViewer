#version 330 core
	
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vColor[];
in vec3 vOffset[];

out vec3 fColor;

uniform mat4 MVP;
uniform float Scaling;

void main()
{
	fColor = vColor[0];
	
	//mat4 rotMat = rotationMatrix(vec3(0.0,1.0,1.0), RotationAngle);

	//vec4 position = MVP * (gl_in[0].gl_Position * rotMat);
	//vec4 position = MVP * gl_in[0].gl_Position

	float x = vOffset[0].x;
	float y = vOffset[0].y;
	float z = vOffset[0].z;

	//TODO: Need to figure out the pos/neg for z values for top/bottom/left/right tiles
	if (y > 0.0) {
		gl_Position = MVP * (gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0));
		EmitVertex(); 			 
					  			 
		gl_Position = MVP * (gl_in[0].gl_Position + vec4( x, -y, z, 0.0));
		EmitVertex(); 			 
					  			 
		gl_Position = MVP * (gl_in[0].gl_Position + vec4(-x,  y, -z, 0.0));
		EmitVertex(); 			 
								 
		gl_Position = MVP * (gl_in[0].gl_Position + vec4( x,  y, z, 0.0));
		EmitVertex();
	} else {
		gl_Position = MVP * (gl_in[0].gl_Position + vec4(-x, -y, -z, 0.0));
		EmitVertex(); 			 
					  			 
		gl_Position = MVP * (gl_in[0].gl_Position + vec4( x, -y, -z, 0.0));
		EmitVertex(); 			 
					  			 
		gl_Position = MVP * (gl_in[0].gl_Position + vec4(-x,  y, z, 0.0));
		EmitVertex(); 			 
								 
		gl_Position = MVP * (gl_in[0].gl_Position + vec4( x,  y, z, 0.0));
		EmitVertex();
	}

	//gl_Position = position + vec4(-x, -y, z, 0.0);
    //EmitVertex(); 
	//  
    //gl_Position = position + vec4( x, -y, z, 0.0);
    //EmitVertex(); 						  
	//  						  
    //gl_Position = position + vec4(-x,  y, z, 0.0);
    //EmitVertex(); 						  
	//  						  
    //gl_Position = position + vec4( x,  y, z, 0.0);
    //EmitVertex();

	EndPrimitive();
}