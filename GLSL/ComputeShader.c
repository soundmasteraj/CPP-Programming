#include <windows.h>
#include <GL/gl.h>

typedef int(APIENTRY* PFNWGLSWAPINTERVALEXTPROC)(int i);
typedef GLuint(APIENTRY* PFNGLCREATEPROGRAMPROC)();
typedef GLuint(APIENTRY* PFNGLCREATESHADERPROC)(GLenum t);
typedef void(APIENTRY* PFNGLSHADERSOURCEPROC)(GLuint s, GLsizei c, const char*const*string, const GLint* i);
typedef void(APIENTRY* PFNGLCOMPILESHADERPROC)(GLuint s);
typedef void(APIENTRY* PFNGLATTACHSHADERPROC)(GLuint p, GLuint s);
typedef void(APIENTRY* PFNGLLINKPROGRAMPROC)(GLuint p);
typedef void(APIENTRY* PFNGLUSEPROGRAMPROC)(GLuint p);
typedef void (APIENTRY* PFNGLDISPATCHCOMPUTEPROC) (GLuint x, GLuint y, GLuint z);
typedef void (APIENTRY* PFNGLBINDIMAGETEXTUREPROC) (GLuint a, GLuint b, GLint c, GLboolean d, GLint e, GLenum f, GLenum g);

static const char* ComputeShader = \
	"#version 430 core\n"
	"writeonly uniform image2D writer;"
	"layout (local_size_x = 16, local_size_y = 16) in;"
	"void main()"
	"{"
		"vec2 coordinates = gl_GlobalInvocationID.xy;"
		"vec2 resolution = vec2(1024,1024);"
		"vec2 k = sign(cos(coordinates/resolution.yy*32.0));"
		"imageStore(writer,ivec2(gl_GlobalInvocationID),vec4(k.x*k.y));"
	"}" ;
	
static const char* FragmentShader = \
	"#version 430 core\n"
	"layout (location=0) out vec4 color;"
	"uniform sampler2D reader;"
	"void main()"
	"{"	
		"color = texture(reader,gl_FragCoord.xy/vec2(1920,1080));"
	"}";

void LoadTexture() 
{
	GLuint h;
	glBindTexture(GL_TEXTURE_2D, h);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1024,1024,0,GL_RGBA,GL_FLOAT,0);
	((PFNGLBINDIMAGETEXTUREPROC)wglGetProcAddress("glBindImageTexture"))(0,h,0,GL_FALSE,0,0x88B9,GL_RGBA8);
}

int MakeShader(const char* source, GLenum type)
{
	int p = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();
	int s = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(type);	
	((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s,1,&source,0);
	((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
	((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(p,s);
	((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(p);
	return p;
}

int main()
{ 
	ShowCursor(0);
	PIXELFORMATDESCRIPTOR pfd = {0,0,PFD_DOUBLEBUFFER};
	HDC hdc = GetDC(CreateWindow("static",0,WS_POPUP|WS_VISIBLE|WS_MAXIMIZE,0,0,0,0,0,0,0,0));
	SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);
	wglMakeCurrent(hdc, wglCreateContext(hdc));
	((PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress ("wglSwapIntervalEXT")) (0);	
	int FS = MakeShader(FragmentShader,0x8B30);
	int CS = MakeShader(ComputeShader,0x91B9);
	LoadTexture();
	((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(CS);
	((PFNGLDISPATCHCOMPUTEPROC)wglGetProcAddress("glDispatchCompute"))(1024/16,1024/16,1);
	((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(FS);					
	do
	{ 
		glRects(-1,-1,1,1);
		wglSwapLayerBuffers(hdc,WGL_SWAP_MAIN_PLANE);
	} while (!GetAsyncKeyState(VK_ESCAPE));
	return 0;
}