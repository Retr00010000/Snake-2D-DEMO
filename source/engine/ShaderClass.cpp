#include "shaderClass.h"

// Reads a text file and outputs a string with everything in the text file
std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(static_cast<size_t>(in.tellg()));
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}



// Constructor that build the Shader Program from 2 different shaders
Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	// Read vertexFile and fragmentFile and store the strings
	std::string vertexCode = get_file_contents(vertexFile);
	std::string fragmentCode = get_file_contents(fragmentFile);

	// Convert the shader source strings into character arrays
	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	// Create Vertex Shader Object and get its reference
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// Attach Vertex Shader source to the Vertex Shader Object
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	// Compile the Vertex Shader into machine code
	glCompileShader(vertexShader);
	// Checks if Shader compiled succesfully
	compileErrors(vertexShader, "VERTEX");

	// Create Fragment Shader Object and get its reference
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// Attach Fragment Shader source to the Fragment Shader Object
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	// Compile the Vertex Shader into machine code
	glCompileShader(fragmentShader);
	// Checks if Shader compiled succesfully
	compileErrors(fragmentShader, "FRAGMENT");

	// Create Shader Program Object and get its reference
	ID = glCreateProgram();
	// Attach the Vertex and Fragment Shaders to the Shader Program
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	// Wrap-up/Link all the shaders together into the Shader Program
	glLinkProgram(ID);
	// Checks if Shaders linked succesfully
	compileErrors(ID, "PROGRAM");

	// Delete the now useless Vertex and Fragment Shader objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}
// Activates the Shader Program
void Shader::Activate()
{
	glUseProgram(ID);
}

// Deletes the Shader Program
void Shader::Delete()
{
	glDeleteProgram(ID);
}

// Checks if the different Shaders have compiled properly
void Shader::compileErrors(unsigned int shader, const char* type)
{
	// Stores status of compilation
	GLint hasCompiled;
	// Character array to store error message in
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
		if (hasCompiled == GL_FALSE)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
		}
	}
}

/* Steps on how a shader program is created in OpenGL:

1. File Reading (C++ Filesystem I/O)

The constructor opens "default.vert" and "default.frag" from your disk using standard C++ file streams (std::ifstream)
and reads their entire text contents into two C++ string variables:

vertexCode (contains the raw GLSL text of default.vert)
fragmentCode (contains the raw GLSL text of default.frag)


2. Creating GPU Shader Containers (glCreateShader).

OpenGL creates two blank shader slots on your GPU and gives back temporary integer IDs:

GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);


3. Feeding Text Code to GPU (glShaderSource).

The C++ text strings are passed to the GPU driver memory:

const char* vertexSource = vertexCode.c_str();
const char* fragmentSource = fragmentCode.c_str();

glShaderSource(vertexShader, 1, &vertexSource, NULL);
glShaderSource(fragmentShader, 1, &fragmentSource, NULL);


4. Compiling GLSL to GPU Machine Code (glCompileShader).

The graphics driver's internal compiler compiles the text strings into binary GPU instructions:

glCompileShader(vertexShader);
glCompileShader(fragmentShader);


5. Creating the Unified Shader Program (glCreateProgram).

OpenGL creates a master program object that will hold both shaders together and assigns its ID to your member variable ID (i.e. shaderProgram.ID):

ID = glCreateProgram();

6. Attaching and Linking (glAttachShader & glLinkProgram).


The compiled vertex shader and fragment shader are linked together into one executable pipeline:

glAttachShader(ID, vertexShader);
glAttachShader(ID, fragmentShader);
glLinkProgram(ID);

The linking phase: OpenGL checks that out vec3 color; in default.vert matches in vec3 color; in default.frag.

7. Deleting Intermediate Objects (glDeleteShader).

Now that the shaders are linked inside the master program ID, the individual intermediate shader objects are cleaned up:

glDeleteShader(vertexShader);
glDeleteShader(fragmentShader);

*/
