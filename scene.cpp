#include "scene.h"
#include "glad/include/glad/glad.h"
#include <iostream>


	//prints the elements of a given 4*4 matrix to the console
	static void printMat(const glm::mat4 mat)
	{
		printf(" matrix: \n");
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
				printf("%f ", mat[j][i]);
			printf("\n");
		}
	}

	//default constructor
	Scene::Scene()
	{
		//set the line width:
		glLineWidth(5);
	
		//creates a camera with default parameters
		cameras.push_back(new Camera(60.0f,1.0,0.1f,100.0f));		
		pickedShape = -1;
		depth = 0;
		cameraIndx = 0;
		xold = 0;
		yold = 0;
		isActive = false;
	}

	//constructor with parameters
	Scene::Scene(float angle,float relationWH,float near1, float far1)
	{
		//glLineWidth(5);
		cameras.push_back(new Camera(angle,relationWH,near1,far1));
		pickedShape = -1;
		depth = 0;
		cameraIndx = 0;
		xold = 0;
		yold = 0;
		isActive = false;
	}

	//adds a shape to the scene by loading it from a file
	void Scene::AddShapeFromFile(const std::string& fileName,int parent,unsigned int mode)
	{
		chainParents.push_back(parent); //container for relationships between shapes in the scene
		shapes.push_back(new Shape(fileName,mode));
	}

	//adds a shape to the scene based in its type
	void Scene::AddShape(int type, int parent,unsigned int mode)
	{
		chainParents.push_back(parent);
		shapes.push_back(new Shape(type,mode));
	}

	//adds a copy of an exiting shape to the scene
	void Scene::AddShapeCopy(int indx,int parent,unsigned int mode)
	{
		chainParents.push_back(parent);
		shapes.push_back(new Shape(*shapes[indx],mode));
	}

	//adds a shader to the scene
	void Scene::AddShader(const std::string& fileName)
	{
		shaders.push_back(new Shader(fileName));
	}

	//adds texture to the scene, either loaded from a file or created from raw data
	void Scene::AddTexture(const std::string& textureFileName,bool for2D)
	{
		textures.push_back(new Texture(textureFileName));
	}
	void Scene::AddTexture(int width,int height, unsigned char *data)
	{
		textures.push_back(new Texture(width,height,data));
	}

	//adds a new canera to the scene with specified parameters and translates it to a given position
	void Scene::AddCamera(glm::vec3& pos , float fov,float relationWH , float zNear, float zFar)
	{
		cameras.push_back(new Camera(fov,relationWH,zNear,zFar));
		cameras.back()->MyTranslate(pos,0);
	}

	
	//renders the scene
	void Scene::customDraw(int shaderIndx, int cameraIndx, int buffer, bool toClear, bool debugMode, int screenNum)
	{
		// Enable depth testing for proper rendering of 3D objects
		glEnable(GL_DEPTH_TEST);
		// Calculate the transformation matrix 'Normal' using the MakeTrans() function
		glm::mat4 Normal = MakeTrans();
		// Calculate the Model-View-Projection (MVP) matrix
		glm::mat4 MVP = cameras[cameraIndx]->GetViewProjection() * glm::inverse(cameras[cameraIndx]->MakeTrans());
		
		int p = pickedShape;// Store the current pickedShape value
		if (toClear)
		{ // If toClear is true, clear the screen with a specified color
			if (shaderIndx > 0)
				Clear(1, 1, 1, 1); // White background for objects
			else
				Clear(0, 0, 0, 0); // Transparent background for picking
		}
		if (screenNum == 0) {
			glViewport(0, 0, 256, 256);
		}
		else if (screenNum == 1) {
			glViewport(256, 0, 256, 256);
		}
		else if (screenNum == 2) {
			glViewport(0, 256, 256, 256);
		}
		else if (screenNum == 3) {
			glViewport(256, 256, 256, 256);
		}

		// Iterate over each shape in the scene
			for (unsigned int i = 0; i<shapes.size(); i++)
			{
				// Check if the shape should be rendered (Is2Render() is true)
				if(shapes[i]->Is2Render())
				{
					// Calculate the Model matrix for the current shape
					glm::mat4 Model = Normal * shapes[i]->MakeTrans();

					// Check if using a shader other than the default (shaderIndx > 0)
					if(shaderIndx > 0)
					{
						// Update the shader with MVP and Model matrices
						Update(MVP,Model,shapes[i]->GetShader());
						// Draw the shape with the specified shader, textures, and rendering mode
						shapes[i]->Draw(shaders,textures,false);
					}
					else
					{ // Picking mode: Update the shader with MVP and Model matrices
						Update(MVP,Model,0);
						// Draw the shape in picking mode
						shapes[i]->Draw(shaders,textures,true);
					}
				}
			}
			pickedShape = p;
		} 

	void Scene::Draw(int shaderIndx, int cameraIndx, int buffer, bool toClear, bool debugMode)
	{
		glEnable(GL_DEPTH_TEST);
		glm::mat4 Normal = MakeTrans();

		glm::mat4 MVP = cameras[cameraIndx]->GetViewProjection() * glm::inverse(cameras[cameraIndx]->MakeTrans());
		int p = pickedShape;
		if (toClear)
		{
			if (shaderIndx > 0)
				Clear(1, 0, 1, 1);
			else
				Clear(0, 0, 0, 0);
		}

		for (unsigned int i = 0; i < shapes.size(); i++)
		{
			if (shapes[i]->Is2Render())
			{
				glm::mat4 Model = Normal * shapes[i]->MakeTrans();

				if (shaderIndx > 0)
				{
					Update(MVP, Model, shapes[i]->GetShader());
					shapes[i]->Draw(shaders, textures, false);
				}
				else
				{ //picking
					Update(MVP, Model, 0);
					shapes[i]->Draw(shaders, textures, true);
				}
			}
		}
		pickedShape = p;
	}
	
	//moves camera based on the specified type and ampunt
	void Scene::MoveCamera(int cameraIndx,int type,float amt)
	{
		switch (type)
		{
			case xTranslate:
				cameras[cameraIndx]->MyTranslate(glm::vec3(amt,0,0),0);
			break;
			case yTranslate:
				cameras[cameraIndx]->MyTranslate(glm::vec3(0,amt,0),0);
			break;
			case zTranslate:
				cameras[cameraIndx]->MyTranslate(glm::vec3(0,0,amt),0);
			break;
			case xRotate:
				cameras[cameraIndx]->MyRotate(amt,glm::vec3(1,0,0),0);
			break;
			case yRotate:
				cameras[cameraIndx]->MyRotate(amt,glm::vec3(0,1,0),0);
			break;
			case zRotate:
				cameras[cameraIndx]->MyRotate(amt,glm::vec3(0,0,1),0);
			break;
			default:
				break;
		}
	}

	//applys transformation to the currently picked shape
	void Scene::ShapeTransformation(int type,float amt)
	{
		if(glm::abs(amt)>1e-5)
		{
			switch (type)
			{
			case xTranslate:
				shapes[pickedShape]->MyTranslate(glm::vec3(amt,0,0),0);
			break;
			case yTranslate:
				shapes[pickedShape]->MyTranslate(glm::vec3(0,amt,0),0);
			break;
			case zTranslate:
				shapes[pickedShape]->MyTranslate(glm::vec3(0,0,amt),0);
			break;
			case xRotate:
				shapes[pickedShape]->MyRotate(amt,glm::vec3(1,0,0),0);
			break;
			case yRotate:
				shapes[pickedShape]->MyRotate(amt,glm::vec3(0,1,0),0);
			break;
			case zRotate:
				shapes[pickedShape]->MyRotate(amt,glm::vec3(0,0,1),0);
			break;
			default:
				break;
			}
		}

	}
	//resizes the scene and adjust camera's projection
	void Scene::Resize(int width,int height)
	{
	
		cameras[0]->SetProjection(cameras[0]->GetAngle(),(float)width/height);
		glViewport(0,0,width,height);
		std::cout<<cameras[0]->GetRelationWH()<<std::endl;
	}

	float Scene::Picking(int x,int y)
	{
		
		
		return 0;
	}
	//return coordinates in global system for a tip of arm position is local system 
	//handles nouse interactions for translation and rotation
	void Scene::MouseProccessing(int button)
	{
		if(pickedShape == -1)
		{
			if(button == 1 )
			{				

				MyTranslate(glm::vec3(-xrel/20.0f,0,0),0);
				MyTranslate(glm::vec3(0,yrel/20.0f,0),0);
				WhenTranslate();
			}
			else
			{
				MyRotate(xrel/2.0f,glm::vec3(1,0,0),0);
				MyRotate(yrel/2.0f,glm::vec3(0,0,1),0);
				WhenRotate();
			}
		}
	}
	
	void Scene::ZeroShapesTrans()
	{
		for (unsigned int i = 0; i < shapes.size(); i++)
		{
			shapes[i]->ZeroTrans();
		}
	}
	
	void Scene::ReadPixel()
	{
		glReadPixels(1,1,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&depth);
	}
	//updates mouse position
	void Scene::UpdatePosition(float xpos, float ypos)
	{
		xrel = xold - xpos;
		yrel = yold - ypos;
		xold = xpos;
		yold = ypos;
	}

	void Scene::HideShape(int shpIndx)
	{
			shapes[shpIndx]->Hide();
	}

	void Scene::UnhideShape(int shpIndx)
	{
			
			shapes[shpIndx]->Unhide();
	}
	//clears the color and depth buffers
	void Scene::Clear(float r, float g, float b, float a)
	{
		glClearColor(r, g, b, a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	//destructor
	Scene::~Scene(void)
{
	for (Shape* shp : shapes)
		{
			delete shp;
		}
	for (Camera* cam : cameras)
		{
			delete cam;
		}
	for(Shader* sdr: shaders)
		{
			delete sdr;
		}
	for(Texture* tex: textures)
		{
			delete tex;
		}

}


	 
	