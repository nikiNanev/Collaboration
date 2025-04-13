//> all
#version 450

layout (location = 0) out vec3 outColor;

void main() 
{
	//const array of positions for the triangles
	const vec3 positions[12] = vec3[12](
		vec3(-0.25f, -0.25f, 0.0f), //bottom left
		vec3(0.25f, -0.25f, 0.0f), //bottom right
		vec3(0.0f, 0.0f, 0.0f), //top

		vec3(0.25f, 0.25f, 0.0f),
		vec3(-0.25f, 0.25f, 0.0f), //top left
		vec3(0.0f, 0.0f, 0.0f), //top right

		vec3(-0.25f, 0.25f, 0.0f), //top left
		vec3(0.0f, 0.0f, 0.0f),
		vec3(-0.25f, -0.25f, 0.0f), //bottom left

		vec3(0.25f, -0.25f, 0.0f), //bottom right
		vec3(0.0f, 0.0f, 0.0f), //top
		vec3(0.25f, 0.25f, 0.0f) //top right
	);

	//const array of colors for the triangle
	const vec3 colors[12] = vec3[12](
		vec3(0.0f, 1.0f, 0.0f), //green
		vec3(0.0f, 1.0f, 0.0f), //green
		vec3(0.f, 1.0f, 0.0f),  //green

		vec3(1.0f, 0.0f, 0.0f), //red
		vec3(1.0f, 0.0f, 0.0f), //red
		vec3(1.0f, 0.0f, 0.0f),  //red

		vec3(0.0f, 0.0f, 1.0f), //blue
		vec3(0.0f, 0.0f, 1.0f), //blue
		vec3(0.0f, 0.0f, 1.0f), //blue

		vec3(1.0f, 1.0f, 0.0f), //yellow
		vec3(1.0f, 1.0f, 0.0f), //yellow
		vec3(1.0f, 1.0f, 0.0f)  //yellow
	);

	//output the position of each vertex
	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
	outColor = colors[gl_VertexIndex];
}
//< all