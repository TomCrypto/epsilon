#pragma once

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
    double remains;
    /** @brief The number of triangles in the scene. **/
    uint32_t tris;
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

        /** @brief Draws the interface outline. **/
        void DrawFrame();

        /** @brief Draws the progress bar.
          * @param progress Renderer progress, from 0 to 1.
        **/
        void DisplayProgress(double progress);

        /** @brief Displays the remaining and elapsed time.
          * @param remains The remaining time.
          * @param elapsed The elapsed time.
        **/
        void DisplayTime(double remains, double elapsed);


        /** @brief Writes a string at a given line.
          * @param line The line to write the string at.
          * @param msg The message to write.
        **/
        void WriteLine(size_t line, std::string msg);

        /** @brief Redraws the curses window. **/
        void Redraw();

	public:
        /** @brief Engine source (the directory containing the scene). **/
        std::string source;
        /** @brief Engine output (where to save the final render). **/
        std::string output;
        /** @brief OpenCL platform to use for rendering. **/
		cl::Platform platform;
        /** @brief OpenCL device to use for rendering. **/
		cl::Device device;
        /** @brief Render width, in pixels. **/
        size_t width;
        /** @brief Render height, in pixels. **/
        size_t height;
        /** @brief Number of render passes. **/
        size_t passes;

        /** @brief Constructs the interface and sets it up. **/
        Interface();

        /** @brief Frees the interface and all associated resources. **/
		~Interface();

        /** @brief Enables or disables input.
          * @param input If \c true, input is enabled.
        **/
        void SetInput(bool input);

        /** @brief Displays a message in the "render status" field.
          * @param msg The message to display.
          * @param error If \c true, the message will appear red.
          * @note If the message is too long, it will be truncated with an
          *       ellipsis.
        **/
        void DisplayStatus(std::string msg, bool error);
		
        /** @brief Takes statistics from the engine and displays them.
          * @param statistics The engine statistics.
        **/
		void GiveStatistics(Statistics statistics);

        /** @brief Lets the user select engine parameters.
          * @return Returns \c false if something went wrong, \c true
          *         otherwise. **/
        bool GetInput();

        /** @brief Waits for any key press from the user. **/
		void Pause();
};
