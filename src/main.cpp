#include <iostream>
#include <cstdio>
#include <interface/interface.hpp>
#include <engine/renderer.hpp>
#include <unistd.h>

using namespace std;

int main(/* int argc, char* argv[] */)
{
	Interface* interface = new Interface();

	try
	{
		interface->GetInput();

		interface->DisplayStatus("Preparing render...", false);
		interface->Refresh();

		Renderer* renderer = new Renderer(interface->width, interface->height, interface->passes,
								 	      interface->platform, interface->device, interface->source, interface->output);

		interface->DisplayStatus("Rendering...", false);

		double progress, etc, elapsed;
		uint32_t triangles;
		while (!renderer->Execute())
		{
			progress = *(double*)renderer->Query(Query::Progress);
			etc = *(double*)renderer->Query(Query::EstimatedTime);
			elapsed = *(double*)renderer->Query(Query::ElapsedTime);
			triangles = *(uint32_t*)renderer->Query(Query::TriangleCount);
			
			interface->DisplayStatistics(elapsed, etc, progress, triangles);
		}

		interface->DisplayStatistics(elapsed, 0.0, 1.0, triangles);
		interface->DisplayStatus("Finalizing render...", false);
		interface->Refresh();
		delete renderer;
		interface->DisplayStatus("Render complete.", false);
	}
	catch (exception& e)
	{
		interface->DisplayStatus(e.what(), true);
	}

	interface->Pause();
	delete interface;

	return EXIT_SUCCESS;
}
