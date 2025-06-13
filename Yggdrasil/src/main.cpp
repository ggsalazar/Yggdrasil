#include <iostream>
#include <fstream>
#include <stdio.h>
#include <SDL3/SDL.h>
#include "../packages/imgui/imgui.h"
#include "../packages/imgui/imgui_impl_sdl3.h"
#include "../packages/imgui/imgui_impl_sdlrenderer3.h"
#include "../packages/json-3.12.0/single_include/nlohmann/json.hpp"

using namespace std;
using json = nlohmann::json;

int main() {

	//Init SDL
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		printf("Error: SDL_Init(): %s\n", SDL_GetError());
		return -1;
	}

	// Create window with SDL_Renderer graphics context
	float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay()); //Look up what this is doing later
	SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	//SDL_Window* window = SDL_CreateWindow("Yggdrasil", (int)(1280 * main_scale), (int)(720 * main_scale), window_flags);
	SDL_Window* window = SDL_CreateWindow("Yggdrasil", 1600, 900, window_flags);
	if (!window) {
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return -1;
	}
	SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
	SDL_SetRenderVSync(renderer, 1);
	if (!renderer) {
		SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
		return -1;
	}
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(window);



	ImGuiContext* imgui_context = ImGui::CreateContext();
	ImGui::SetCurrentContext(imgui_context);

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer3_Init(renderer);

	bool app_running = true;
	ImVec4 clear_color = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
	int trunks_open = 0;
	vector<string> trunk_names;

	bool export_error = false;

	while (app_running) {
		//Poll events
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
				app_running = false;
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
				app_running = false;
		}

		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
			SDL_Delay(10);
			continue;
		}

		//Start the Dear ImGui frame
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
		
		if (ImGui::Button("New Trunk")) {
			++trunks_open;
			string new_name = "[Character Name]";
			if (trunks_open > 1)
				new_name += to_string(trunks_open);

			trunk_names.push_back(new_name);
			cout << trunks_open << "\n";
		}

		for (int t = 0; t < trunks_open; ++t) {
			ImGui::Begin(trunk_names[t].c_str());

			if (ImGui::Button("Export")) {

				if (trunk_names[t] == "[Character Name]")
					export_error = true;
				else {
					json test;
					ofstream out(trunk_names[t] + "_dialogue.json");
					out << setw(4) << test << endl;
					out.close();
				}
			}

			if (export_error) ImGui::BeginPopup("Error! Character must have a name!");

			ImGui::End();
		}
		//ImGui::Begin("[Character Name]");

		



		//ImGui::End();

		// Rendering
		ImGui::Render();
		SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y); //What is this doing?
		SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w); //What is this doing?
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
		SDL_RenderPresent(renderer);
	}
	
	//Cleanup
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext(imgui_context);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}