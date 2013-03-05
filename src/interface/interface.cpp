#include <interface/interface.hpp>

/* These are constants corresponding to lines on the interface terminal. */
#define LINE_PLATFORM   3
#define LINE_DEVICE     5
#define LINE_SCENEFILE  7
#define LINE_OUTPUTFILE 9
#define LINE_WIDTH      11
#define LINE_HEIGHT     13
#define LINE_PASSES     15
#define LINE_STATUS     17
#define LINE_ETC        19
#define LINE_PROGRESS   21
#define LINE_STATISTICS 23

/* These are constants defining specific color pairs. */
#define COLOR_NORMAL 1
#define COLOR_TITLE1 2
#define COLOR_TITLE2 3
#define COLOR_ERROR  4
#define COLOR_STATUS 5

Interface::Interface()
{
    window = initscr();

    if (has_colors())
    {
        start_color();
        init_pair(COLOR_NORMAL, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_TITLE1, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_TITLE2, COLOR_CYAN,  COLOR_BLACK);
        init_pair(COLOR_ERROR,  COLOR_RED,   COLOR_BLACK);
        init_pair(COLOR_STATUS, COLOR_GREEN, COLOR_BLACK);
    }

	DrawFrame();
}

void Interface::WriteLine(size_t line, std::string msg)
{
    std::string fmt = msg;

    /* Is the message too long? */
    if (msg.length() > ((line < 6) ? 44 : 60))
    {
        /* Truncate and add ellipsis. */
        fmt.erase((line < 6) ? 41 : 57, -1);
        fmt += "...";
    }

    /* Fill in with whitespace to avoid artifacts. */
    ssize_t missing = ((line < 6) ? 44 : 60) - fmt.length();
    for (ssize_t t = 0; t < missing; ++t) fmt += " ";

    mvprintw(line, 18, fmt.c_str());
}

void Interface::Redraw()
{
    refresh();
    doupdate();
}

void Interface::SetInput(bool input)
{
    curs_set((int)input);
    if (input) echo();
    else noecho();
}

void Interface::DisplayStatus(std::string message, bool error)
{
    /* If this is an error, draw it bright red to draw attention. */
    if (error) { attron(COLOR_PAIR(COLOR_ERROR)); attron(A_BOLD); }
    else { attron(COLOR_PAIR(COLOR_STATUS)); attroff(A_BOLD); }

    WriteLine(LINE_STATUS, message);
    Redraw();
}

void Interface::DrawFrame()
{
    box(this->window, 0, 0);

    /* Draw the horizontal lines. */
    for (size_t t = 2; t < 24; t += 2)
    {
        mvaddch(t, 0, ACS_LTEE);
        mvaddch(t, 79, ACS_RTEE);
        mvhline(t, 1, 0, 78);
    }

    /* Draw the vertical lines. */
    for (size_t t = 0; t < 25; ++t)
    {
        if (t % 2 == 0)
        {
            if ((t == 0) || (t == 24))
            {
                mvaddch(t, 16, (t == 0) ? ACS_TTEE : ACS_BTEE);
                if (t == 0) mvaddch(t, 63, ACS_TTEE);
            }
            else
            {
                mvaddch(t, 16, ACS_PLUS);
                if (t < 6) mvaddch(t, 63, ACS_PLUS);
                if (t == 6) mvaddch(t, 63, ACS_BTEE);
            }
        }
        else
        {
            mvaddch(t, 16, ACS_VLINE);
            if (t < 6) mvaddch(t, 63, ACS_VLINE);
        }
    }

    /* Write the title at the top of the terminal. */
    attron(COLOR_PAIR(COLOR_TITLE2)); attron(A_BOLD);
    mvprintw(1, 2, "   epsilon"); mvprintw(1, 65, "    v0.12");
    attron(COLOR_PAIR(COLOR_TITLE1)); attron(A_BOLD);
    mvprintw(1, 23, "Physically Based Spectral Renderer");

    /* Write all labels in their correct places. */
    attron(COLOR_PAIR(COLOR_NORMAL)); attron(A_BOLD);
    mvprintw(LINE_PLATFORM  , 2, "Host Platform");
    mvprintw(LINE_DEVICE    , 2, "OpenCL Device");
    mvprintw(LINE_SCENEFILE , 2, "Engine Source");
    mvprintw(LINE_OUTPUTFILE, 2, "Engine Output");
    mvprintw(LINE_WIDTH     , 2, "Render  Width");
    mvprintw(LINE_HEIGHT    , 2, "Render Height");
    mvprintw(LINE_PASSES    , 2, "Render Passes");
    mvprintw(LINE_STATUS    , 2, "Engine Status");
    mvprintw(LINE_ETC       , 2, "Completion In");
    mvprintw(LINE_PROGRESS  , 2, "Cur. Progress");
    mvprintw(LINE_STATISTICS, 2, "Engine Stats.");
}

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

/* Trims a string. */
void trim(std::string& str)
{
    std::string::size_type pos1 = str.find_first_not_of(' ');
    std::string::size_type pos2 = str.find_last_not_of(' ');
    str = str.substr(pos1 == std::string::npos ? 0 : pos1, 
                     pos2 == std::string::npos ? str.length() - 1
                     : pos2 - pos1 + 1);
}

bool Interface::GetInput()
{
    try {
    size_t platformIndex, deviceIndex;
    int key;

    /* Disable cursor/echo for device selection. */
    SetInput(false);

    /* Obtain the list of platforms.. */
    std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

    platformIndex = 0;
    DisplayStatus("Please select the OpenCL platform (left/right arrow keys).",
                  false);
   
    key = 0; 
    while (key != '\n')
    {
        std::string name;

        platform = platforms[platformIndex];
        size_t count = platforms.size();
		
        platform.getInfo(CL_PLATFORM_NAME, &name);
        trim(name);

        attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
        WriteLine(LINE_PLATFORM, name); /* Draw platforn name, and index. */
        mvprintw(LINE_PLATFORM, 66, "#%d out of %d", platformIndex + 1, count);

        Redraw();
        key = getch();

        /* Left key = previous platform, right key = next platform. */
        if ((key == KEY_LEFT) || (platformIndex > 0)) platformIndex--;
        if ((key == KEY_RIGHT) || (platformIndex < count - 1)) platformIndex++;
    }

    deviceIndex = 0;
    DisplayStatus("Please select the OpenCL device.", false);

    /* Get the list of devices... */
	std::vector<cl::Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    key = 0;
    while (key != '\n')
    {
        std::string name;

        device = devices[deviceIndex];
        size_t count = devices.size();
		
        device.getInfo(CL_DEVICE_NAME, &name);
		trim(name);

        attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
        WriteLine(LINE_DEVICE, name); /* Again, print device, and index. */
        mvprintw(LINE_DEVICE, 66, "#%d out of %d", deviceIndex + 1, count);

        Redraw();
        key = getch();

        if ((key == KEY_LEFT) || (deviceIndex > 0)) deviceIndex--;
        if ((key == KEY_RIGHT) || (deviceIndex < count - 1)) deviceIndex++;
    }

    SetInput(true);
    char input[61];

    /* Get scene directory. */
    DisplayStatus("Please enter the scene directory to use.", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    mvgetnstr(LINE_SCENEFILE, 18, input, 60);
    source = input;

    /* Get output file. */
    DisplayStatus("Please enter the output image.", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    mvgetnstr(LINE_OUTPUTFILE, 18, input, 60);
    output = input;

    /* Get render width and height. */
    DisplayStatus("Please enter the render width and height.", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    mvgetnstr(LINE_WIDTH, 18, input, 6);
    width = atoi(input);
    
    mvgetnstr(LINE_HEIGHT, 18, input, 6);
    height = atoi(input);

    /* Get the passes per pixel count. */
    DisplayStatus("Please enter the passes (per pixel) desired.", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    mvgetnstr(LINE_PASSES, 18, input, 8);
    passes = atoi(input);

    SetInput(false);
    } catch (...) { return false; }

    return ((source != "") && (output != "") && (width + height + passes > 0));
}

void Interface::DisplayProgress(double progress)
{
    /* Handle the discontinuity here. */
	size_t prog = (size_t)(progress * 54);
	size_t count = (prog > 27) ? prog + 6 : prog;

	attron(COLOR_PAIR(COLOR_TITLE1)); attron(A_BOLD);

    /* Progress bar drawn here. */
	for (size_t t = 0; t < count; ++t)
        mvaddch(LINE_PROGRESS, 18 + t, ACS_CKBOARD);

	mvprintw(LINE_PROGRESS, 45, " %.3d%% ", (int)(progress * 100));
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    Redraw();
}

/* Converts seconds to time HHH:MM:SS. */
std::string TimeToString(size_t seconds)
{
    size_t minutes = (seconds % 3600) / 60;
    size_t hours = seconds / 3600;
    seconds = seconds % 60;

    std::stringstream stream;
    stream << std::setfill('0') << std::setw(3) << hours << ":";
    stream << std::setfill('0') << std::setw(2) << minutes << ":";
    stream << std::setfill('0') << std::setw(2) << seconds;
    return stream.str();
}

void Interface::DisplayTime(double remains, double elapsed)
{
	attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
	if (remains < 0.0)
    {
        /* Not enough data to estimate time. */
        WriteLine(LINE_ETC, "Calculating...");
    }
	else
	{
        std::string R = TimeToString((size_t)(remains + 0.5));
        std::string E = TimeToString((size_t)(elapsed + 0.5));

        std::string display = "         [" + R + " remains]    ["
                            + E + " elapsed]";
        WriteLine(LINE_ETC, display);
	}
}

void Interface::GiveStatistics(Statistics statistics)
{
	DisplayProgress(statistics.progress);
    
    /* This is just to make sure we don't get issues. */
    if (statistics.progress == 1.0) statistics.remains = 0.0;

	std::stringstream ss;
	ss << "Triangles: " << statistics.tris << ", ";

	/* Is there enough data? */
	if (statistics.elapsed < 1)
	{
		ss << "no statistics available.";
		attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
		WriteLine(LINE_STATISTICS, ss.str());
	}
	else
	{
        /* Compute the pass speed (over all pixels) and the total number of
         * pixel passes (those quantities are related, the latter is in fact
         * independent of render width/height and so more accurate). */
		double done = statistics.progress / statistics.elapsed;
		double pass_speed = passes * done;
		double speed = width * height * pass_speed * 1e-6;

		ss << std::fixed;
        ss.precision(1);

        ss << pass_speed << " passes/second [" << speed << " MPP/s]";
		attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
		WriteLine(LINE_STATISTICS, ss.str());
	}

	DisplayTime(statistics.remains, statistics.elapsed);
    Redraw();
}

void Interface::Pause()
{
    getch();
}

Interface::~Interface()
{
    endwin();
}

