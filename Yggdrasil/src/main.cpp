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
	ImVec2 resolution(1600, 900);
	SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	//SDL_Window* window = SDL_CreateWindow("Yggdrasil", (int)(1280 * main_scale), (int)(720 * main_scale), window_flags);
	SDL_Window* window = SDL_CreateWindow("Yggdrasil", resolution.x, resolution.y, window_flags);
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
	
	//Misc. variables
	float padding = 10.f;
	float error_text_y = 66;

	//Set the "New Trunk" button position
	ImVec2 button_size(100, 30);
	ImVec2 pos = resolution;
	pos.x -= button_size.x + padding;
	pos.y -= button_size.y + padding;

	//Trunk variables
	int trunks_open = 0;
	char trunk_name[128] = "";
	vector<string> trunk_names;

	//Branch variables
	char branch_name[128] = "";


	while (app_running) {
		//Get size of window
		resolution = ImGui::GetIO().DisplaySize;

		//Poll events
		{
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
		}

		//Start the Dear ImGui frame
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
		
		//New Trunk
		{
			//Set button position
			pos.x = resolution.x - (button_size.x + padding);
			pos.y = resolution.y - (button_size.y + padding);
			ImGui::SetNextWindowPos(pos);
			ImGui::SetNextWindowSize(button_size);
			//Make invisible window that just contains the new_trunk button
			ImGui::Begin("Invisible", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar
											 | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings  | ImGuiWindowFlags_NoBackground);
			ImGui::SetCursorScreenPos(pos);
			if (ImGui::Button("New Trunk", button_size)) {
				for (int c = 0; c < 128; ++c)
					trunk_name[c] = '\0';
				ImGui::OpenPopup("New Trunk");
			}

			if (ImGui::BeginPopupModal("New Trunk", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Enter Character name:");

				//Text input field
				ImGui::InputText("Character Name", trunk_name, IM_ARRAYSIZE(trunk_name));

				//Trunk name validation
				if (ImGui::Button("Done") or ImGui::IsKeyPressed(ImGuiKey_Enter) or ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
					if (trunk_name[0] == '\0')
						ImGui::OpenPopup("Trunk Error!");
					else {
						ImGui::CloseCurrentPopup();
						++trunks_open;
						string new_name = trunk_name;
						trunk_names.push_back(new_name);
					}
				}

				//Cancel
				ImGui::SetCursorPos(ImVec2(100, ImGui::GetWindowSize().y - 26));
				if (ImGui::Button("Cancel"))
					ImGui::CloseCurrentPopup();

				ImGui::SetNextWindowPos(ImVec2(resolution.x * .5 - 150, resolution.y * .5 - 75), ImGuiCond_Always);
				ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);
				if (ImGui::BeginPopup("Trunk Error!")) {
					//SetCursorPos is relative to current window
					ImGui::SetCursorPos(ImVec2(60, error_text_y));
					ImGui::Text("Character must have a name!");
					ImGui::EndPopup();
				}

				ImGui::EndPopup();
			}
			ImGui::End();
		}
		//eo New Trunk

		//Trunk editing
		{
			for (int t = 0; t < trunks_open; ++t) {
				ImGui::SetNextWindowPos(ImVec2(150, 100), ImGuiCond_Appearing);
				ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_Appearing);
				ImGui::Begin(trunk_names[t].c_str());

				//New Branch
				{
					if (ImGui::Button("New Branch")) {
						ImGui::OpenPopup("New Branch");
						for (int c = 0; c < 128; ++c)
							branch_name[c] = '\0';
					}

					if (ImGui::BeginPopupModal("New Branch", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
						ImGui::Text("Enter Branch name:");

						//Text input field
						ImGui::InputText("Branch Name", branch_name, IM_ARRAYSIZE(branch_name));

						//Branch name validation
						if (ImGui::Button("Done") or ImGui::IsKeyPressed(ImGuiKey_Enter) or ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
							if (branch_name[0] == '\0')
								ImGui::OpenPopup("Branch Error!");
							else {
								ImGui::CloseCurrentPopup();
								//++trunks_open;
								//string new_name = trunk_name;
								//trunk_names.push_back(new_name);
							}
						}

						ImGui::SetNextWindowPos(ImVec2(resolution.x * .5 - 150, resolution.y * .5 - 75), ImGuiCond_Always);
						ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);
						if (ImGui::BeginPopup("Branch Error!")) {
							//SetCursorPos is relative to current window
							ImGui::SetCursorPos(ImVec2(66, error_text_y));
							ImGui::Text("Branch must have a name!");
							ImGui::EndPopup();
						}

						ImGui::EndPopup();
					}
				}
				
				
				
				
				//Exporting
				{
					ImVec2 trunk_size = ImGui::GetWindowSize();
					ImGui::SetCursorPos(ImVec2(trunk_size.x - (button_size.x*.6f), trunk_size.y - button_size.y));
					if (ImGui::Button("Export")) {
						json j;
						ofstream out(trunk_names[t] + "_dialogue.json");
						out << setw(4) << j << endl;
						out.close();

						//Success popup open
						ImGui::OpenPopup("Export Successful!");
					}

					//Successful Export Popup
					ImGui::SetNextWindowPos(ImVec2(resolution.x * .5 - 150, resolution.y * .5 - 75), ImGuiCond_Always);
					ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_Always);
					if (ImGui::BeginPopup("Export Successful!")) {
						//SetCursorPos is relative to current window
						ImGui::SetCursorPos(ImVec2(86, 64));
						ImGui::Text("Export Successful!");
						ImGui::EndPopup();
					}
				}

				ImGui::End();
			}
		}
		//eo Trunk editing

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