#pragma once
#include <common/common.hpp>
#include <devices/devices.hpp>

class Interface
{
	public:
		WINDOW* window;
        std::string source, output;
		double progress;
		cl::Platform platform;
		cl::Device device;
		size_t platformIndex, deviceIndex;
		size_t samples, width, height;

        DeviceList deviceList;
        void WriteLine(size_t line, std::string msg);
		
        Interface();
		~Interface();

        void DisplayStatus(std::string message, bool error);
		void DisplayProgress();
		void DisplayTime(double etc);

		void DrawFrame();
        void GetInput();
		void Redraw();
};
