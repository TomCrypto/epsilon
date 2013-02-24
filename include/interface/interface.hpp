#pragma once

#include <common/error.hpp>

#include <CL/cl.hpp>
#include <ncurses.h>
#include <iomanip>
#include <sstream>

/** @file interface.hpp
  * @brief Interface manipulation.
**/

/** @struct Statistics
  * @brief Engine statistics.
  *
  * This structure contains statistics to be filled by the engine, such that
  * the interface can then display them, properly formatted.
**/
struct Statistics
{
    /** @brief The renderer's progress, from 0 to 1. **/
    double progress;
    /** @brief The elapsed render time, in seconds. **/
    double elapsed;
    /** @brief The estimated remaining render time, in seconds. **/
    double estimated;
    /** @brief The number of triangles in the scene. **/
    uint32_t triangles;
};

/** @class Interface
  * @brief Console User Interface.
  *
  * This class manages a curses-based console user interface (CUI) which lets
  * the user select the engine parameters, and provides clear visual feedback
  * including render progress (as a pretty progress bar) and statistics.
**/
class Interface
{
    private:
        /** @brief The underlying curses window. **/
        WINDOW* window;

	public:
        /** Engine source (the directory containing the scene). **/
        std::string source;
        /** Engine output (where to save the final render). **/
        std::string output;

		double progress;

        /** OpenCL platform to use for rendering. **/
		cl::Platform platform;
        /** OpenCL device to use for rendering. **/
		cl::Device device;
        /** Render width, in pixels. **/
        size_t width;
        /** Render height, in pixels. **/
        size_t height;
        /** Number of render passes. **/
        size_t passes;

        void WriteLine(size_t line, std::string msg);
		
        Interface();
		~Interface();

        void DisplayStatus(std::string message, bool error);
		void DisplayProgress();
		void DisplayTime(double etc, double elapsed);

		void GiveStatistics(Statistics statistics);

		void DrawFrame();
        void GetInput();
		void Refresh();
		void Pause();
};
