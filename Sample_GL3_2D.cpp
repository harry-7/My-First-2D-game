#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <bits/stdc++.h>
#include <unistd.h>
#include <glad/glad.h>
#include <FTGL/ftgl.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>

#define XMAX 560
#define XMIN -560
#define YMAX 260
#define YMIN -260

#define SHOOT 1
#define SHOOT1 2
#define airfric 0.2/15
#define X_COLLIDE 1
#define Y_COLLIDE 2
#define XY_COLLIDE 3
#define NO_COLLIDE 0
#define sqr(a) a*a
#define GRAVITY 1
#define NB 6
#define MAXL 150
#define NC 8
#define out1(x)cout<<#x<<" is "<<x<<endl
#define out2(x,y)cout<<#x<<" is "<<x<<" "<<#y <<" is "<<y<<endl
#define out3(x,y,z)cout<<#x<<" is "<<x<<" "<<#y<<" is "<<y<<" "<<#z<<" is "<<z<<endl;
#define out4(a,b,c,d)cout<<#a<<" is "<<a<<" "<<#b<<"  is "<<b<<" "<<#c<<" is "<<c<<" "<<#d<<" is "<<d<<endl;

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;
	GLuint TextureBuffer;
	GLuint TextureID;

	GLenum PrimitiveMode; // GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY
	GLenum FillMode; // GL_FILL, GL_LINE
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID; // For use with normal shader
	GLuint TexMatrixID; // For use with texture shader
} Matrices;

struct FTGLFont {
	FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
} GL3Font;

GLuint programID, fontProgramID, textureProgramID;

typedef struct COLOR{
	float r;
	float g;
	float b;
} COLOR;

typedef struct object{
	VAO *obj;
	string id;
	COLOR color;
	double x_vel;
	double y_vel;
	double x;
	double y;
	double radius_x;
	double radius_y;
	double gravity;
	double angle;
	double fric;
	bool inair;
	bool onground;
	double radius;
	bool to_rot;
	bool is_possible;
} object;
float score = 0;

map<string,object> mymap;
double MAXV = 20;

int window_width = 1200;
int window_height = 600;

int limits[] = {20,20,19,18,16,14,11,10,7};
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	cout << "Compiling shader : " <<  vertex_file_path << endl;
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	cout << VertexShaderErrorMessage.data() << endl;

	// Compile Fragment Shader
	cout << "Compiling shader : " << fragment_file_path << endl;
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	cout << FragmentShaderErrorMessage.data() << endl;

	// Link the program
	cout << "Linking program" << endl;
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	cout << ProgramErrorMessage.data() << endl;

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	cout << "Error: " << description << endl;
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

glm::vec3 getRGBfromHue (int hue)
{
	float intp;
	float fracp = modff(hue/60.0, &intp);
	float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

	if (hue < 60)
		return glm::vec3(1,x,0);
	else if (hue < 120)
		return glm::vec3(x,1,0);
	else if (hue < 180)
		return glm::vec3(0,1,x);
	else if (hue < 240)
		return glm::vec3(0,x,1);
	else if (hue < 300)
		return glm::vec3(x,0,1);
	else
		return glm::vec3(1,0,x);
}

int done = 0;
/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;
	vao->TextureID = textureID;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
						  0,                  // attribute 0. Vertices
						  3,                  // size (x,y,z)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
	glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
						  2,                  // attribute 2. Textures
						  2,                  // size (s,t)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	return vao;
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	if(vao == NULL)return;
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void draw3DTexturedObject (struct VAO* vao)
{
	if(vao == NULL)return;
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Bind Textures using texture units
	glBindTexture(GL_TEXTURE_2D, vao->TextureID);

	// Enable Vertex Attribute 2 - Texture
	glEnableVertexAttribArray(2);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

	// Unbind Textures to be safe
	glBindTexture(GL_TEXTURE_2D, 0);
}

/* Create an OpenGL Texture from an image */
GLuint createTexture (const char* filename)
{
	GLuint TextureID;
	// Generate Texture Buffer
	glGenTextures(1, &TextureID);
	// All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
	glBindTexture(GL_TEXTURE_2D, TextureID);
	// Set our texture parameters
	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering (interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load image and create OpenGL texture
	int twidth, theight;
	unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
	SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

	return TextureID;
}



/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
double orig_x_speed = 3.0f,orig_y_speed = 3.0;
double startt;
bool todraw = 0;
int cnt;
int stt = 0;
int keyst = 0;
int cntdif = 0;
int st1 = 0,state,s1=0;
int pst = 0;
float zoom_camera = 1,offset = 0;
int number_of_balls = 0;
int levels,limit_balls;

float get_angle(GLFWwindow* window){
	double xp,yp;
	float x,y,vy,vx;
	glfwGetCursorPos(window, &xp, &yp);
	xp-=600;
	yp-=300;
	yp=-yp;
	x = mymap["canon"].x;
	y = mymap["canon"].y;
	//out2(xp-x,yp-y);
	return (atan2(yp-y,xp-x)/M_PI)*180;;
}
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.
	if (action == GLFW_RELEASE) {
		keyst = 0;
		switch (key) {
			case GLFW_KEY_W:
				pst = 0;
				MAXV+=5;
				MAXV = min(MAXV,50.0);
				break;
			case GLFW_KEY_S:
				pst = 0;
				MAXV-=5;
				MAXV = max(MAXV,10.0);
				break;
			case GLFW_KEY_R:
				if(done == 3){
					levels --;
				}
				done = 2;
				break;
			case GLFW_KEY_N:
				if(done == 3){
					done = 2;
				}
				break;
			case GLFW_KEY_DOWN:
				zoom_camera /= 1.1; //make it bigger than current size
				zoom_camera = max(0.5f,zoom_camera);
				break;
			case GLFW_KEY_UP:
				zoom_camera *= 1.1; //make it bigger than current size
				zoom_camera = min(5.0f,zoom_camera);
				break;
			case GLFW_KEY_B:
				if(done!=0){
					if(levels > 1)levels--;
					if(done == 3)levels--;
					levels = max(levels,1);
					done = 2;
				}
				break;
			case GLFW_KEY_RIGHT:
				if(zoom_camera!=1||offset!=0){
				offset+=5;
				offset = max(offset,40.0f);
				}
				break;
			case GLFW_KEY_LEFT:
				if(zoom_camera!=1||offset!=0){
				offset-=5;
				offset = max(offset,-40.0f);
			}
				break;
			default:
				break;
		}
	}
	else if (action == GLFW_PRESS) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			case GLFW_KEY_U:
				keyst = 1;
				break;
			case GLFW_KEY_W:
				pst = 1;
				break;
			case GLFW_KEY_S:
				pst = -1;
				break;
			case GLFW_KEY_D:
				keyst = 2;
				break;
			case GLFW_KEY_SPACE:
				keyst = 0;
				todraw = 1;
				mymap["ball"].inair = 1;
				float angle = mymap["rod"].angle*M_PI/180.0;
				float vx = MAXV*cos(angle);
				float vy = MAXV*sin(angle);
				vy = min(vy - 3,26.0f);
				if(vy > 7) vx = min(vx,30.0f);
				else vx = min(vx,38.0f);
				vx = max(0.0f,vx);
				mymap["ball"].x_vel = vx;
				mymap["ball"].y_vel = vy;
				mymap["ball"].x = mymap["canon"].x+(2*(mymap["rod"].radius_x))*cos(angle);
				mymap["ball"].y = mymap["canon"].y+(2*(mymap["rod"].radius_x))*sin(angle);
				for(int i = 0;i < 4; i++){
					string s = "rectangle";
					s+=('0'+i+1);
					mymap[s].is_possible = 1;

				}
				cntdif = 20;
				stt = 0;
				s1 =1;
				cnt = 0;
				state = 2;
				st1 = 0;
				number_of_balls++;
				break;

		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}

int sign(float x){
	if(x <0)return -1;
	else return 1;
}

void mousescroll(GLFWwindow* window, double xoffset, double yoffset){

	if (yoffset==-1) { 
		zoom_camera /= 1.1; //make it bigger than current size
		zoom_camera = max(0.5f,zoom_camera);
	}
	else if(yoffset==1){
		zoom_camera *= 1.1; //make it bigger than current size
		zoom_camera = min(5.0f,zoom_camera);
	}

}

/* Executed when a mouse button is pressed/released */

void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_RELEASE){
				if (s1==1)return;
				//if(keyst != 0)return;
				todraw = 1;
				double xp,yp;
				float x,y,vy,vx;
				glfwGetCursorPos(window, &xp, &yp);
				xp-=600;
				yp-=300;
				yp=-yp;
				x = mymap["canon"].x;
				y = mymap["canon"].y;
				vy = min((yp-y)/15 - 3.0,26.0);
				if(vy > 7) vx = min((xp-x)/15 -3,30.0);
				else vx = min((xp-x)/15,38.0);
				//out2(vx,vy);
				mymap["ball"].inair = 1;
				float angle = (get_angle(window)*M_PI)/180.0;
				vx = max(0.0f,vx);
				mymap["ball"].x_vel = vx;
				mymap["ball"].y_vel = vy;
				mymap["ball"].x = mymap["canon"].x+(2*(mymap["rod"].radius_x))*cos(angle);
				mymap["ball"].y = mymap["canon"].y+(2*(mymap["rod"].radius_x))*sin(angle);
				string str = "rod";
				mymap[str].angle = get_angle(window);
				mymap[str].angle = max(mymap[str].angle,0.0);
				mymap[str].angle = min(mymap[str].angle,90.0);
				for(int i = 0;i < 4; i++){
					string s = "rectangle";
					s+=('0'+i+1);
					mymap[s].is_possible = 1;

				}
				MAXV = sqrt(vx*vx + vy*vy);	
				stt = 0;
				s1 =1;
				cnt = 0;
				state = 2;
				st1 = 0;
				cntdif = 20;
				number_of_balls++;
				//out1(state);

			}
			else if (action == GLFW_REPEAT){
				break;
			}
			else if(action == GLFW_PRESS){
				if(keyst!=0)return;
				st1 = 1;

			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			if (action == GLFW_RELEASE)
				break;
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	   is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-600.0f, 600.0f, -300.0f, 300.0f, 0.1f, 500.0f);
}

VAO *main_rectangle;
VAO *main_rectangle1;
VAO *main_rectangle2;
VAO *power_bar;

// Creates the triangle object used in this sample code
inline float get_dist(float x,float y,float x1,float y1){
	return sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
}

struct VAO* createPolygon(int parts,COLOR c,float radius1,float radius2){
	GLfloat vertex_buffer_data[225],color_buffer_data[225];
	int i,j;
	float angle=(2*M_PI/parts);
	float current_angle = 0;
	for(i=0;i<parts;i++){
		for(j=0;j<3;j++){
			float sum = 0;
			if(j == 0)sum = 50/255.0;
			color_buffer_data[i*9+j*3]=	min(c.r+sum,1.0f);
			color_buffer_data[i*9+j*3+1]=min(c.g+sum,1.0f);
			color_buffer_data[i*9+j*3+2]=min(c.b+sum,1.0f);
		}
		vertex_buffer_data[i*9] = 0;
		vertex_buffer_data[i*9+1] = 0;
		vertex_buffer_data[i*9+2]=0;
		vertex_buffer_data[i*9+3]=radius1*cos(current_angle);
		vertex_buffer_data[i*9+4]=radius2*sin(current_angle);
		vertex_buffer_data[i*9+5]=0;
		vertex_buffer_data[i*9+6]=radius1*cos(current_angle+angle);
		vertex_buffer_data[i*9+7]=radius2*sin(current_angle+angle);
		vertex_buffer_data[i*9+8]=0;
		current_angle+=angle;
	}
	return create3DObject(GL_TRIANGLES, 3*parts, vertex_buffer_data, color_buffer_data, GL_FILL);
}

struct VAO* generate_rect(string s,float radius1,float radius2,COLOR c){

	GLfloat a[20] = {
		-radius1,-radius2,0.0,
		-radius1,radius2,0.0,
		radius1,radius2,0.0,
		radius1,radius2,0.0,
		radius1,-radius2,0.0,
		-radius1,-radius2,0.0,
	};
	GLfloat b[20];
	for(int i=0;i<2;i++){
		for(int j=0;j<3;j++){
			float sum = 0;
			if(s[s.length()-1] == '7'){
				if(j == 0){
					sum = 40/255.0;
				}
			}
			b[i*9+j*3] = min(c.r+sum,1.0f);
			b[i*9+j*3+1] =min(c.g+sum,1.0f);
			b[i*9+j*3+2] = min(c.b+sum,1.0f);
		}
	}
	return create3DObject(GL_TRIANGLES,6,a,b,GL_FILL);
}

void createTriangle ()
{
	object temp,tmp,tmp2;
	float radius = 60;
	temp.x = XMIN+1.5*radius;
	temp.y = YMIN+radius;
	int parts = 14;
	temp.color.r = 52/255.0;
	temp.color.g = 42/255.0 ;
	temp.color.b = 25/255.0 ;
	temp.obj = createPolygon(parts,temp.color,radius,radius);
	temp.angle = 180.0/parts;
	temp.radius_x = radius;
	temp.radius_y = radius;	
	temp.radius = radius;
	mymap["canon"] = temp;

	/* circle */

	radius -=15;
	tmp2.x = temp.x;
	tmp2.y = temp.y;
	parts = 14;
	tmp2.color.r = 	92/255.0;
	tmp2.color.g = 84/255.0;
	tmp2.color.b =  70/255.0;
	tmp2.obj = createPolygon(parts,tmp2.color,radius,radius);
	tmp2.angle = 180.0/parts;
	tmp2.radius_x = radius;
	tmp2.radius_y = radius;
	tmp2.radius = radius;
	mymap["canon2"] = tmp2;

	/* canon rod */

	float radius1 = 50;
	float radius2 = 18;
	tmp.x = temp.x+radius+radius1;
	tmp.y = temp.y;
	parts = 4;
	tmp.color.r = 52/255.0;
	tmp.color.g = 42/255.0;
	tmp.color.b = 25/255.0;
	tmp.radius_x = radius1;
	tmp.radius_y = radius2;
	tmp.radius = sqrt(radius1*radius1+radius2*radius2);
	tmp.obj = generate_rect("rod",radius1,radius2,tmp.color);
	tmp.angle = 0;
	mymap["rod"] = tmp;

	/* Boxes */

	COLOR c1,c2,c3;
	c1.r = 150/255.0;
	c1.g = 111/255.0;
	c1.b = 	51/255.0;
	c2.r =  	160/255.0;
	c2.g = 125/255.0;
	c2.b =  	71/255.0;
	c3.r = 179/255.0;
	c3.g = 150/255.0;	
	c3.b = 107/255.0;
	COLOR GOLD;
	GOLD.r = 255/255.0;
	GOLD.g = 226/255.0;
	GOLD.b = 25/255.0;
	float side = 20;
	parts = 4;
	float x = XMAX - 12*side -30*(levels-1);
	float y = YMIN +side;
	for(int i = 0; i < NB+1; i++){
		object tmp;
		tmp.x = x;
		tmp.y = y;
		if(i == NB){
			tmp.x = x+3.4*side;
			tmp.y = y;

		}
		if(i/2 == 0)tmp.color = c1;
		else if(i/2 == 1)tmp.color = c2;
		else tmp.color = c1;
		if(i == NB){
			tmp.color = GOLD;
		}
		tmp.radius_x = side;
		tmp.radius_y = side;
		tmp.y_vel = 0;
		tmp.gravity = GRAVITY;
		tmp.onground = 1;
		tmp.inair = 0;
		tmp.radius = sqrt(2.0)*side;
		string str="rectangle";
		str+=('0'+(i+1));
		tmp.obj = generate_rect(str,side,side,tmp.color);
		if(i%2 == 1){
			y+=2*side;
			x-=6.8*side;
		}
		else x+=6.8*side;
		tmp.angle = 0;
		mymap[str] = tmp;
		tmp.fric = 3;
	}

	/* Canonball */

	object tmp1;
	radius = 15;
	parts = 20;
	tmp1.x = temp.x;
	tmp1.y = temp.y;
	tmp1.gravity = GRAVITY;
	tmp1.angle = 0;
	tmp1.color.r = tmp1.color.g = tmp1.color.b = 0.0f;
	tmp1.radius_y = 15;
	tmp1.radius_x = 15;
	tmp1.obj = createPolygon(parts,tmp1.color,radius,radius);
	tmp1.onground = 0;
	tmp1.inair = 0;
	tmp1.x_vel = tmp1.y_vel = 0;
	tmp1.fric = 3;
	mymap["ball"] = tmp1;
	tmp.radius = radius;

	/* Coins */

	x = XMIN + 150 ;
	y = 0;
	side = 18;
	parts = 20;
	for(int i = 0; i < NC;i++){
		object tmp;
		tmp.x = x+rand()%3*side;
		tmp.y = y+rand()%4*side;
		tmp.color = GOLD;
		tmp.radius_x = side;
		tmp.radius_y = side;
		tmp.y_vel = 0;
		tmp.x_vel = 0;
		tmp.gravity = 0;
		tmp.onground = 0;
		tmp.inair = 0;
		tmp.radius = side;
		string str="coin";
		str+=('0'+(i+1));
		tmp.obj = createPolygon(parts,tmp.color,side,side);
		tmp.angle = 0;
		mymap[str] = tmp;
		if(i%2 == 0){
			y+=6*side;
		}
		else{
			y -= 6*side;
		}
		x+=6*side;
	}

	/* Marker */
	object m;
	m.x = XMIN + 45;
	m.y = 105;
	m.color.r = m.color.b = m.color.g = 0;
	m.x_vel = 0;
	m.y_vel = 0;
	m.gravity = 0;
	m.angle = 0;
	m.onground = 0;
	GLfloat a [] = {
		-5, 0,0, // vertex 0
		5,5,0, // vertex 1
		5,-5,0, // vertex 2
	};
	GLfloat b[] = {
		0,0,0,
		0,0,0,
		0,0,0,
	};
	m.obj = create3DObject(GL_TRIANGLES, 3, a, b, GL_FILL);
	mymap["marker"] = m;

}
// Creates the rectangle object used in this sample code
VAO* rectangle;
void createRectangle ()
{
	// GL3 accepts only Triangles. Quads are not supported
	GLfloat vertex_buffer_data [] = {
		XMIN,YMIN,0, // vertex 1
		XMIN,YMIN/3.0+1,0, // vertex 2
		XMAX, YMIN/3.0+1,0, // vertex 3

		XMAX, YMIN/3.0+1,0, // vertex 3
		XMAX, YMIN,0, // vertex 4
		XMIN,YMIN,0  // vertex 1
	};

	GLfloat color_buffer_data [20];
	for(int i = 0;i < 6; i++){
		color_buffer_data[i*3] = 135/255.0;
		color_buffer_data[i*3+1] = 206/255.0;
		color_buffer_data[i*3+2] = 250/255.0;
	}


	GLfloat vertex_buffer_data1 [] = {
		XMIN,YMIN/3.0,0, // vertex 1
		XMIN,YMAX/3.0,0, // vertex 2
		XMAX, YMAX/3.0,0, // vertex 3

		XMAX, YMAX/3.0,0, // vertex 3
		XMAX, YMIN/3.0,0, // vertex 4
		XMIN,YMIN/3.0,0  // vertex 1
	};

	GLfloat color_buffer_data1 [20];
	for(int i = 0;i < 6; i++){
		color_buffer_data1[i*3] = 157/255.0;
		color_buffer_data1[i*3+1] = 214/255.0;
		color_buffer_data1[i*3+2] = 250/255.0;

	}
	GLfloat vertex_buffer_data2 [] = {
		XMIN,YMAX/3.0,0, // vertex 1
		XMIN,YMAX,0, // vertex 2
		XMAX, YMAX,0, // vertex 3

		XMAX, YMAX,0, // vertex 3
		XMAX, YMAX/3.0,0, // vertex 4
		XMIN,YMAX/3.0,0  // vertex 1
	};

	GLfloat color_buffer_data2 [20];
	for(int i = 0;i < 6; i++){
		color_buffer_data2[i*3] = 	179/255.0;
		color_buffer_data2[i*3+1] =  223/255.0;
		color_buffer_data2[i*3+2] =  251/255.0;

	} GLfloat vertex_buffer_data3 [] = {
		XMIN+10,100.0,0, // vertex 1
		XMIN+10,YMAX-10,0, // vertex 2
		XMIN+30, YMAX-10,0, // vertex 3

		XMIN+30, YMAX-10,0, // vertex 3
		XMIN+30, 100.0,0, // vertex 4
		XMIN+10,100.0,0  // vertex 1
	};

	GLfloat color_buffer_data3 [] = {
		0.0,1.0,0.0, // color 1
		1.0,0.0,0.0, // color 2
		1.0,0.0,0.0, // color 3

		1,0,0, // color 3
		0,1,0, // color 4
		0,1,0,  // color 1
	};


	main_rectangle = create3DObject(GL_TRIANGLES,6,vertex_buffer_data,color_buffer_data,GL_FILL);
	main_rectangle1 = create3DObject(GL_TRIANGLES,6,vertex_buffer_data1,color_buffer_data1,GL_FILL);
	main_rectangle2 = create3DObject(GL_TRIANGLES,6,vertex_buffer_data2,color_buffer_data2,GL_FILL);
	power_bar = create3DObject(GL_TRIANGLES,6,vertex_buffer_data3,color_buffer_data3,GL_FILL);
}

void createBackground(GLuint textureID)
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		XMIN,YMIN,0, // vertex 1
		XMAX,YMIN,0, // vertex 2
		XMAX, YMAX,0, // vertex 3

		XMAX, YMAX,0, // vertex 3
		XMIN, YMAX,0, // vertex 4
		XMIN,YMIN,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
	static const GLfloat texture_buffer_data [] = {
		0,1, // TexCoord 1 - bot left
		1,1, // TexCoord 2 - bot right
		1,0, // TexCoord 3 - top right

		1,0, // TexCoord 3 - top right
		0,0, // TexCoord 4 - top left
		0,1  // TexCoord 1 - bot left
	};

	// create3DTexturedObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
}


bool check_collision(object a,object b){
	float dist = get_dist(a.x,a.y,b.x,b.y);

	float exp1= a.radius+b.radius;
	//out2(dist,exp1);
	if(dist < exp1) return true;
	return false;
}
bool check_collision_rect(object a,object b){
	if((abs(a.x - b.x )<a.radius_x+b.radius_x) && (abs(a.y-b.y)<a.radius_y+b.radius_y)) return true;
	return false;
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;



void display_end(char *str){
	Matrices.model = glm::mat4(1.0f);
	glm::vec3 fontColor = glm::vec3(0,0,0);
	glm::mat4 translateText = glm::translate(glm::vec3(XMIN+100,YMAX-100,0));
	float fontScaleValue = 40 ;
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	glm::mat4 MVP = Matrices.projection * Matrices.view * Matrices.model;
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render(str);
	translateText = glm::translate(glm::vec3(0,-1,0));
	Matrices.model *= (translateText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render("Press R to restart the level, Q to quit");
	translateText = glm::translate(glm::vec3(0,-1,0));
	Matrices.model *= (translateText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render("Press B to go back to previous level");
	if(done == 3){
		translateText = glm::translate(glm::vec3(0,-1,0));
		Matrices.model *= (translateText);
		MVP = Matrices.projection * Matrices.view * Matrices.model;
		glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
		GL3Font.font->Render("Press N to proceed to next level");
	}

}

void display_string(float x,float y,char *str,float fontScaleValue){
	glm::vec3 fontColor = glm::vec3(0,0,0);
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(x,y,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
	Matrices.model *= (translateText * scaleText);
	glm::mat4 MVP = Matrices.projection * Matrices.view * Matrices.model;
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render(str);

}
/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window)
{
	// clear the color and depth in the frame buffer

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	// use the loaded shader program
	// Don't change unless you know what you are doing

	// Eye - Location of camera. Don't change unless you are sure!!

	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	Matrices.projection = glm::ortho(-600.0f/zoom_camera+offset, 600.0f/zoom_camera+offset, -300.0f/zoom_camera, 300.0f/zoom_camera, 0.1f, 500.0f);
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	glUseProgram(textureProgramID);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
	glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle * rotateRectangle);
	MVP = VP * Matrices.model;

	// Copy MVP to texture shaders
	glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

	// Set the texture sampler to access Texture0 memory
	glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DTexturedObject(rectangle);

		glUseProgram (programID);

	// Load identity to model matrix


	// draw3DObject draws the VAO given to it using current MVP matrix

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();

	Matrices.model = glm::mat4(1.0f);	
	translateRectangle = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef
	rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= translateRectangle*(rotateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	if(number_of_balls > limit_balls){
		done  = 1;
	}

	if(done == 1){
		usleep(500000);
		glUseProgram(fontProgramID);
		char score_str[80];
		sprintf(score_str,"Game over. Thanks for playing Your score is %0.lf",score);
		display_end(score_str);
		return;
	}
	else if (done == 3){
		usleep(500000);
		glUseProgram(fontProgramID);
		char score_str[80];
		sprintf(score_str,"Congratulations. Level Successfully completed. Thanks for playing Your score is %0.lf",score);
		display_end(score_str);
		return;
	}
	draw3DObject(power_bar);
	map<string,object>::iterator itr;
	if(pst!=0){
		MAXV+=pst*0.5;
		MAXV = max(MAXV,10.0);
		MAXV = min(MAXV,50.0);
	}

	for(itr = mymap.begin(); itr != mymap.end(); itr++ ){
		string str = itr->first;
		float oldx = mymap[str].x;
		float oldy = mymap[str].y;
		mymap[str].x+=mymap[str].x_vel;
		mymap[str].y+=mymap[str].y_vel;
		bool todo = true;
		if(str == "ball" && todraw == 0) todo = false;
		if(todo){
			if(str == "rod")
				if(keyst == 1){
					mymap[str].angle+=1;
					mymap[str].angle = min(mymap[str].angle,90.0);
				}
				else if(keyst == 2){
					mymap[str].angle-=1;
					mymap[str].angle = max(mymap[str].angle,0.0);
				}
			if(mymap[str].x>=XMAX-mymap[str].radius_x){
				mymap[str].x=XMAX-mymap[str].radius_x;
				mymap[str].x_vel = -(mymap[str].x_vel)*0.6;
			}
			if(mymap[str].y <= YMIN+mymap[str].radius_y){
				mymap[str].y=YMIN+mymap[str].radius_y;
				if(str!="ball"  && mymap[str].y_vel < mymap[str].gravity)mymap[str].y_vel = 0;
				else mymap[str].y_vel = -min(mymap[str].y_vel*0.5,3*mymap[str].gravity);
			}
			else mymap[str].y_vel-=mymap[str].gravity;
			if(mymap[str].onground == 1){
				float v = mymap[str].x_vel;
				if(v<0) v+=mymap[str].fric;
				else  if(v>0) v-=mymap[str].fric;
				if(v*mymap[str].x_vel <=0) v = 0;
				if(v == mymap[str].x_vel) v = 0;
				mymap[str].x_vel = v;
			}
			if(mymap[str].inair == 1){
				float vx = mymap[str].x_vel;
				float nvx = abs(airfric*vx);
				if(vx < 0) nvx = vx+nvx;
				else nvx = vx - nvx;
				if(vx*nvx < 0) nvx = 0;
				mymap[str].x_vel = nvx;
			}
			if(abs(mymap[str].y_vel)<mymap[str].gravity) mymap[str].y_vel = 0;
			if(mymap[str].inair == 1 && mymap[str].y == YMIN+mymap[str].radius_y && mymap[str].y_vel < mymap[str].gravity){
				mymap[str].inair = 0;
				mymap[str].y_vel = 0;
			}

			if(abs(mymap[str].y_vel)<mymap[str].gravity)mymap[str].y_vel = 0;

			if(mymap[str].y == YMIN+mymap[str].radius_y && mymap[str].y_vel < mymap[str].gravity){
				mymap[str].inair = 0;
				mymap[str].onground = 1;
				mymap[str].y_vel = 0;
			}
			if(str == "ball"){
				float vx = mymap[str].x_vel;
				float vy = mymap[str].y_vel;
				for(int i = 0;i<NC;i++){
					string ss = "coin";
					ss+='0'+(i+1);
					if(check_collision(mymap[ss],mymap[str])){
						score+=10;
						mymap.erase(mymap.find(ss));
					}	
				}
				for(int i = 0; i < NB; i++){
					string s="rectangle";
					s+='0'+(i+1);
					if(check_collision(mymap[str],mymap[s])==1){
						object a = mymap[str];
						object b = mymap[s];
						if(mymap[s].is_possible){
							mymap[s].to_rot = 1;
							mymap[s].angle = 90;
							mymap[s].is_possible = 0;
						}
						if(a.x+a.radius_x>b.x-b.radius_x && a.x < b.x && abs(a.y-b.y) < b.radius_y){

							/* left collsion */
							//cout << "left\n";
							mymap[str].x_vel = mymap[str].x_vel/2;
							mymap[s].x_vel = mymap[str].x_vel*2;
							mymap[str].x = mymap[s].x - mymap[str].radius_x - mymap[s].radius_x;
						}
						else if (a.x-a.radius_x<b.x+b.radius_x && a.x > b.x && abs(a.y-b.y) < b.radius_y){
							/*right collision */
							//cout << "right\n";
							mymap[str].x_vel = -mymap[str].x_vel/2;
							mymap[s].x_vel = -mymap[str].x_vel;
							mymap[s].x = mymap[str].x - mymap[str].radius_x - mymap[s].radius_x;	
						}
						else if (a.y > b.y && abs(a.x-b.x)<(a.radius_x+b.radius_x)/2){

							/* top collsion */
							//cout << "top\n";

							mymap[str].y = mymap[s].y + mymap[s].radius_y + mymap[str].radius_y;
							mymap[str].y_vel = -mymap[str].y_vel*0.8;
							mymap[str].x_vel/=2;
							mymap[s].x_vel = mymap[str].x_vel;
						}


						else if(a.y > b.y && abs(a.x-b.x) < (a.radius_x+b.radius_x)/2){
							//cout << "Down\n";
							mymap[str].y = mymap[s].y - mymap[s].radius_y - mymap[str].radius_y;
							mymap[str].y_vel = -mymap[str].y_vel*0.6;
							mymap[s].x_vel = mymap[str].x_vel/2;
							mymap[s].y_vel = -mymap[str].y_vel/2;
						}

					}
				}
				string s = "rectangle";
				s+='0'+NB+1;
				if(check_collision(mymap[str],mymap[s])){
					mymap.erase(mymap.find(s));
					mymap[str].y = YMIN+mymap[str].radius_y;
					score+=100;
					score+=(limit_balls - number_of_balls)*5*(levels/3+1);
					done = 3;
					levels++;
					levels = min(levels,8);
				}
				if(vx == 0 && vy!=0){
					vx = 3;
				}
				if(vx == 0 && (mymap[str].x>0 ||mymap[str].y==YMIN+mymap[str].radius_y)){
					if(stt == 0){
						startt = glfwGetTime();
						stt = 1;
					}
					else if(stt == 1){
						float curt = glfwGetTime();
						if(curt - startt >= 2/16.0){
							stt = 2;
						}
					}
					else{
						mymap[str].x = mymap["canon"].x;
						mymap[str].y = mymap["canon"].y;
						s1 = 0;
						mymap[str].inair = 0;
						mymap[str].onground = 0;
						todraw = 0;
						MAXV = 10;
					}
				}
				if(oldy == mymap[str].y && mymap[str].y!=YMIN+mymap[str].radius_y && mymap[str].y_vel > mymap[str].gravity){
					if(stt == 0){
						startt = glfwGetTime();
						stt = 1;
					}
					else if(stt == 1){
						float curt = glfwGetTime();
						if(curt - startt >= 2/16.0){
							stt = 2;
						}
					}
					else{
						mymap[str].x = mymap["canon"].x;
						mymap[str].y = mymap["canon"].y;
						s1 = 0;
						mymap[str].inair = 0;
						mymap[str].onground = 0;
						todraw = 0;
						MAXV = 10;
					}
				}
				if(vy > 5)vx*=2;
			}
			if(str[0] == 'r' && str[1] == 'e'){
				/* It is a rectangle  */
				if(mymap[str].to_rot)
					mymap[str].angle -= 15.0;
				if(mymap[str].angle < 0){
					mymap[str].to_rot = 0;
					mymap[str].angle = 0;
				}
				for(int i = 0; i < NB; i++){
					string s="rectangle";
					s+='0'+(i+1);
					object a = mymap[str];
					object b = mymap[s];
					//cout << "Called for "<< str << " " << s << endl; 
					//out4(a.x,b.x,a.y,b.y);
					if(s!=str && check_collision_rect(mymap[str],mymap[s])){
						//cout << "Entered for " << str << " and " << s << endl;

						if(a.y > b.y && a.y <= b.y + b.radius_y+a.radius_y){
							/* its on top */
							//cout << "Entered top for " << s << " and " << str << endl;	
							mymap[str].y =mymap[s].y+mymap[s].radius_y+mymap[str].radius_y;
							mymap[str].y_vel = 0;
							//out2(mymap[str].y,mymap[s].y);
						}
						else if(a.y == b.y && a.x < b.x+b.radius_x+a.radius_x){
							//cout << "gone\n";
							//mymap[str].x = mymap[s].x - mymap[s].radius_x-mymap[str].radius_x;
							float diff = (b.x-a.x);
							mymap[str].x-=diff/2;
							mymap[s].x+=diff/2-2;
							if(mymap[s].x > XMAX-b.radius_x){
								mymap[str].x-=mymap[s].radius_x;
								mymap[s].x = b.x;
							}
						}
						else if(a.x == b.x && a.y == b.y){
							/* Overlapped Ouch */

							if(str < s){
								mymap[str].x = mymap[str].x = mymap[s].x - mymap[s].radius_x-mymap[str].radius_x;
							}
						}
					}
				}
			}
			if(mymap[str].x>=XMAX-mymap[str].radius_x){
				mymap[str].x=XMAX-mymap[str].radius_x;
				//mymap[str].x_vel = -(mymap[str].x_vel)*0.6;
			}
			if(mymap[str].y <= YMIN+mymap[str].radius_y){
				mymap[str].y=YMIN+mymap[str].radius_y;
				//if(str!="ball"  && mymap[str].y_vel < mymap[str].gravity)mymap[str].y_vel = 0;
				//else mymap[str].y_vel = -min(mymap[str].y_vel*0.5,3*mymap[str].gravity);
			}
			if(mymap[str].y < mymap["canon"].y && str!="canon" && str!="canon2" && str!="rod" && mymap[str].x <= mymap["canon"].x+mymap["canon"].radius_x+mymap[str].radius_x){
				mymap[str].x = mymap["canon"].x+mymap["canon"].radius_x+mymap[str].radius_x+10;
				mymap[str].x_vel = mymap[str].y_vel = 0;
			}
			if(str == "ball" && todraw == 0)continue;
			Matrices.model = glm::mat4(1.0f);
			if(str == "rod"){
				if(st1 == 1){
					mymap[str].angle = get_angle(window);
					mymap[str].angle = max(mymap[str].angle,0.0);
					mymap[str].angle = min(mymap[str].angle,90.0);
				}

				translateRectangle = glm::translate (glm::vec3(mymap["canon"].x-cntdif*1.0, mymap["canon"].y, 0));        // glTranslatef
				rotateRectangle = glm::rotate((float)(mymap[str].angle*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
				Matrices.model *= translateRectangle*(rotateRectangle);
				MVP = VP * Matrices.model;
				translateRectangle = glm::translate(glm::vec3(-mymap["canon"].x+cntdif*1.0,-mymap["canon"].y,0));
				Matrices.model *= translateRectangle;
				translateRectangle = glm::translate (glm::vec3(mymap[str].x-cntdif*1.0, mymap[str].y, 0));
				Matrices.model *= translateRectangle;
				MVP = VP * Matrices.model;
				//out1(state);
				if(state == SHOOT1){
					//out1(mymap[str].angle);
					mymap[str].angle-=1;
					if(mymap[str].angle < 0){
						mymap[str].angle = 0;
						state = 0;
					}
				}
			}
			else{
				float newx = mymap[str].x;
				float newy = mymap[str].y;
				if(str[0] == 'c' && str[1] == 'a'){
					newx-=cntdif;
					cntdif--;			
					cntdif = max(0,cntdif);
				}
				if(str == "marker"){
					newy+=(MAXV-10)/40.0*MAXL - 5;
				}

				translateRectangle = glm::translate (glm::vec3(newx, newy, 0));
				rotateRectangle = glm::rotate((float)(mymap[str].angle*M_PI/180.0f), glm::vec3(0,0,1));
				Matrices.model *= translateRectangle*(rotateRectangle);
				MVP = VP * Matrices.model;
			}

			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
			draw3DObject(mymap[str].obj);
		}
	}
	//static int fontScale = 0;
	float fontScaleValue = 36 ;
	glm::vec3 fontColor = glm::vec3(0,0,0);
	glUseProgram(fontProgramID);


	char score_str[30],level_str[30],left_str[30];
	sprintf(level_str,"level: %d",levels);
	sprintf(score_str,"score: %0.lf",score);
	sprintf(left_str,"Number of Balls left: %d",limit_balls - number_of_balls);
	display_string(XMIN+60,YMAX-30,level_str,fontScaleValue);
	display_string(XMAX - 140,YMAX - 30,score_str,fontScaleValue);
	display_string(XMIN+60,YMAX - 50,left_str,fontScaleValue);
	
	
}

GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "My Angry Birds Version", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	   is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetScrollCallback(window,mousescroll);

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	/* Objects should be created before any other gl function and shaders */
	// Create the models
	

	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();

	glActiveTexture(GL_TEXTURE0);
	// load an image file directly as a new OpenGL texture
	// GLuint texID = SOIL_load_OGL_texture ("beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL3
	GLuint textureID = createTexture("background.png");
	// check for an error during the load process
	if(textureID == 0 )
		cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

	// Create and compile our GLSL program from the texture shaders
	textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");
	createBackground(textureID);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL3.vert", "Sample_GL3.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.3, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	const char* fontfile = "Monaco.ttf";
	GL3Font.font = new FTExtrudeFont(fontfile); // 3D extrude style rendering

	if(GL3Font.font->Error())
	{
		cout << "Error: Could not load font `" << fontfile << "'" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Create and compile our GLSL program from the font shaders
	fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
	GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
	fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
	fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
	fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
	GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
	GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

	GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
	GL3Font.font->FaceSize(1);
	GL3Font.font->Depth(0);
	GL3Font.font->Outset(0, 0);
	GL3Font.font->CharMap(ft_encoding_unicode);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}


void initialise(){
	score = 0;	
	mymap.clear();
	MAXV = 20;
	camera_rotation_angle = 90;
	rectangle_rotation = 0;
	triangle_rotation = 0;
	triangle_rot_dir = 1;
	rectangle_rot_dir = 1;
	triangle_rot_status = true;
	rectangle_rot_status = true;
	orig_x_speed = 3.0f,orig_y_speed = 3.0;
	startt;
	todraw = 0;
	cnt = 0;
	stt = 0;
	keyst = 0;
	cntdif = 0;
	st1 = 0,state,s1=0;
	pst = 0;
	done = 0;
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
	number_of_balls = 0;
	limit_balls = limits[levels];

}

int main (int argc, char** argv)
{


	GLFWwindow* window = initGLFW(window_width, window_height);

	initGL (window, window_width, window_height);

	double last_update_time = glfwGetTime(), current_time;
	levels = 1;
	limit_balls = limits[levels];

	/* Draw in loop */
	while (!glfwWindowShouldClose(window)) {
		// OpenGL Draw commands
		draw(window);

		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();
		if(done == 2){
			initialise();
		}
		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 1.0) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
			//draw(window);
		}
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
