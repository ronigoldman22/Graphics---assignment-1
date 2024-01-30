#include "game.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"

static void printMat(const glm::mat4 mat)
{
	std::cout << " matrix:" << std::endl;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			std::cout << mat[j][i] << " ";
		std::cout << std::endl;
	}
}

Game::Game() : Scene() //default constructor for the Game class. It initializes the game scene by calling the base class (Scene) constructor.
{
}

Game::Game(float angle, float relationWH, float near1, float far1) : Scene(angle, relationWH, near1, far1)// constructor that takes parameters to set up the camera and perspective. It also calls the base class constructor.
{
}

void Game::Init()//This function initializes the game. It loads shaders, a texture, and adds a shape to the scene (a plane in this case). It also sets up the camera and initializes some variables.
{

	AddShader("../res/shaders/pickingShader");
	AddShader("../res/shaders/basicShader");

	//AddTexture("../res/textures/lena256.jpg",false);
	int width, height, numComponents;
	unsigned char* data = stbi_load("res\textures\lena256.jpg", &width, &height, &numComponents, 4);
	AddShape(Plane, -1, TRIANGLES);

	pickedShape = 0;

	//SetShapeTex(0,0);
	MoveCamera(0, zTranslate, 10);
	pickedShape = -1;

	//ReadPixel(); //uncomment when you are reading from the z-buffer
}

void Game::Update(const glm::mat4& MVP, const glm::mat4& Model, const int  shaderIndx)//This function updates the game state. It sets uniforms in the shader related to the model-view-projection matrices, normals, light direction, and light color.
{
	Shader* s = shaders[shaderIndx];
	int r = ((pickedShape + 1) & 0x000000FF) >> 0;
	int g = ((pickedShape + 1) & 0x0000FF00) >> 8;
	int b = ((pickedShape + 1) & 0x00FF0000) >> 16;
	s->Bind();
	s->SetUniformMat4f("MVP", MVP);
	s->SetUniformMat4f("Normal", Model);
	s->SetUniform4f("lightDirection", 0.0f, 0.0f, -1.0f, 0.0f);
	if (shaderIndx == 0)
		s->SetUniform4f("lightColor", r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
	else
		s->SetUniform4f("lightColor", 0.7f, 0.8f, 0.1f, 1.0f);
	s->Unbind();
}

void Game::WhenRotate()
{
}

void Game::WhenTranslate()
{
}

void Game::Motion()
{
	if (isActive)
	{
	}
}

Game::~Game(void)
{
}