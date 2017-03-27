#ifndef TRADING_GRAPH_H
#define TRADING_GRAPH_H

#include <GLES3/gl3.h>
#include <numeric>
#include <random>
#include "PVRShell/PVRShell.h"

namespace app_nmsp
{
	class TradingGraph
	{
		public:
			explicit TradingGraph();

			void loadGrid(GLint);
			void initGrid();
			void initGraphs(GLint);
			void loadGraphs();
			void prepareMoveTool(GLint, const glm::vec2&);
			void renderGrid(GLuint);
			void renderMoveTool(GLuint);
			void renderWholeGraphs(GLuint);
			void renderGraphs(GLuint, int, int);
			void setDimension(const glm::vec2&);
			void setOriginPosition(const glm::vec2&);
			void setGridColor(const glm::vec4&);
			void setGraphColor(const glm::vec4&);
			void setSMAGraphColor(const glm::vec4&);
			void setEMAGraphColor(const glm::vec4&);
			void setMoveToolColor(const glm::vec4&);
			void translate(const glm::vec2&);
			void setSMAperiod(const pvr::uint32&);
			void setEMAperiod(const pvr::uint32&);
			void deleteBuffers() const;
			void setFunction(const std::function<double(double)>);
			void setStepX(const double);
			bool finishReached() const;

			glm::mat4 getTransformMatrix() const;
			GLfloat getCurrentX() const;

		private:
			glm::vec4 gridColor, graphColor, SMAcolor, EMAcolor, moveToolColor;
			struct vertex
			{
				GLfloat x, y;
			};
			std::vector<vertex> gridVertices;
			std::vector<vertex> graphVertices;
			std::vector<vertex> SMAGraphVertices;
			std::vector<vertex> EMAGraphVertices;
			std::vector<GLfloat> moveToolVertices;
			glm::vec2 originPosition;
			glm::mat4 transformMatrix, translation, scale;
			glm::vec2 dimensions;
			double stepX;
			GLuint vbo_grid, vbo_graph, vbo_sma, vbo_ema, move_vbo;
			GLint posLocation, posLocationGraph, posLocMoveTool;
			GLfloat curveWidth;
			bool ended;
			std::function<double(double)> graphFunc;
			pvr::uint32 smaPeriod, emaPeriod;
			GLfloat currentX;
			
	};
};

#endif // !TRADING_GRAPH_H