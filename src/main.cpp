#include <interface/interface.hpp>
#include <engine/renderer.hpp>

#include <unistd.h>
#include <cstdlib>
#include <cstdio>

#ifdef _WIN32
#undef interface
#endif

/* To ensure log cleanly ends, fd automatically closed. */
void end(void) { fprintf(stderr, "\n--- END LOG ---\n"); }

/* This function will query the statistics from the renderer. */
void QueryStatistics(Renderer* renderer, Statistics& statistics)
{
    statistics.progress = *(double*)renderer->Query(Query::Progress);
    statistics.remains  = *(double*)renderer->Query(Query::EstimatedTime);
    statistics.elapsed  = *(double*)renderer->Query(Query::ElapsedTime);
    statistics.tris = *(uint32_t*)renderer->Query(Query::TriangleCount);
}

int main(/* int argc, char* argv[] */)
{
    /* Create an error log to stderr. */
    FILE* log = fopen("error.log", "w");
    dup2(fileno(log), 2);
    atexit(end);

    fprintf(stderr, "--- BEGIN LOG ---\n\n");

    /* Enumerate some build information here. */
    #ifdef _WIN32
    fprintf(stderr, "[+] Running on Windows.\n");
    #elif __linux__
    fprintf(stderr, "[+] Running on Linux.\n");
    #else
    #error Unrecognized OS. Override at your own risk.
    #endif

    #ifdef LOW_RES_TIMER
    fprintf(stderr, "[+] Using low-resolution timer.\n");
    #else
    fprintf(stderr, "[+] Using high-resolution timer.\n");
    #endif

    fprintf(stderr, "\n[+] Initializing interface.\n");
    Interface* interface = new Interface();
    Renderer* renderer = nullptr;

    try
    {
        fprintf(stderr, "[+] Waiting for user input.\n");
        if (!interface->GetInput())
            throw std::runtime_error("Invalid parameters.");

        interface->DisplayStatus("Preparing render...", false);

        fprintf(stderr, "[+] Initializing renderer.\n\n");
        renderer = new Renderer(interface->width,
                                interface->height,
                                interface->passes,
                                interface->platform,
                                interface->device,
                                interface->source,
                                interface->output);

        interface->DisplayStatus("Rendering...", false);

        fprintf(stderr, "\n[+] Starting render passes.\n\n");

        Statistics statistics;
        QueryStatistics(renderer, statistics);
        interface->GiveStatistics(statistics);
        
        do
        {
            QueryStatistics(renderer, statistics);
            interface->GiveStatistics(statistics);
        }
        while (!renderer->Execute());

        QueryStatistics(renderer, statistics);
        interface->GiveStatistics(statistics);

        interface->DisplayStatus("Finalizing render...", false);
        fprintf(stderr, "[+] Finalizing render.\n\n");

        delete renderer;

        fprintf(stderr, "\n[+] Renderer successfully terminated.\n");
        interface->DisplayStatus("Render complete.", false);
    }
    catch (std::exception& e)
    {
        /* Print error to screen and into log. */
        interface->DisplayStatus(e.what(), true);
        fprintf(stderr, "\n[EXCEPTION RAISED] ");
        fprintf(stderr, "-> %s\n\n", e.what());
        delete renderer;
        fprintf(stderr, "[+] Killed.\n");
    }

    interface->Pause();
    delete interface;
    return 0;
}
