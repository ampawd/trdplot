#if defined(__APPLE__) && defined (TARGET_OS_IPHONE)
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#endif

#include "PVRShell/PVRShellNoPVRApi.h"
#include "PVRUIRenderer/PVRUIRenderer.h"

/*

TODO:

*/

#include <string>
#include <random>
#include <algorithm>
#include <chrono>

#include "shader_manager.h"
#include "trading_graph.h"

using namespace app_nmsp;


const glm::vec2 WIN_SIZE(1000, 700);
const pvr::int64 INNER_WIDTH = 7*WIN_SIZE.x;


class TradingApp : public pvr::Shell
{
	private:
		GLuint shaderProgramGrid, shaderProgramGraph;
		GLint vPositionLoc, vPositionLocGraph, mvpLoc, mvpLocGraph;
		ShaderManager shaderManager;
		TradingGraph tradingGraph;		
		
		glm::vec2 graphTranslation;
		glm::vec2 moveTranslation;
		glm::mat4 projection;
		glm::mat4 projectionModel;
		std::chrono::high_resolution_clock::time_point lastRenderTimePoint;
		std::chrono::high_resolution_clock::time_point currentTimePoint;
		std::pair<int, int> lastSegmentIndexes;		
		GLfloat moveXspeed;
		pvr::PointerLocation mouseLoc, mouseDown;
		//pvr::ui::UIRenderer uiRenderer;
		//pvr::ui::Text graphsNotesText;
		//pvr::api::AssetStore assetStore;
		//pvr::api::Fbo onScreenFbo;
		//pvr::api::CommandBuffer primaryCommandBuffer;
		//pvr::api::SecondaryCommandBuffer commandBufferText;

	public:
		virtual pvr::Result::Enum initApplication();
		virtual pvr::Result::Enum initView();
		virtual pvr::Result::Enum releaseView();
		virtual pvr::Result::Enum quitApplication();
		virtual pvr::Result::Enum renderFrame();
		void eventButtonDown(int);
};

pvr::Result::Enum TradingApp::initApplication()
{
	this->setAASamples(4);
	projection = glm::ortho(0.0f, WIN_SIZE.x, 0.0f, WIN_SIZE.y);
	lastSegmentIndexes.first = 0;
	lastSegmentIndexes.second = 1;
	moveXspeed = 0; 
	this->setDimensions(WIN_SIZE.x, WIN_SIZE.y);
	this->setTitle("Trading Graph test app");
	return pvr::Result::Success;
}

pvr::Result::Enum TradingApp::quitApplication()
{
	return pvr::Result::Success;
}

pvr::Result::Enum TradingApp::initView()
{
	Shader vshader(GL_VERTEX_SHADER, "shaders/vshader");
	Shader fshader_grid(GL_FRAGMENT_SHADER, "shaders/fshader_grid");
	Shader fshader_graph(GL_FRAGMENT_SHADER, "shaders/fshader_graph");

	if (pvr::Result::Enum::InvalidData == shaderManager.buildProgram(vshader, fshader_grid, &shaderProgramGrid))
	{
		this->setExitMessage(shaderManager.getErrorMessage().c_str());
		return pvr::Result::Enum::InvalidData;
	}

	if (pvr::Result::Enum::InvalidData == shaderManager.buildProgram(vshader, fshader_graph, &shaderProgramGraph))
	{
		this->setExitMessage(shaderManager.getErrorMessage().c_str());
		return pvr::Result::Enum::InvalidData;
	}

	shaderManager.use(shaderProgramGrid);
	vPositionLoc = glGetAttribLocation(shaderProgramGrid, "vPosition"),
	mvpLoc = glGetUniformLocation(shaderProgramGrid, "mvp");
	tradingGraph.setDimension(glm::vec2(INNER_WIDTH, WIN_SIZE.y));
	tradingGraph.initGrid();
	tradingGraph.setGridColor(glm::vec4(0.5, 0.5, 0.5, 0.0));
	tradingGraph.loadGrid(vPositionLoc);

	shaderManager.use(shaderProgramGraph);
	vPositionLocGraph = glGetAttribLocation(shaderProgramGraph, "vPosition");
	mvpLocGraph = glGetUniformLocation(shaderProgramGraph, "mvp");

	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_real_distribution<double> distribution(50.0f, WIN_SIZE.y / 1.5);
	auto graphFunc = [this, &generator, distribution](double x) mutable -> double
	{
		return distribution(generator);
	};
	
	tradingGraph.setGraphColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
	tradingGraph.setSMAGraphColor(glm::vec4(1.0, 0.0, 0.0, 1.0));
	tradingGraph.setEMAGraphColor(glm::vec4(1.0, 1.0, 0.0, 1.0));
	//tradingGraph.setStepX(50);
	tradingGraph.setSMAperiod(10);
	tradingGraph.setEMAperiod(10);
	tradingGraph.setFunction(graphFunc);

	tradingGraph.initGraphs(vPositionLocGraph);
	tradingGraph.loadGraphs();
	tradingGraph.setMoveToolColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
	tradingGraph.prepareMoveTool(vPositionLoc, WIN_SIZE);

	//assetStore.init(*this);
	//onScreenFbo = getGraphicsContext()->createOnScreenFbo(0);
	//uiRenderer.init(getGraphicsContext(), onScreenFbo->getRenderPass(), 0);

	//graphsNotesText = uiRenderer.createText("Hello, world");
	//graphsNotesText->setAnchor(pvr::ui::Anchor::TopLeft, glm::vec2(-1, 0));
	//graphsNotesText->setPixelOffset(30, -20);

	//primaryCommandBuffer = getGraphicsContext()->createCommandBufferOnDefaultPool();
	//commandBufferText = getGraphicsContext()->createSecondaryCommandBufferOnDefaultPool();

	lastRenderTimePoint = std::chrono::high_resolution_clock::now();		
	return pvr::Result::Success;
}

pvr::Result::Enum TradingApp::releaseView()
{
	tradingGraph.deleteBuffers();
	glDeleteProgram(shaderProgramGrid);

	//uiRenderer.release();
	//onScreenFbo.reset();
	//graphsNotesText.reset();
	//assetStore.releaseAll();
	//primaryCommandBuffer.reset();
	//commandBufferText.reset();

	return pvr::Result::Success;
}

void TradingApp::eventButtonDown(int idx)
{
	if (idx == 0)
	{
		mouseDown = getPointerAbsolutePosition();
	}
}

pvr::Result::Enum TradingApp::renderFrame()
{
	glClearColor(0.23f, 0.24f, 0.24f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//primaryCommandBuffer->beginRecording();
	//primaryCommandBuffer->beginRenderPass(onScreenFbo, pvr::Rectanglei(0, 0, getWidth(), getHeight()), false);	

	//commandBufferText->beginRecording(onScreenFbo->getRenderPass());
	//uiRenderer.beginRendering(commandBufferText);
	//graphsNotesText->render();
	//uiRenderer.endRendering();
	//commandBufferText->endRecording();
	//
	//primaryCommandBuffer->endRenderPass();
	//primaryCommandBuffer->endRecording();
	//primaryCommandBuffer->submit();

	shaderManager.use(shaderProgramGrid);

	if (getPointingDeviceState().isDragging())
	{
		mouseLoc = getPointingDeviceState().position();	
		graphTranslation.x -= mouseDown.x - mouseLoc.x;	//	tranbslate it to the left
		mouseDown.x = mouseLoc.x;
	}
	else
	{
		graphTranslation.x -= moveXspeed;
	}

	if (graphTranslation.x > 0)
	{
		graphTranslation.x = 0.0f;
	}
	if (graphTranslation.x < -INNER_WIDTH + WIN_SIZE.x)
	{
		graphTranslation.x = -INNER_WIDTH + WIN_SIZE.x;
	}

	tradingGraph.translate(graphTranslation);
	projectionModel = projection * tradingGraph.getTransformMatrix();
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(projectionModel));	
	tradingGraph.renderGrid(shaderProgramGrid);


	moveTranslation.x = getPointerAbsolutePosition().x - WIN_SIZE.x/2;
	moveTranslation.y = -(getPointerAbsolutePosition().y - WIN_SIZE.y/2);
	glm::mat4 moveModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(moveTranslation, 0.0f));
	glUniformMatrix4fv(mvpLocGraph, 1, GL_FALSE, glm::value_ptr(projection * moveModelMatrix));
	tradingGraph.renderMoveTool(shaderProgramGrid);


	shaderManager.use(shaderProgramGraph);
	glUniformMatrix4fv(mvpLocGraph, 1, GL_FALSE, glm::value_ptr(projectionModel));
	if (! tradingGraph.finishReached())
	{
		tradingGraph.renderGraphs(shaderProgramGraph, 0, lastSegmentIndexes.second);
	} 
	else
	{
		moveXspeed = 0;
		tradingGraph.renderWholeGraphs(shaderProgramGraph);
	}	

	currentTimePoint = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> time_span = 
		std::chrono::duration_cast<std::chrono::duration<double>>(currentTimePoint - lastRenderTimePoint);

	if (time_span.count() >= 1.0f)
	{
		lastRenderTimePoint = currentTimePoint;		
		
		if (tradingGraph.getCurrentX() >= WIN_SIZE.x + abs(graphTranslation.x))
		{
			moveXspeed = 1;
		}		
		lastSegmentIndexes.second++;
	}


	return pvr::Result::Success;
}

std::auto_ptr<pvr::Shell> pvr::newDemo()
{
	return std::auto_ptr<pvr::Shell>(new TradingApp());
}