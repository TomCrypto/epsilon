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

/* This method will write an arbitrary message to a particular line. The method
 * will also truncate messages which are too long, using an ellipsis.        */
void Interface::WriteLine(size_t line, std::string msg)
{
    std::string fmt = msg;

    if (msg.length() > ((line < 6) ? 44 : 60))
    {
        fmt.erase((line < 6) ? 41 : 57, -1);
        fmt += "...";
    }

    size_t missing = ((line < 6) ? 44 : 60) - fmt.length();
    for (size_t t = 0; t < missing; ++t) fmt += " ";

    mvprintw(line, 18, fmt.c_str());
}

/* Updates the "Render Status" field in the interface and colors it accordingly
 * depending on whether its an error or not. Truncates message if too long.  */
void Interface::DisplayStatus(std::string message, bool error)
{
    if (error) { attron(COLOR_PAIR(COLOR_ERROR)); attron(A_BOLD); }
    else { attron(COLOR_PAIR(COLOR_STATUS)); attroff(A_BOLD); }

    WriteLine(LINE_STATUS, message);

	if (error)
	{
		curs_set(0);
	    noecho();
	}

    refresh();
    doupdate();
}

/* Draws the constant elements of the interface once and for all. This includes
 * borders and other elements which will never change throughout execution.  */
void Interface::DrawFrame()
{
    box(this->window, 0, 0);

    for (size_t t = 2; t < 24; t += 2)
    {
        /* Horizontal lines. */
        mvaddch(t, 0, ACS_LTEE);
        mvaddch(t, 79, ACS_RTEE);
        mvhline(t, 1, 0, 78);
    }

    for (size_t t = 0; t < 25; ++t)
    {
        /* Vertical lines. */
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

    /* Write the title at the top. */
    attron(COLOR_PAIR(COLOR_TITLE2)); attron(A_BOLD);
    mvprintw(1, 2, "epsilon v0.10"); mvprintw(1, 65, "epsilon v0.10");
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

// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

/* This method will ask the user for the required inputs to the renderer, using
 * the ncurses blocking getch. */
void Interface::GetInput()
{
    size_t platformIndex, deviceIndex;
    int key;

    /* Disable cursor/echo for device selection. */
    curs_set(0);
    noecho();

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

        attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
        WriteLine(LINE_PLATFORM, name);

        mvprintw(LINE_PLATFORM, 66, "#%d out of %d", platformIndex + 1, count);

        Refresh();
        key = getch();

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
		name = trim(name);

        attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
        WriteLine(LINE_DEVICE, name);

        mvprintw(LINE_DEVICE, 66, "#%d out of %d", deviceIndex + 1, count);

        Refresh();
        key = getch();

        if ((key == KEY_LEFT) || (deviceIndex > 0)) deviceIndex--;
        if ((key == KEY_RIGHT) || (deviceIndex < count - 1)) deviceIndex++;
    }

    /* Re-enable visual feedback. */
    curs_set(1);
    echo();

    char input[61];

    /* Get scene file. */
    DisplayStatus("Please enter the scene to render.", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    move(LINE_SCENEFILE, 18);
    getnstr(input, 60);
    this->source = input;

    /* Get output file. */
    DisplayStatus("Please enter the output file.", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    move(LINE_OUTPUTFILE, 18);
    getnstr(input, 60);
    this->output = input;

    /* Get render width and height. */
    DisplayStatus("Please enter the render width and height.", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    move(LINE_WIDTH, 18);
    getnstr(input, 6);
    this->width = atoi(input);
    move(LINE_HEIGHT, 18);
    getnstr(input, 6);
    this->height = atoi(input);

    /* Get the passes per pixel count. */
    DisplayStatus("Please enter the passes (per pixel) desired.", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    move(LINE_PASSES, 18);
    getnstr(input, 8);
    this->passes = atoi(input);

    /* We're done with input. */
    curs_set(0);
    noecho();

    /* Do some basic error checking. */
    if (passes == 0) throw std::runtime_error("Invalid parameters provided.");
}

void Interface::DisplayProgress()
{
    /* Handle the discontinuity here. */
	size_t prog = (size_t)(progress * 54);
	size_t count = (prog > 27) ? prog + 6 : prog;

	attron(COLOR_PAIR(COLOR_TITLE1)); attron(A_BOLD);

    /* Progress bar drawn here. */
	for (size_t t = 0; t < count; ++t)
        mvaddch(LINE_PROGRESS, 18 + t, ACS_CKBOARD);

	attron(COLOR_PAIR(COLOR_TITLE1)); attron(A_BOLD);

	mvprintw(LINE_PROGRESS, 45, " %.3d%% ", (int)(progress * 100));

    Refresh();
}

void Interface::DisplayTime(double etc, double elapsed)
{
	attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
	if ((etc < 0.0) && (progress != 1.0)) WriteLine(LINE_ETC, "Calculating...");
	else
	{
		if (progress == 1.0) etc = 0.0;
		size_t t = (size_t)(etc - 0.5);
		int etc_h = t / 3600;
		int etc_m = (t % 3600) / 60;
		int etc_s = t % 60;

		t = (size_t)(elapsed - 0.5);
		int elapsed_h = t / 3600;
		int elapsed_m = (t % 3600) / 60;
		int elapsed_s = t % 60;

		std::stringstream ss;
		ss << "         [";
		ss << std::setfill('0') << std::setw(3);
		ss << etc_h << ":";
		ss << std::setfill('0') << std::setw(2);
		ss << etc_m << ":";
		ss << std::setfill('0') << std::setw(2);
		ss << etc_s;
		ss << " remains]    [";
		ss << std::setfill('0') << std::setw(3);
		ss << elapsed_h << ":";
		ss << std::setfill('0') << std::setw(2);
		ss << elapsed_m << ":";
		ss << std::setfill('0') << std::setw(2);
		ss << elapsed_s;
		ss << " elapsed]";

		WriteLine(LINE_ETC, ss.str());
	}
}

void Interface::GiveStatistics(Statistics statistics)
{
	this->progress = statistics.progress;
	DisplayProgress();

	/* Is there enough time data? */
	if (statistics.elapsed < 1)
	{
		std::stringstream ss;
		ss << "Triangles: " << statistics.triangles << ", no statistics available.";
		attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
		WriteLine(LINE_STATISTICS, ss.str());
	}
	else
	{
		/* Total samples so far = progress * width * height. */
		/* Total samples per second = total samples / second. */
		double pass_speed = this->passes * statistics.progress / statistics.elapsed;
		double speed = this->passes * statistics.progress * this->width * this->height / statistics.elapsed;
		speed /= (1000 * 1000);

		std::stringstream ss;
		ss.precision(1);
		ss << std::fixed;
		ss << "Triangles: " << statistics.triangles << ", " << pass_speed << " passes/second [" << speed << " MPP/s]";
		attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
		WriteLine(LINE_STATISTICS, ss.str());
	}

	DisplayTime(statistics.estimated, statistics.elapsed);
    refresh();
    doupdate();
}

void Interface::Refresh()
{
    refresh();
	doupdate();
}

void Interface::Pause()
{
    getch();
}

Interface::~Interface()
{
    endwin();
}

