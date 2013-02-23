#pragma once
#include <common/common.hpp>

class Interface
{
	public:
		WINDOW* window;
        std::string source, output;
		double progress;
		cl::Platform platform;
		cl::Device device;
		size_t passes, width, height;

        void WriteLine(size_t line, std::string msg);
		
        Interface();
		~Interface();

        void DisplayStatus(std::string message, bool error);
		void DisplayProgress();
		void DisplayTime(double etc, double elapsed);
		void DisplayStatistics(double elapsed,
                               double estimated,
                               double progress,
                               uint32_t triangles);

		void DrawFrame();
        void GetInput();
		void Refresh();
		void Pause();
};
