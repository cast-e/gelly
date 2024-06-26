#include <gelly-cpu-refs/Logging.h>
#include <raylib.h>

#include <cstdio>

#include "IVisualizer.h"
#include "WindowConstants.h"
#include "visualizers/CMarchingCubesVisualizer.h"
#include "visualizers/Example.h"

static IVisualizer *g_visualizer = nullptr;

void ChangeVisualizer(IVisualizer *newVisualizer) {
	GCR_LOG_INFO(
		"Swapping the '%s' visualizer for '%s'",
		g_visualizer ? g_visualizer->GetName() : "none",
		newVisualizer->GetName()
	);

	delete g_visualizer;
	g_visualizer = newVisualizer;
}

void Render() {
	ClearBackground(BLACK);
	DrawText("gelly cpu ref visualizer", 10, 10, 20, WHITE);
	DrawFPS(WINDOW_WIDTH - 100, 10);

	if (!g_visualizer) {
		DrawText("No visualizer loaded", 10, 40, 20, WHITE);
		return;
	}

	char loadedVisualizerText[256];
	snprintf(
		loadedVisualizerText,
		sizeof(loadedVisualizerText),
		"Loaded visualizer: %s",
		g_visualizer->GetName()
	);

	char currentStatusText[256];
	snprintf(
		currentStatusText,
		sizeof(currentStatusText),
		"Current status: %s",
		g_visualizer->GetCurrentStatus()
	);

	DrawText(loadedVisualizerText, 10, 40, 20, WHITE);
	DrawText(currentStatusText, 10, 70, 20, WHITE);

	g_visualizer->OnNewFrame();
}

void HandleInput() {
	if (IsKeyPressed(KEY_SPACE)) {
		g_visualizer->Start();
	}
}

int main() {
	GCR_LOG_INFO("Hello world!");
	GCR_LOG_INFO("Creating window...");

	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);

	ChangeVisualizer(new CMarchingCubesVisualizer());
	g_visualizer->Start();

	while (!WindowShouldClose()) {
		BeginDrawing();
		Render();
		HandleInput();
		EndDrawing();
	}

	CloseWindow();

	return 0;
}