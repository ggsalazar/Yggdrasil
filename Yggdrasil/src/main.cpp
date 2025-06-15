#include <iostream>
#include <fstream>
#include <stdio.h>
#include <unordered_map>
#include <SDL3/SDL.h>
#include "../packages/imgui/imgui.h"
#include "../packages/imgui/imgui_internal.h"
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
	float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay()); //Look up what this is doing later
	ImVec2 resolution(1600, 900);
	SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	SDL_Window* window = SDL_CreateWindow("Yggdrasil", (int)(resolution.x * scale), (int)(resolution.y * scale), window_flags);
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


	//ImGui setup
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
	int branches_open = 0;
	char branch_name[128] = "";
	vector<string> branch_names;
	vector<ImVec2> branch_posits;
	//Key is branch name, value is trunk name; multiple branches can have only one owner
	unordered_map<string, string> branch_owners;

	//Stem variables
	static const char* stem_options[] = {"Texts", "Choices"};
	static int current_stem = 0;

	//Leaf variables


	while (app_running) {
		//Get size of application window
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
				ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 58, ImGui::GetWindowSize().y - 27));
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

		//Trunk stuff
		{
			for (int t = 0; t < trunks_open; ++t) {
				//Set trunk position & size
				ImGui::SetNextWindowPos(ImVec2(150, 100), ImGuiCond_Appearing);
				ImGui::SetNextWindowSize(ImVec2(900, 700), ImGuiCond_Appearing);
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
								++branches_open;
								string new_name = branch_name;
								branch_names.push_back(new_name);
								branch_posits.push_back(ImVec2(ImGui::GetWindowPos().x + 30, ImGui::GetWindowPos().y+30));

								//This trunk owns this branch
								branch_owners[new_name] = trunk_names[t];
							}
						}

						//Cancel
						ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 58, ImGui::GetWindowSize().y - 27));
						if (ImGui::Button("Cancel"))
							ImGui::CloseCurrentPopup();


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
		//eo Trunk stuff




		//Branch editing
		{
			for (int b = 0; b < branches_open; ++b) {
				//Set branch position & size
				ImGui::SetNextWindowPos(branch_posits[b], ImGuiCond_Appearing);
				ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);
				ImGui::Begin(branch_names[b].c_str());
				ImGui::SetWindowFocus();
				branch_posits[b] = ImGui::GetWindowPos();

				//Branches cannot leave the confines of their trunks
				//Get a pointer to the owning trunk
				ImGuiWindow* owner = ImGui::FindWindowByName(branch_owners[branch_names[b]].c_str());
				if (owner and owner->WasActive) {
					//x
					if (branch_posits[b].x < owner->Pos.x) branch_posits[b].x = owner->Pos.x;
					else if (branch_posits[b].x + ImGui::GetWindowSize().x > owner->Pos.x + owner->Size.x) branch_posits[b].x = owner->Pos.x + owner->Size.x - ImGui::GetWindowSize().x;
					//y
					if (branch_posits[b].y < owner->Pos.y) branch_posits[b].y = owner->Pos.y;
					else if (branch_posits[b].y + ImGui::GetWindowSize().y > owner->Pos.y + owner->Size.y) branch_posits[b].y = owner->Pos.y + owner->Size.y - ImGui::GetWindowSize().y;
				}
				ImGui::SetWindowPos(branch_posits[b], ImGuiCond_Always);



				//Create a new stem
				if (ImGui::BeginCombo("New Stem", stem_options[0])) {
					for (int s = 0; s < IM_ARRAYSIZE(stem_options); ++s) {
						bool is_selected = current_stem == s;
						if (ImGui::Selectable(stem_options[s], is_selected)) current_stem = s;

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}
				

				ImGui::End();
			}
		}





		// Rendering
		ImGui::Render();
		SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
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