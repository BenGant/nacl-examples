//-----------------------------------------------------------------------------
//           Name: ogl_glslang_simple_vs2ps.cpp
//         Author: Kevin Harris
//  Last Modified: 04/21/05
//    Description: This sample demonstrates how to write vertex and fragment 
//                 shaders using OpenGL's new high-level shading language 
//                 GLslang.
//
//   Control Keys: F1 - Toggle usage of vertex and fragment shaders.
//
// Note: The fragment shader has been changed slightly from what the 
//       fixed-function pipeline does by default so you can see a noticeable 
//       change when toggling the shaders on and off. Instead of modulating 
//       the vertex color with the texture's texel, the fragment shader adds 
//       the two together, which causes the fragment shader to produce a 
//       brighter, washed-out image. This modification can be switched back in 
//       the fragment shader file.
//-----------------------------------------------------------------------------



#define PPAPI	//Are we using Pepper!
//#define GLES2_SUPPORT_CLIENT_SIDE_ARRAYS

//leave these dfined for both PPAPI and Win32 compiles, so we get the benefit of some windows functions
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef PPAPI
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
//#include <GL/glaux.h> //CLM we don't use gluax any more
//#include "resource.h" //un-needed because we're not touching icons
#include <GL/gl.h>
#include <GL/glu.h>
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")


//-----------------------------------------------------------------------------
// FUNCTION POINTERS FOR OPENGL EXTENSIONS
//-----------------------------------------------------------------------------

// For convenience, this project ships with its own "glext.h" extension header 
// file. If you have trouble running this sample, it may be that this "glext.h" 
// file is defining something that your hardware doesn’t actually support. 
// Try recompiling the sample using your own local, vendor-specific "glext.h" 
// header file.


#include <glext.h> // Your local header file

// GL_ARB_shader_objects
PFNGLCREATEPROGRAMOBJECTARBPROC  glCreateProgramObjectARB  = NULL;
PFNGLDELETEOBJECTARBPROC         glDeleteObjectARB         = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC     glUseProgramObjectARB     = NULL;
PFNGLCREATESHADEROBJECTARBPROC   glCreateShaderObjectARB   = NULL;
PFNGLSHADERSOURCEARBPROC         glShaderSourceARB         = NULL;
PFNGLCOMPILESHADERARBPROC        glCompileShaderARB        = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = NULL;
PFNGLATTACHOBJECTARBPROC         glAttachObjectARB         = NULL;
PFNGLGETINFOLOGARBPROC           glGetInfoLogARB           = NULL;
PFNGLLINKPROGRAMARBPROC          glLinkProgramARB          = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC   glGetUniformLocationARB   = NULL;
PFNGLUNIFORM4FARBPROC            glUniform4fARB            = NULL;
PFNGLUNIFORM1IARBPROC            glUniform1iARB            = NULL;
HDC	      g_hDC       = NULL;

HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;

#else

#include <cstdio>
#include <sstream>
#include <iostream>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <math.h>

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppp.h"
#include "ppapi/c/ppb_core.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"
#include "ppapi/c/pp_input_event.h"
#include "ppapi/c/ppb_opengles2.h"
#include "ppapi/c/ppb_graphics_3d.h"
#include "ppapi/lib/gl/gles2/gl2ext_ppapi.h" 
#include <gles2/gl2.h>

static PPB_Messaging* ppb_messaging_interface = NULL;
static PPB_Var* ppb_var_interface = NULL;
static PPB_Core* ppb_core_interface = NULL;
static PPB_Instance* ppb_instance_interface = NULL;
static PPB_Graphics3D* ppb_g3d_interface = NULL;
const PPB_OpenGLES2* PPBOpenGLES2 = NULL;

PP_Resource graphicsContext_;
PP_Instance appInstance_;	

//GLES2 specific
typedef GLuint GLhandleARB;


//#pragma comment(lib,"ppapi_gles2.lib")


#endif

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------


GLuint    g_textureID = 0;
GLuint	 g_vboID = 0;
GLuint	 g_ibID = 0;
GLhandleARB g_programObj;
GLhandleARB g_vertexShader;
GLhandleARB g_fragmentShader;
GLuint      g_location_testTexture;
GLuint     g_location_positionLoc;
GLuint     g_location_texCoordLoc;
GLuint     g_location_colorLoc;
GLuint     g_location_MVP;

bool g_bUseShaders = true;

float g_fSpinX = 0.0f;
float g_fSpinY = 0.0f;

// GL_T2F_C3F_V3F
struct Vertex
{
	float tu, tv;
	float r, g, b;
	float x, y, z;
};

Vertex g_quadVertices[] =
{
	// tu,  tv     r    g    b       x     y     z 
	{ 0.0f,0.0f,  1.0f,1.0f,0.0f, -1.0f,-1.0f, -4.0f },
	{ 1.0f,0.0f,  1.0f,0.0f,0.0f,  1.0f,-1.0f, -4.0f },
	{ 1.0f,1.0f,  0.0f,1.0f,0.0f,  1.0f, 1.0f, -4.0f },
	{ 0.0f,1.0f,  0.0f,0.0f,1.0f, -1.0f, 1.0f, -4.0f },
};

GLubyte g_Indices[] = { 0, 1, 2, 0, 2, 3 };

//-----------------------------------------------------------------------------

void init(void);
void shutDown(void);
unsigned char *readShaderFile(const char *fileName);
void initShader(void);

#ifndef PPAPI

LRESULT CALLBACK WindowProc(HWND g_hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void render(void);
//-----------------------------------------------------------------------------
int initInstance()
{
	WNDCLASSEX winClass;
	MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));

	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof(WNDCLASSEX);
	winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = g_hInstance;
    winClass.hIcon	       = NULL;// CLM not needed LoadIcon(g_hInstance, (LPCTSTR)IDI_OPENGL_ICON);
    winClass.hIconSm	   =  NULL;// CLM not needed LoadIcon(g_hInstance, (LPCTSTR)IDI_OPENGL_ICON);
	winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;
	
	if( !RegisterClassEx(&winClass) )
		return E_FAIL;

	g_hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						    "OpenGL - Simple Vertex & Fragment Shader Using GLslang",
							WS_OVERLAPPEDWINDOW,
					 	    0,0, 640,480, NULL, NULL, g_hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

    ShowWindow( g_hWnd, 1 /*nCmdShow*/);//CLM hard-code this to 1 for hax-scake
	 
    UpdateWindow( g_hWnd );

	init();
	initShader();

	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
		    render();
	}

	shutDown();

    UnregisterClass( "MY_WINDOWS_CLASS", g_hInstance );

	return uMsg.wParam;
}

//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   g_hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam )
{
    static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bMousing;
    
    switch( msg )
	{
        case WM_KEYDOWN:
		{
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage(0);
					break;

				case VK_F1:
					g_bUseShaders = !g_bUseShaders;
					break;
			}
		}
        break;

        case WM_LBUTTONDOWN:
		{
			ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD (lParam);
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD (lParam);
			bMousing = true;
		}
		break;

		case WM_LBUTTONUP:
		{
			bMousing = false;
		}
		break;

		case WM_MOUSEMOVE:
		{
			ptCurrentMousePosit.x = LOWORD (lParam);
			ptCurrentMousePosit.y = HIWORD (lParam);

			if( bMousing )
			{
				g_fSpinX -= (ptCurrentMousePosit.x - ptLastMousePosit.x);
				g_fSpinY -= (ptCurrentMousePosit.y - ptLastMousePosit.y);
			}
			
			ptLastMousePosit.x = ptCurrentMousePosit.x;
            ptLastMousePosit.y = ptCurrentMousePosit.y;
		}
		break;
		
		case WM_SIZE:
		{
			int nWidth  = LOWORD(lParam); 
			int nHeight = HIWORD(lParam);
			glViewport(0, 0, nWidth, nHeight);

			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( 45.0, (GLdouble)nWidth / (GLdouble)nHeight, 0.1, 100.0);
		}
		break;

		case WM_CLOSE:
		{
			PostQuitMessage(0);	
		}

        case WM_DESTROY:
		{
            PostQuitMessage(0);
		}
        break;
		
		default:
		{
			return DefWindowProc( g_hWnd, msg, wParam, lParam );
		}
		break;
	}

	return 0;
}
#else

//prototype header
void render(void* pData, int32_t dataSize);

int initInstance()
{
	init();
	
	//kick off rendering loop here
	//now that everything's loaded, kick off our first render frame
	PP_CompletionCallback cc = PP_MakeCompletionCallback(render, 0);
  int32_t swap_result;
  swap_result = ppb_g3d_interface->SwapBuffers(graphicsContext_, cc);
	
	return 0;
}

#endif

//-----------------------------------------------------------------------------
void init( void )
{
#ifndef PPAPI
	GLuint PixelFormat;

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW |PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
#else
		// Lazily create the Pepper context.

	int32_t attribs[] = {
			PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
			PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
			PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 8,
			PP_GRAPHICS3DATTRIB_SAMPLES, 0,
			PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 0,
			PP_GRAPHICS3DATTRIB_WIDTH, 640,
			PP_GRAPHICS3DATTRIB_HEIGHT, 480,
			PP_GRAPHICS3DATTRIB_NONE
	};

	graphicsContext_ =  ppb_g3d_interface->Create(appInstance_, 0, attribs);
	int32_t success =  ppb_instance_interface->BindGraphics(appInstance_, graphicsContext_);
	if (success == PP_FALSE) 
	{
			glSetCurrentContextPPAPI(0);
			return;
	}


	glSetCurrentContextPPAPI(graphicsContext_);
	glViewport(0,0, 640,480);

	initShader();

	
#endif
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );


	//CLM note that we don't have an image decoder in the gluax library any more. I converted the test.bmp to test.raw and provided it
	char img[128*128*3];
	FILE* f = fopen("C:\\Users\\colton\\Desktop\\mainroach.git\\plugin-port2\\plugin-port2\\test.raw" ,"rb");
	if(f)
	{
		fread(&img[0], 128*128*3,1,f);
		fclose(f);
	
		glGenTextures( 1, &g_textureID );

		glBindTexture( GL_TEXTURE_2D, g_textureID );

		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 128,128, 0,GL_RGB, GL_UNSIGNED_BYTE, &img[0] );
	}

#ifdef PPAPI
		
	//generate a buffer object

   glGenBuffers(1, &g_vboID);
   glBindBuffer(GL_ARRAY_BUFFER, g_vboID);
   glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Vertex), (void*)&g_quadVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &g_ibID);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibID);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(char), (void*)&g_Indices[0], GL_STATIC_DRAW);
	
	
#endif
	
}


//-----------------------------------------------------------------------------
unsigned char *readShaderFile( const char *fileName )
{
    FILE *file = fopen( fileName, "r" );

    if( file == NULL )
    {
        MessageBox( NULL, "Cannot open shader file!", "ERROR",  MB_OK | MB_ICONEXCLAMATION );
		return 0;
    }

    struct _stat fileStats;

    if( _stat( fileName, &fileStats ) != 0 )
    {
        MessageBox( NULL, "Cannot get file stats for shader file!", "ERROR",MB_OK | MB_ICONEXCLAMATION );
        return 0;
    }

    unsigned char *buffer = new unsigned char[fileStats.st_size];

	int bytes = fread( buffer, 1, fileStats.st_size, file );

    buffer[bytes] = 0;

	fclose( file );

	return buffer;
}

#include <vector>
GLuint LoadShader(GLenum type, char const * source){
	//postMessage("loading shader");

	GLuint shader = glCreateShader(type);
	if (!shader) {
		GLenum res= glGetError();
	//	postMessage("glCreateShader() failed" );
		return res;
	}

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint compiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled){
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen){
			std::vector<char> msg(infoLen);
			glGetShaderInfoLog(shader, infoLen, NULL, &msg[0]);
			
			 MessageBox( NULL, &msg[0], "ERROR COMPILING SHADER",MB_OK | MB_ICONEXCLAMATION );

		}
		else
			MessageBox( NULL, "UNKNOWN", "ERROR COMPILING SHADER",MB_OK | MB_ICONEXCLAMATION );

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

//-----------------------------------------------------------------------------
void initShader( void )
{
	//
	// If the required extension is present, get the addresses of its 
	// functions that we wish to use...
	//
#ifndef PPAPI
	char *ext = (char*)glGetString( GL_EXTENSIONS );

    if( strstr( ext, "GL_ARB_shading_language_100" ) == NULL )
    {
        //This extension string indicates that the OpenGL Shading Language,
        // version 1.00, is supported.
        MessageBox(NULL,"GL_ARB_shading_language_100 extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }

    if( strstr( ext, "GL_ARB_shader_objects" ) == NULL )
    {
        MessageBox(NULL,"GL_ARB_shader_objects extension was not found",
            "ERROR",MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    else
    {
        glCreateProgramObjectARB  = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
        glDeleteObjectARB         = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
        glUseProgramObjectARB     = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
        glCreateShaderObjectARB   = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
        glShaderSourceARB         = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
        glCompileShaderARB        = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
        glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
        glAttachObjectARB         = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
        glGetInfoLogARB           = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
        glLinkProgramARB          = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
        glGetUniformLocationARB   = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocationARB");
        glUniform4fARB            = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress("glUniform4fARB");
		glUniform1iARB            = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");

        if( !glCreateProgramObjectARB || !glDeleteObjectARB || !glUseProgramObjectARB ||
            !glCreateShaderObjectARB || !glCreateShaderObjectARB || !glCompileShaderARB || 
            !glGetObjectParameterivARB || !glAttachObjectARB || !glGetInfoLogARB || 
            !glLinkProgramARB || !glGetUniformLocationARB || !glUniform4fARB ||
			!glUniform1iARB )
        {
            MessageBox(NULL,"One or more GL_ARB_shader_objects functions were not found",
                "ERROR",MB_OK|MB_ICONEXCLAMATION);
            return;
        }
    }

    const char *vertexShaderStrings[1];
    const char *fragmentShaderStrings[1];
    GLint bVertCompiled;
    GLint bFragCompiled;
    GLint bLinked;
    char str[4096];

	//
	// Create the vertex shader...
	//

    g_vertexShader = glCreateShaderObjectARB( GL_VERTEX_SHADER_ARB );

	unsigned char *vertexShaderAssembly = readShaderFile( "C:\\Users\\colton\\Desktop\\mainroach.git\\plugin-port2\\plugin-port2\\vertex_shader.vert" );
    vertexShaderStrings[0] = (char*)vertexShaderAssembly;
    glShaderSourceARB( g_vertexShader, 1, vertexShaderStrings, NULL );
    glCompileShaderARB( g_vertexShader);
    delete vertexShaderAssembly;

    glGetObjectParameterivARB( g_vertexShader, GL_OBJECT_COMPILE_STATUS_ARB, 
                               &bVertCompiled );
    if( bVertCompiled  == false )
	{
		glGetInfoLogARB(g_vertexShader, sizeof(str), NULL, str);
		MessageBox( NULL, str, "Vertex Shader Compile Error", MB_OK|MB_ICONEXCLAMATION );
	}

	//
	// Create the fragment shader...
	//

    g_fragmentShader = glCreateShaderObjectARB( GL_FRAGMENT_SHADER_ARB );

    unsigned char *fragmentShaderAssembly = readShaderFile( "C:\\Users\\colton\\Desktop\\mainroach.git\\plugin-port2\\plugin-port2\\fragment_shader.frag" );
    fragmentShaderStrings[0] = (char*)fragmentShaderAssembly;
    glShaderSourceARB( g_fragmentShader, 1, fragmentShaderStrings, NULL );
    glCompileShaderARB( g_fragmentShader );
    delete fragmentShaderAssembly;

    glGetObjectParameterivARB( g_fragmentShader, GL_OBJECT_COMPILE_STATUS_ARB, 
                               &bFragCompiled );
    if( bFragCompiled == false )
	{
		glGetInfoLogARB( g_fragmentShader, sizeof(str), NULL, str );
		MessageBox( NULL, str, "Fragment Shader Compile Error", MB_OK|MB_ICONEXCLAMATION );
	}

    //
    // Create a program object and attach the two compiled shaders...
    //

    g_programObj = glCreateProgramObjectARB();
    glAttachObjectARB( g_programObj, g_vertexShader );
    glAttachObjectARB( g_programObj, g_fragmentShader );

    //
    // Link the program object and print out the info log...
    //

    glLinkProgramARB( g_programObj );
    glGetObjectParameterivARB( g_programObj, GL_OBJECT_LINK_STATUS_ARB, &bLinked );

    if( bLinked == false )
	{
		glGetInfoLogARB( g_programObj, sizeof(str), NULL, str );
		MessageBox( NULL, str, "Linking Error", MB_OK|MB_ICONEXCLAMATION );
	}

	//
	// Locate some parameters by name so we can set them later...
	//

	g_location_testTexture = glGetUniformLocationARB( g_programObj, "testTexture" );
#else
	
	unsigned char *vertexShaderAssembly = readShaderFile( "C:\\Users\\colton\\Desktop\\mainroach.git\\plugin-port2\\plugin-port2\\vertex_shader_es2.vert" );
	g_vertexShader = LoadShader(GL_VERTEX_SHADER,(char*)vertexShaderAssembly);
	delete[] vertexShaderAssembly;

	unsigned char *fragmentShaderAssembly = readShaderFile( "C:\\Users\\colton\\Desktop\\mainroach.git\\plugin-port2\\plugin-port2\\fragment_shader_es2.frag" );
	g_fragmentShader = LoadShader(GL_FRAGMENT_SHADER,(char*)fragmentShaderAssembly);
	delete[] fragmentShaderAssembly;


   g_programObj = glCreateProgram();
   glAttachShader(g_programObj, g_vertexShader);
   glAttachShader(g_programObj, g_fragmentShader);

   glLinkProgram(g_programObj);


   g_location_positionLoc = glGetAttribLocation  ( g_programObj, "a_position" );
	g_location_colorLoc    = glGetAttribLocation  ( g_programObj, "a_color" );
   g_location_texCoordLoc = glGetAttribLocation  ( g_programObj, "a_texCoord" );
   

   g_location_MVP = glGetUniformLocation( g_programObj, "a_MVP" );
   g_location_testTexture = glGetUniformLocation ( g_programObj, "s_texture" );

#endif
}

//-----------------------------------------------------------------------------
void shutDown( void )
{
#ifndef PPAPI
    glDeleteTextures( 1, &g_textureID );

    glDeleteObjectARB( g_vertexShader );
    glDeleteObjectARB( g_fragmentShader );
    glDeleteObjectARB( g_programObj );

	if( g_hRC != NULL )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( g_hRC );
		g_hRC = NULL;							
	}

	if( g_hDC != NULL )
	{
		ReleaseDC( g_hWnd, g_hDC );
		g_hDC = NULL;
	}
#else
#endif
}


//-----------------------------------------------------------------------------
#ifndef PPAPI
void render(void)
#else
//from - http://www.opengl.org/wiki/GluPerspective_code

void glhFrustumf2(float *matrix, float left, float right, float bottom, float top,
                  float znear, float zfar)
{
    float temp, temp2, temp3, temp4;
    temp = 2.0 * znear;
    temp2 = right - left;
    temp3 = top - bottom;
    temp4 = zfar - znear;
    matrix[0] = temp / temp2;
    matrix[1] = 0.0;
    matrix[2] = 0.0;
    matrix[3] = 0.0;
    matrix[4] = 0.0;
    matrix[5] = temp / temp3;
    matrix[6] = 0.0;
    matrix[7] = 0.0;
    matrix[8] = (right + left) / temp2;
    matrix[9] = (top + bottom) / temp3;
    matrix[10] = (-zfar - znear) / temp4;
    matrix[11] = -1.0;
    matrix[12] = 0.0;
    matrix[13] = 0.0;
    matrix[14] = (-temp * zfar) / temp4;
    matrix[15] = 0.0;
}
//-----------------------------------------------------------------------------
void glhPerspectivef2(float *matrix, float fovyInDegrees, float aspectRatio,
                      float znear, float zfar)
{
    float ymax, xmax;
    ymax = znear * tanf(fovyInDegrees * 3.14f / 360.0);
    xmax = ymax * aspectRatio;
    glhFrustumf2(matrix, -xmax, xmax, -ymax, ymax, znear, zfar);
}

//-----------------------------------------------------------------------------
void render(void* pData, int32_t dataSize)
#endif
{
#ifndef PPAPI
	// Clear the screen and the depth buffer
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, -4.0f );
    glRotatef( -g_fSpinY, 1.0f, 0.0f, 0.0f );
    glRotatef( -g_fSpinX, 0.0f, 1.0f, 0.0f );

	if( g_bUseShaders == true )
	{
		//
		// Use vertex and fragment shaders...
		//

        glUseProgramObjectARB( g_programObj );

        // Identify the texture to use and bind it to texture unit 0
        if( g_location_testTexture != -1 )
            glUniform1iARB( g_location_testTexture, 0 );

		glInterleavedArrays( GL_T2F_C3F_V3F, 0, g_quadVertices );
		glDrawArrays( GL_QUADS, 0, 4 );

		glUseProgramObjectARB( NULL );
	}
	else
	{
		//
		// Render the normal way...
		//

		glBindTexture( GL_TEXTURE_2D, g_textureID );
		glInterleavedArrays( GL_T2F_C3F_V3F, 0, g_quadVertices );
		glDrawArrays( GL_QUADS, 0, 4 );
	}

	SwapBuffers( g_hDC );
#else
	 if(graphicsContext_ ==0)
		 return;

   glClearColor(0.5,0.5,0.5,1);
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

     //set what program to use
	glUseProgram( g_programObj );
	
   //bind our texture to the shader
   glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D,g_textureID );
   glUniform1i ( g_location_testTexture, 0 );


   //create our perspective matrix
   float mpv[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
   glhPerspectivef2(&mpv[0], 45.0f, 640.0f / 480.0f,1, 10);
   glUniformMatrix4fv( g_location_MVP, 1, GL_FALSE, (GLfloat*) &mpv[0] );

   //define the attributes of the vertex
   float* pV = (float*)&g_quadVertices[0];
 	glBindBuffer(GL_ARRAY_BUFFER, g_vboID);
   
   glVertexAttribPointer(g_location_positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,x));
   glEnableVertexAttribArray(g_location_positionLoc);
   glVertexAttribPointer(g_location_texCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,tu));
   glEnableVertexAttribArray(g_location_texCoordLoc);
   glVertexAttribPointer(g_location_colorLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex,r));
   glEnableVertexAttribArray(g_location_colorLoc);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ibID);
	glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_BYTE ,0 );


	GLenum res= glGetError();
	if(res)
	{
		char str[256];
		sprintf(&str[0],"%i",res);
		MessageBox( NULL,  &str[0],"GLERR",MB_OK | MB_ICONEXCLAMATION );
		return;
	}

	 

	PP_CompletionCallback cc = PP_MakeCompletionCallback(render, 0);
  int32_t swap_result;
  swap_result = ppb_g3d_interface->SwapBuffers(graphicsContext_, cc);
#endif
}



//-----------------------------------------------------------------------------
static struct PP_Var CStrToVar(const char* str) {
  if (ppb_var_interface != NULL) {
    return ppb_var_interface->VarFromUtf8(str, strlen(str));
  }
  return PP_MakeUndefined();
}

//-----------------------------------------------------------------------------

static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {

	appInstance_ = instance;

	//PP_Var v =  CStrToVar("Hello a World (NEWLIB)");
  //ppb_messaging_interface->PostMessage(instance,v);
  //fprintf(stdout,"Hello a World (NEWLIB)");

  return PP_TRUE;
}
//-----------------------------------------------------------------------------

static void Instance_DidDestroy(PP_Instance instance) 
{
	ppb_core_interface->ReleaseResource(graphicsContext_);
	graphicsContext_ =0;
}
//-----------------------------------------------------------------------------

static void Instance_DidChangeView(PP_Instance instance,
                                   PP_Resource view_resource) 
{

 
  // On first call, create graphics3d resource.
  if(graphicsContext_ ==0)
  {
	  initInstance();
  
  } 
/*	else if (position.size.width != global.width || position.size.height != global.height) 
	{
    ppb_g3d_interface->ResizeBuffers(global.graphics3d_id, position.size.width,
                                                       position.size.height);
  }
  */
  //global.width = position.size.width;
  //global.height = position.size.height;
  // Pending completion callbacks actually occur here after exit.

}
//-----------------------------------------------------------------------------
static void Instance_DidChangeFocus(PP_Instance instance,
                                    PP_Bool has_focus) {
}

//-----------------------------------------------------------------------------
static PP_Bool Instance_HandleDocumentLoad(PP_Instance instance,
                                           PP_Resource url_loader) {
  /* NaCl modules do not need to handle the document load function. */
  return PP_FALSE;
}
//-----------------------------------------------------------------------------
PP_Bool InputEvent_HandleEvent(PP_Instance instance_id, PP_Resource input_event) {
  // Handle events here...
  
 /* PP_InputEvent_Type type = PPBInputEvent->GetType(input_event);
  switch (type) {
    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
      PP_Point point;
      point = PPBMouseInputEvent->GetPosition(input_event);
      global.cursor_x = point.x;
      global.cursor_y = point.y;
      break;
    default:
      break;
  }*/
  return PP_TRUE;
}


//-----------------------------------------------------------------------------
PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id, PPB_GetInterface get_browser) 
{
  ppb_messaging_interface = (PPB_Messaging*)(get_browser(PPB_MESSAGING_INTERFACE));
  ppb_var_interface = (PPB_Var*)(get_browser(PPB_VAR_INTERFACE));
  ppb_instance_interface = (PPB_Instance*)get_browser(PPB_INSTANCE_INTERFACE);
  ppb_core_interface = (PPB_Core*)get_browser(PPB_CORE_INTERFACE);

  //CLM needed for GL initalization
  ppb_g3d_interface = (PPB_Graphics3D*)get_browser(PPB_GRAPHICS_3D_INTERFACE);
	if (!glInitializePPAPI(get_browser))
		return -1;
  

  return PP_OK;
}

//-----------------------------------------------------------------------------

PP_EXPORT const void* PPP_GetInterface(const char* interface_name) {
  static PPP_Instance instance_interface = {
    &Instance_DidCreate,
    &Instance_DidDestroy,
    &Instance_DidChangeView,
    &Instance_DidChangeFocus,
    &Instance_HandleDocumentLoad
  };

  if (strcmp(interface_name, PPP_INSTANCE_INTERFACE) == 0)
    return &instance_interface;
  return NULL;
}

//-----------------------------------------------------------------------------
PP_EXPORT void PPP_ShutdownModule() {
	glTerminatePPAPI();
}
