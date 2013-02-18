#include <interface/interface.hpp>

/* These are constants corresponding to lines on the interface terminal. */
#define LINE_PLATFORM   3
#define LINE_DEVICE     5
#define LINE_SCENEFILE  7
#define LINE_OUTPUTFILE 9
#define LINE_WIDTH      11
#define LINE_HEIGHT     13
#define LINE_SAMPLES    15
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
    this->window = initscr();
    this->DrawFrame();


    /* Init colors. */
    if (has_colors())
    {
        start_color();
        init_pair(COLOR_NORMAL, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_TITLE1, COLOR_GREEN, COLOR_BLACK);
        init_pair(COLOR_TITLE2, COLOR_CYAN,  COLOR_BLACK);
        init_pair(COLOR_ERROR,  COLOR_RED,   COLOR_BLACK);
        init_pair(COLOR_STATUS, COLOR_YELLOW, COLOR_BLACK);
    }

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

    size_t missing = ((line < 6) ? 41 : 57) - fmt.length();
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
    mvprintw(LINE_PLATFORM  , 2, "OpenCL Platfm");
    mvprintw(LINE_DEVICE    , 2, "OpenCL Device");
    mvprintw(LINE_SCENEFILE , 2, "   Scene File");
    mvprintw(LINE_OUTPUTFILE, 2, "  Output File");
    mvprintw(LINE_WIDTH     , 2, " Render Width");
    mvprintw(LINE_HEIGHT    , 2, "Render Height");
    mvprintw(LINE_SAMPLES   , 2, "Samples/Pixel");
    mvprintw(LINE_STATUS    , 2, "Render Status");
    mvprintw(LINE_ETC       , 2, "Completion In");
    mvprintw(LINE_PROGRESS  , 2, "Avg. Progress");
    mvprintw(LINE_STATISTICS, 2, "   Statistics");
}

/* This method will ask the user for the required inputs to the renderer, using
 * the ncurses blocking getch. */
void Interface::GetInput()
{
    this->deviceList.Initialize();

    /* Don't want visual feedback for platform/device selection. */
    curs_set(0);
    noecho();

    /* Get platform. */
    this->platform = 0;
    DisplayStatus("Please select the OpenCL platform (up/down arrow keys).",
                  false);
   
    int input = 0; 
    while (input != '\n')
    {
        Platform platform = this->deviceList.platforms[this->platform];
        size_t count = this->deviceList.platforms.size();

        attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
        WriteLine(LINE_PLATFORM, platform.name);
        mvprintw(LINE_PLATFORM, 66, "#%d out of %d",
                 this->platform + 1, count);

        doupdate();
        input = getch();

        if ((input == KEY_DOWN) || (this->platform > 0)) this->platform--;
        if ((input == KEY_UP) || (this->platform > count)) this->platform++;
    }

    /* Get device. */
    this->device = 0;
    DisplayStatus("Please select the OpenCL device.", false);

    input = 0;
    while (input != '\n')
    {
        Platform platform = this->deviceList.platforms[this->platform];
        size_t count = platform.devices.size();

        attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
        WriteLine(LINE_DEVICE, platform.devices[this->device].name);
        mvprintw(LINE_DEVICE, 66, "#%d out of %d",
                 this->device + 1, count);

        doupdate();
        input = getch();

        if ((input == KEY_DOWN) || (this->device > 0)) this->device--;
        if ((input == KEY_UP) || (this->device > count)) this->device++;
    }

    /* We want visual feedback again. */
    curs_set(1);
    echo();

    char text[61];

    /* Get scene file. */
    DisplayStatus("Please enter the scene file. (WIP: LEAVE EMPTY)", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    move(LINE_SCENEFILE, 18);
    getnstr(text, 60);
    this->sceneFile = text;

    /* Get output file. */
    DisplayStatus("Please enter the output file.", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    move(LINE_OUTPUTFILE, 18);
    getnstr(text, 60);
    this->outputFile = text;

    /* Get render width and height. */
    DisplayStatus("Please enter the render width and height.", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    move(LINE_WIDTH, 18);
    getnstr(text, 6);
    this->width = atoi(text);
    move(LINE_HEIGHT, 18);
    getnstr(text, 6);
    this->height = atoi(text);

    /* Get the sample per pixel count. */
    DisplayStatus("Please enter the samples per pixel desired.", false);
    attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
    move(LINE_SAMPLES, 18);
    getnstr(text, 8);
    this->samples = atoi(text);

    /* We're done with input. */
    curs_set(0);
    noecho();

    /* Do some basic error checking. */
    size_t check = this->width * this->height;
    if ((this->samples == 0) || ((check & (check - 1)) != 0))
    {
        throw std::runtime_error("Invalid parameters provided.");
    }
}

void Interface::DisplayProgress()
{
	attron(COLOR_PAIR(COLOR_NORMAL)); attroff(A_BOLD);
	mvprintw(LINE_PROGRESS, 46, "%.3d%%", (int)(progress * 100));
	refresh();
	doupdate();
}

void Interface::Redraw()
{
    int c = getch();
}

Interface::~Interface()
{
    endwin();
}

