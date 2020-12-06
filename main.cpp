/*
*        Computer Graphics Course - Shenzhen University
*    Week 9 - Phong Reflectance Model (per-fragment shading)
* ============================================================
*
* - ����������ǲο����룬����Ҫ����ο���ҵ˵��������˳������ɡ�
* - ��������OpenGL�����������������У���ο���һ��ʵ��γ�����ĵ���
*/

#include "include/Angel.h"
#include "include/TriMesh.h"

//#pragma comment(lib, "glew32.lib")

#include <cstdlib>
#include <iostream>

using namespace std;

GLuint programID;
GLuint vertexArrayID;
GLuint vertexBufferID;
GLuint vertexNormalID;
GLuint vertexIndexBuffer;

GLuint vPositionID;
GLuint vNormalID;

GLuint modelMatrixID;
GLuint viewMatrixID;
GLuint projMatrixID;

GLuint coefficientsID;
GLuint lightPosID;

TriMesh* mesh = new TriMesh();
vec3 lightPos(0.0, 0.0, 2.0);
//////////////////////////////////////////////////////////////////////////
// ƽ������
class Plane {
public:
	vector<vec3f> vertices;
	vector<vec3f> vnormals;
	vector<vec3i> index;

	Plane(GLfloat y = -1., GLfloat size = 1.0) {
		// ��ʼ��ƽ��
		vertices.push_back(vec3f(-size, y, -size));
		vertices.push_back(vec3f(-size, y, size));
		vertices.push_back(vec3f(size, y, size));
		vertices.push_back(vec3f(size, y, -size));


		// ����
		index.push_back(vec3i(0, 1, 3));
		index.push_back(vec3i(2, 1, 3));

		// ������
		vnormals.push_back(vec3f(0, 1, 0));
		vnormals.push_back(vec3f(0, 1, 0));
		vnormals.push_back(vec3f(0, 1, 0));
		vnormals.push_back(vec3f(0, 1, 0));
	}
	void add_index(int step) {
		for (int i = 0; i < index.size(); ++i) {
			index[i].a += step;
			index[i].b += step;
			index[i].c += step;
		}
	}
};
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// �����������

// �������
double Delta = M_PI / 2.0;
double Theta = M_PI / 2.0;
double R = 1.0;

namespace Camera
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projMatrix;

	mat4 ortho(const GLfloat left, const GLfloat right,
		const GLfloat bottom, const GLfloat top,
		const GLfloat zNear, const GLfloat zFar)
	{
		// TODO �밴��ʵ������ݲ�ȫ����۲����ļ���
		mat4 c;
		c[0][0] = 2.0 / (right - left);
		c[1][1] = 2.0 / (top - bottom);
		c[2][2] = -2.0 / (zFar - zNear);
		c[3][3] = 1.0;
		c[0][3] = -(right + left) / (right - left);
		c[1][3] = -(top + bottom) / (top - bottom);
		c[2][3] = -(zFar + zNear) / (zFar - zNear);
		return c;
	}

	mat4 perspective(const GLfloat fovy, const GLfloat aspect,
		const GLfloat zNear, const GLfloat zFar)
	{
		// TODO �밴��ʵ������ݲ�ȫ����۲����ļ���
		GLfloat top = tan(fovy * M_PI / 180 / 2) * zNear;
		GLfloat right = top * aspect;

		mat4 c;
		c[0][0] = zNear / right;
		c[1][1] = zNear / top;
		c[2][2] = -(zFar + zNear) / (zFar - zNear);
		c[2][3] = -(2.0 * zFar * zNear) / (zFar - zNear);
		c[3][2] = -1.0;
		c[3][3] = 0.0;
		return c;
	}

	mat4 lookAt(const vec4& eye, const vec4& at, const vec4& up)
	{
		// TODO �밴��ʵ������ݲ�ȫ����۲����ļ���
		vec4 n = normalize(eye - at);
		vec4 u = normalize(vec4(cross(up, n), 0.0));
		vec4 v = normalize(vec4(cross(n, u), 0.0));

		vec4 t = vec4(0.0, 0.0, 0.0, 1.0);
		mat4 c = mat4(u, v, n, t);
		return c * Translate(-eye);
	}
}

//////////////////////////////////////////////////////////////////////////
// OpenGL ��ʼ����y = -1.1��ƽ��
Plane plane(-1.1, 3);

void init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	programID = InitShader("vshader_frag.glsl", "fshader_frag.glsl");

	// �Ӷ�����ɫ����ƬԪ��ɫ���л�ȡ������λ��
	vPositionID = glGetAttribLocation(programID, "vPosition");
	vNormalID = glGetAttribLocation(programID, "vNormal");
	lightPosID = glGetUniformLocation(programID, "lightPos");

	modelMatrixID = glGetUniformLocation(programID, "modelMatrix");
	viewMatrixID = glGetUniformLocation(programID, "viewMatrix");
	projMatrixID = glGetUniformLocation(programID, "projMatrix");
	coefficientsID = glGetUniformLocation(programID, "coefficients");

	// ��ȡ�ⲿ��άģ��
	mesh->read_off("sphere.off");

	vector<vec3f> vs = mesh->v();
	vector<vec3i> fs = mesh->f();
	vector<vec3f> ns;

	// TODO ������ģ����ÿ������ķ����������洢��ns������
	for (int i = 0; i < vs.size(); ++i) {
		ns.push_back(vs[i] - vec3(0.0, 0.0, 0.0));
	}
	// ��ƽ��׷�ӵ�vs, fs, ns������
	plane.add_index(vs.size());
	vs.insert(vs.end(), plane.vertices.begin(), plane.vertices.end());
	fs.insert(fs.end(), plane.index.begin(), plane.index.end());
	ns.insert(ns.end(), plane.vnormals.begin(), plane.vnormals.end());

	// ����VAO
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// ����VBO�����󶨶�������
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vs.size() * sizeof(vec3f), vs.data(), GL_STATIC_DRAW);

	// ����VBO�����󶨷���������
	glGenBuffers(1, &vertexNormalID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexNormalID);
	glBufferData(GL_ARRAY_BUFFER, ns.size() * sizeof(vec3f), ns.data(), GL_STATIC_DRAW);

	// ����VBO�����󶨶�������
	glGenBuffers(1, &vertexIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, fs.size() * sizeof(vec3i), fs.data(), GL_STATIC_DRAW);

	// OpenGL��Ӧ״̬����
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// ���óɻ�ɫ����
	float tmp = 0.5;
	glClearColor(tmp, tmp, tmp, 1.0);
}

//////////////////////////////////////////////////////////////////////////
// ��Ⱦ

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);

	//Ĭ������
	// TODO �����������

	vec4 eye = vec4(R * sin(Delta) * cos(Theta), R * cos(Delta), R * sin(Delta) * sin(Theta), 0);
	vec4 at = vec4(0, 0, 0, 0);
	vec4 up = vec4(0, 1, 0, 0);

	Camera::modelMatrix = mat4(1.); // �����Ƶ�y���Ϸ�
	Camera::viewMatrix = Camera::lookAt(eye, at, up);
	Camera::projMatrix = Camera::ortho(-3, 3, -3, 3, -3, 3);


	glUniformMatrix4fv(viewMatrixID, 1, GL_TRUE, &Camera::viewMatrix[0][0]);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, &Camera::projMatrix[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_TRUE, &Camera::modelMatrix[0][0]);


	// ����Դλ�ô��붥����ɫ��
	glUniform3fv(lightPosID, 1, &lightPos[0]);
	// ��ϵ������ƬԪ��ɫ��
	vec3 coefficients(1.0, 1.0, 0.5);
	glUniform3fv(coefficientsID, 1, &coefficients[0]);


	glEnableVertexAttribArray(vPositionID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glVertexAttribPointer(
		vPositionID,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glEnableVertexAttribArray(vNormalID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexNormalID);
	glVertexAttribPointer(
		vNormalID,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);

	glDrawElements(
		GL_TRIANGLES,
		int((mesh->f().size()) * 3),
		GL_UNSIGNED_INT,
		(void*)0
	);
	// ����ƽ��
	coefficients = vec3(1., 0., 0.);
	glUniform3fv(coefficientsID, 1, &coefficients[0]); // ͨ��coefficents�������ֹ�ĵ�ϵ�����Ա��ڻ���ƽ��ʱȥ�������䣬���淴���Ӱ��

	glDrawElements(
		GL_TRIANGLES,
		int(2 * 3),
		GL_UNSIGNED_INT,
		(void *)(mesh->f().size() * sizeof(vec3i)) // ע��˴�����byte����λ�����ʹ��sizeof()
	);


	// ������Ӱ����
	float lx = lightPos[0];
	float ly = lightPos[1];
	float lz = lightPos[2];

	mat4 shadowProjMatrix(-ly, 0.0, 0.0, 0.0,
		lx, 0.0, lz, 1.0,
		0.0, 0.0, -ly, 0.0,
		0.0, 0.0, 0.0, -ly);
	// ��Ӱ����
	// Ϊ�˽�ͶӰ����Ϊ-1����Ҫ�Ƚ�������y�����ƶ�һ����ͶӰ���������-y�����ƶ�һ��
	Camera::modelMatrix = Translate(0, -1, 0) * shadowProjMatrix * Translate(0, 1, 0);

	glUniformMatrix4fv(viewMatrixID, 1, GL_TRUE, &Camera::viewMatrix[0][0]);
	glUniformMatrix4fv(projMatrixID, 1, GL_TRUE, &Camera::projMatrix[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_TRUE, &Camera::modelMatrix[0][0]);

	// ��ϵ������ƬԪ��ɫ�����Ա㽫Ӱ����Ϊ��ɫ
	coefficients = vec3(0, 0, 0);
	glUniform3fv(coefficientsID, 1, &coefficients[0]);

	glDrawElements(
		GL_TRIANGLES,
		int(mesh->f().size() * 3),
		GL_UNSIGNED_INT,
		(void*)0
	);

	// ��������
	glDisableVertexAttribArray(vPositionID);
	glUseProgram(0);

	glutSwapBuffers();
}

//////////////////////////////////////////////////////////////////////////
// �������ô���

void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
}

//////////////////////////////////////////////////////////////////////////
void idle(void)
{
	glutPostRedisplay();
}
//////////////////////////////////////////////////////////////////////////
// �����Ӧ����

void mouse(int x, int y)
{
	lightPos[0] = (x - 250) / 25.0;
	lightPos[1] = (250 - y) / 25.0;
	glutIdleFunc(idle);
	return;
}

//////////////////////////////////////////////////////////////////////////
// ������Ӧ����

// ����Delta
void updateDelta(int sign, double step) {
	Delta += sign * step;

}

// ����Theta
void updateTheta(int sign, double step) {
	Theta += sign * step;
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 033:	// ESC�� �� 'q' ���˳���Ϸ
		exit(EXIT_SUCCESS);
		break;
	case 'q':
		exit(EXIT_SUCCESS);
		break;
		// Todo�����̿��������λ�úͳ���
		// w, s --> R����
		// e, d --> Delta
		// r, f --> Theta
	case 'w':
		R += 0.1;
		break;
	case 's':
		R -= 0.1;
		break;
	case 'e':
		updateDelta(1, 0.1);
		break;
	case 'd':
		updateDelta(-1, 0.1);
		break;
	case 'r':
		updateTheta(1, 0.1);
		// ����Ϊ2 * M_PI
		break;
	case 'f':
		updateTheta(-1, 0.1);
		break;
	}
	glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

void clean()
{
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &vertexArrayID);

	if (mesh) {
		delete mesh;
		mesh = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(500, 500);
	glutCreateWindow("OpenGL-Tutorial");

	glewInit();
	init();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMotionFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop();

	clean();

	return 0;
}
