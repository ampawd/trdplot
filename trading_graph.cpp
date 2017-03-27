#include "trading_graph.h"

using namespace app_nmsp;

TradingGraph::TradingGraph()
{
	stepX = 50;
	gridColor.r = 0.5;  gridColor.b = 0.5;  gridColor.b = 0.5; gridColor.a = 1.0;
	graphColor.r = 1.0; graphColor.b = 1.0; graphColor.b = 1.0; graphColor.a = 1.0;
	SMAcolor.r = 1.0; SMAcolor.g = 0.0; SMAcolor.b = 0.0; SMAcolor.a = 1.0;
	EMAcolor.r = 1.0; EMAcolor.g = 1.0; EMAcolor.b = 0.0; EMAcolor.a = 1.0;
	moveToolColor.r = 1.0; moveToolColor.g = 1.0; moveToolColor.b = 1.0; moveToolColor.a = 1.0;
	smaPeriod = 5;
	emaPeriod = 5;
	curveWidth = 1.6;
	ended = false;
}

void TradingGraph::loadGrid(GLint posLocation)
{
	if (gridVertices.empty())
		return;

	this->posLocation = posLocation;

	glGenBuffers(1, &vbo_grid);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_grid);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * gridVertices.size(), &gridVertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(posLocation, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
	glEnableVertexAttribArray(posLocation);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TradingGraph::initGrid()
{
	long x, y;
	for (x = 0; x < dimensions.x; x += stepX)
	{
		for (y = 0; y < dimensions.y; y += stepX / 1.5)
		{
			vertex v1, v2, v3, v4;
			//	vertical line
			v1.x = static_cast<GLfloat>(x + originPosition.x);
			v1.y = static_cast<GLfloat>(dimensions.y + originPosition.y);
			v2.x = static_cast<GLfloat>(x + originPosition.x);
			v2.y = static_cast<GLfloat>(-dimensions.y + originPosition.y);

			//	horizontal line
			v3.x = static_cast<GLfloat>(dimensions.x + originPosition.x);
			v3.y = static_cast<GLfloat>(y + originPosition.y);
			v4.x = static_cast<GLfloat>(-dimensions.x + originPosition.x);
			v4.y = static_cast<GLfloat>(y + originPosition.y);

			gridVertices.push_back(v1);	gridVertices.push_back(v2);
			gridVertices.push_back(v3);	gridVertices.push_back(v4);
		}
	}
}

void TradingGraph::renderGrid(GLuint programm)
{
	transformMatrix = translation * scale;
	glLineWidth(0.1);
	glUniform4f(glGetUniformLocation(programm, "gridColor"), gridColor.r, gridColor.g, gridColor.b, gridColor.a);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_grid);
	glVertexAttribPointer(posLocation, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
	glDrawArrays(GL_LINES, 0, gridVertices.size());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TradingGraph::prepareMoveTool(GLint posLocation, const glm::vec2& win_size)
{
	this->posLocMoveTool = posLocation;
	moveToolVertices = 
	{ 
		-win_size.x * 3, win_size.y / 2, win_size.x * 3, win_size.y / 2,	//	horizontal line
		win_size.x / 2, win_size.y * 3, win_size.x / 2, -win_size.y * 3		//	vertical line
	};

	glGenBuffers(1, &move_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, move_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * moveToolVertices.size(), &moveToolVertices[0], GL_STATIC_DRAW);	
	glVertexAttribPointer(this->posLocMoveTool, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(this->posLocMoveTool);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TradingGraph::renderMoveTool(GLuint programm)
{
	glUniform4f(glGetUniformLocation(programm, "gridColor"), moveToolColor.r, moveToolColor.g, moveToolColor.b, moveToolColor.a);
	glBindBuffer(GL_ARRAY_BUFFER, move_vbo);
	glVertexAttribPointer(this->posLocMoveTool, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid *)0);
	glDrawArrays(GL_LINES, 0, moveToolVertices.size() / 2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool TradingGraph::finishReached() const
{
	return ended;
}

GLfloat TradingGraph::getCurrentX() const
{
	return currentX;
}

void TradingGraph::initGraphs(GLint posLocation)
{
	posLocationGraph = posLocation;
	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_int_distribution<int> distrubition(0, stepX);	
	for (long x = 0, to = dimensions.x; x < to; x += distrubition(generator))
	{
		const double y = graphFunc(x);
		vertex v;
		v.x = x; 
		v.y = y;
		graphVertices.push_back(v);
	}

	//	compute moving averages
	long index = 0; 
	double alpha = 2.0f / (emaPeriod + 1);
	bool first = true;
	GLfloat EMA_PREV;
	for (auto iter = graphVertices.begin(); iter != graphVertices.end(); ++iter)
	{
		vertex sma_v, ema_v;
		sma_v.x = iter->x;		
		ema_v.x = iter->x;
		if (index < smaPeriod)
		{
			sma_v.y = iter->y;
		}
		else
		{
			const GLfloat SMA = std::accumulate(iter - smaPeriod + 1, iter + 1, 0,
				[](GLfloat y, const vertex& v) -> GLfloat {
					return y + v.y;
				}
			) / smaPeriod;

			sma_v.y = SMA;
		}

		SMAGraphVertices.push_back(sma_v);
		if (first)
		{
			EMA_PREV = iter->y;
			first = false;
		}

		GLfloat EMA = alpha * iter->y + (1 - alpha) * EMA_PREV;
		EMA_PREV = EMA;

		ema_v.y = EMA;
		EMAGraphVertices.push_back(ema_v);
		++index;
	}
}

void TradingGraph::loadGraphs()
{
	if (graphVertices.empty() || SMAGraphVertices.empty() || EMAGraphVertices.empty())
	{
		return;
	}

	glGenBuffers(1, &vbo_graph);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_graph);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * graphVertices.size(), &graphVertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(this->posLocationGraph, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
	glEnableVertexAttribArray(this->posLocationGraph);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_sma);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sma);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * SMAGraphVertices.size(), &SMAGraphVertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(this->posLocationGraph, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
	glEnableVertexAttribArray(this->posLocationGraph);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &vbo_ema);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_ema);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * EMAGraphVertices.size(), &EMAGraphVertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(this->posLocationGraph, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
	glEnableVertexAttribArray(this->posLocationGraph);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TradingGraph::renderWholeGraphs(GLuint programm)
{
	glLineWidth(curveWidth);
	glUniform4f(glGetUniformLocation(programm, "graphColor"), graphColor.r, graphColor.g, graphColor.b, graphColor.a);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_graph);
	glVertexAttribPointer(this->posLocationGraph, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
	glDrawArrays(GL_LINE_STRIP, 0, graphVertices.size());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glLineWidth(1);
	glUniform4f(glGetUniformLocation(programm, "graphColor"), SMAcolor.r, SMAcolor.g, SMAcolor.b, SMAcolor.a);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sma);
	glVertexAttribPointer(this->posLocationGraph, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
	glDrawArrays(GL_LINE_STRIP, 0, SMAGraphVertices.size());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glLineWidth(1);
	glUniform4f(glGetUniformLocation(programm, "graphColor"), EMAcolor.r, EMAcolor.g, EMAcolor.b, EMAcolor.a);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_ema);
	glVertexAttribPointer(this->posLocationGraph, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
	glDrawArrays(GL_LINE_STRIP, 0, EMAGraphVertices.size());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TradingGraph::renderGraphs(GLuint programm, int first, int second)
{
	if (second >= graphVertices.size())
	{
		ended = true;
	}

	if (second < graphVertices.size())
	{
		currentX = graphVertices[second].x;
	}

	glLineWidth(curveWidth);
	glUniform4f(glGetUniformLocation(programm, "graphColor"), graphColor.r, graphColor.g, graphColor.b, graphColor.a);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_graph);
	glVertexAttribPointer(this->posLocationGraph, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
	glDrawArrays(GL_LINE_STRIP, first, second);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glLineWidth(1);
	glUniform4f(glGetUniformLocation(programm, "graphColor"), SMAcolor.r, SMAcolor.g, SMAcolor.b, SMAcolor.a);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sma);
	glVertexAttribPointer(this->posLocationGraph, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
	glDrawArrays(GL_LINE_STRIP, first, second);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glLineWidth(1);
	glUniform4f(glGetUniformLocation(programm, "graphColor"), EMAcolor.r, EMAcolor.g, EMAcolor.b, EMAcolor.a);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_ema);
	glVertexAttribPointer(this->posLocationGraph, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid *)0);
	glDrawArrays(GL_LINE_STRIP, first, second);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TradingGraph::setFunction(const std::function<double(double)> func)
{
	graphFunc = func;
}

void TradingGraph::setStepX(const double newStepX)
{
	stepX = newStepX;
}

void TradingGraph::setDimension(const glm::vec2 & newDimensions)
{
	dimensions = newDimensions;
}

void TradingGraph::setOriginPosition(const glm::vec2& newOriginPosition)
{
	originPosition.x = newOriginPosition.x;
	originPosition.y = newOriginPosition.y;
}

void TradingGraph::setGridColor(const glm::vec4& newGridColor)
{
	gridColor = newGridColor;
}

void TradingGraph::setGraphColor(const glm::vec4& newGraphColor)
{
	graphColor = newGraphColor;
}

void TradingGraph::setSMAGraphColor(const glm::vec4& newSMAGraphColor)
{
	SMAcolor = newSMAGraphColor;
}

void TradingGraph::setEMAGraphColor(const glm::vec4& newEMAGraphColor)
{
	EMAcolor = newEMAGraphColor;
}

void TradingGraph::setMoveToolColor(const glm::vec4& newMoveToolColor)
{
	moveToolColor = newMoveToolColor;
}

void TradingGraph::translate(const glm::vec2& translationVector)
{
	translation = glm::translate(glm::mat4(1.0f), glm::vec3(originPosition + translationVector, 1.0));
}

void TradingGraph::setSMAperiod(const pvr::uint32& newSmaPeriod)
{
	smaPeriod = newSmaPeriod < 2 ? 2 : newSmaPeriod > 50 ? 50 : newSmaPeriod;
}

void TradingGraph::setEMAperiod(const pvr::uint32& newEmaPeriod)
{
	emaPeriod = newEmaPeriod < 2 ? 2 : newEmaPeriod > 50 ? 50 : newEmaPeriod;
}

glm::mat4 TradingGraph::getTransformMatrix() const
{
	return transformMatrix;
}

void TradingGraph::deleteBuffers() const
{
	glDeleteBuffers(1, &vbo_grid);
	glDeleteBuffers(1, &vbo_graph);
	glDeleteBuffers(1, &vbo_sma);
	glDeleteBuffers(1, &vbo_ema);
	glDeleteBuffers(1, &move_vbo);
}