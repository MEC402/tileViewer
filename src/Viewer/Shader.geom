#version 330 core
	
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vColor[];
in vec3 vOffset[];

out vec3 fColor;

uniform mat4 MVP;
uniform float Scaling;
uniform float RotationAngle;

// TODO: Extract this and push it through as a uniform
mat4 rotationMatrix(vec3 axis, float angle) {
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}


void main()
{
	fColor = vColor[0];
	
	mat4 rotMat = rotationMatrix(vec3(0.0,1.0,1.0), RotationAngle);

	vec4 position = MVP * (gl_in[0].gl_Position * rotMat);

	float x = vOffset[0].x;
	float y = vOffset[0].y;
	float z = vOffset[0].z;

	//TODO: Need to figure out the pos/neg for z values for top/bottom/left/right tiles
	//gl_Position = position + Scaling * (rotMat*(vec4(-x, -y, z, 0.0)));
	//EmitVertex(); 			 
	//			  			 
	//gl_Position = position + Scaling * (rotMat*(vec4( x, -y, z, 0.0)));
	//EmitVertex(); 			 
	//			  			 
	//gl_Position = position + Scaling * (rotMat*(vec4(-x,  y, z, 0.0)));
	//EmitVertex(); 			 
	//						 
	//gl_Position = position + Scaling * (rotMat*(vec4( x,  y, z, 0.0)));
	//EmitVertex();

	gl_Position = position + vec4(-x, -y, z, 0.0);
       EmitVertex(); 
				  
       gl_Position = position + vec4( x, -y, z, 0.0);
       EmitVertex(); 						  
				  						  
       gl_Position = position + vec4(-x,  y, z, 0.0);
       EmitVertex(); 						  
				  						  
       gl_Position = position + vec4( x,  y, z, 0.0);
       EmitVertex();

	EndPrimitive();
}