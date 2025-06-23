#include "../../include/sound_engine/sound_engine.h"

#include "../../third_party/imgui/window_utilities.h"

#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
#include <iostream>

// Third party

#include <fmt/core.h>
#include <fmt/color.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/alut.h>

int SoundEngine::MainWindow()
{
	// Setup SDL
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) != 0)
	{
		printf("Error: SDL_Init(): %s\n", SDL_GetError());
		return -1;
	}

	// Create window with Vulkan graphics context
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
	SDL_Window *window = SDL_CreateWindow("Sound Engine Main Window", 1280, 720, window_flags);
	if (window == nullptr)
	{
		printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
		return -1;
	}

	ImVector<const char *> extensions;
	{
		uint32_t sdl_extensions_count = 0;
		const char *const *sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);
		for (uint32_t n = 0; n < sdl_extensions_count; n++)
			extensions.push_back(sdl_extensions[n]);
	}
	SetupVulkan(extensions);

	// Create Window Surface
	VkSurfaceKHR surface;
	VkResult err;
	if (SDL_Vulkan_CreateSurface(window, g_Instance, g_Allocator, &surface) == 0)
	{
		printf("Failed to create Vulkan surface.\n");
		return 1;
	}

	// Create Framebuffers
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	ImGui_ImplVulkanH_Window *wd = &g_MainWindowData;
	SetupVulkanWindow(wd, surface, w, h);
	SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_ShowWindow(window);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForVulkan(window);
	ImGui_ImplVulkan_InitInfo init_info = {};
	// init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
	init_info.Instance = g_Instance;
	init_info.PhysicalDevice = g_PhysicalDevice;
	init_info.Device = g_Device;
	init_info.QueueFamily = g_QueueFamily;
	init_info.Queue = g_Queue;
	init_info.PipelineCache = g_PipelineCache;
	init_info.DescriptorPool = g_DescriptorPool;
	init_info.RenderPass = wd->RenderPass;
	init_info.Subpass = 0;
	init_info.MinImageCount = g_MinImageCount;
	init_info.ImageCount = wd->ImageCount;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = g_Allocator;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info);

	// Our state

	bool show_another_window = false;
	bool play_sound = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	bool done = false;
	while (!done)
	{

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
				done = true;
			if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
				done = true;
		}
		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
		{
			SDL_Delay(10);
			continue;
		}

		// Resize swap chain?
		int fb_width, fb_height;
		SDL_GetWindowSize(window, &fb_width, &fb_height);
		if (fb_width > 0 && fb_height > 0 && (g_SwapChainRebuild || g_MainWindowData.Width != fb_width || g_MainWindowData.Height != fb_height))
		{
			ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
			ImGui_ImplVulkanH_CreateOrResizeWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData, g_QueueFamily, g_Allocator, fb_width, fb_height, g_MinImageCount);
			g_MainWindowData.FrameIndex = 0;
			g_SwapChainRebuild = false;
		}

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, Sound Engine!"); // Create a window called "Hello, world!" and append into it.

			ImGui::Text("Things to do with sound engine"); // Display some text (you can use a format strings too)
			ImGui::Checkbox("Tweaking window", &show_another_window);
			ImGui::Checkbox("Play a sound", &play_sound);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);			 // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("Volume UP")) // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("Level = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Tweaking&Plugins", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Here we could tweaking and use some plugins");
			if (ImGui::Button("Close Me, Please"))
				show_another_window = false;
			ImGui::End();
		}

		if (play_sound)
		{
			ImGui::Begin("Playing a sound", &play_sound); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Here is the place of producing the sound.");
			if (ImGui::Button("Play"))
			{
				this->sound();
				this->sound_state = true;
				play_sound = false;
			}

			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		ImDrawData *draw_data = ImGui::GetDrawData();
		const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
		if (!is_minimized)
		{
			wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
			wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
			wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
			wd->ClearValue.color.float32[3] = clear_color.w;
			FrameRender(wd, draw_data);
			FramePresent(wd);
		}
	}

	// Cleanup
	err = vkDeviceWaitIdle(g_Device);
	check_vk_result(err);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	CleanupVulkanWindow();
	CleanupVulkan();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

SoundEngine::SoundEngine()
{
	fmt::print(fg(fmt::color::sea_green), "\n{}\n", "Sound Engine entry point.");
}

// Basic WAV loader for uncompressed mono WAVs
bool loadWavFile(const std::string &filename, std::vector<char> &data,
				 ALenum &format, ALsizei &freq)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file)
	{
		std::cerr << "Failed to open WAV file.\n";
		return false;
	}

	char riff[4];
	file.read(riff, 4); // "RIFF"
	file.ignore(4);		// Chunk size
	char wave[4];
	file.read(wave, 4); // "WAVE"

	// Read chunks
	char chunkId[4];
	int chunkSize;
	short audioFormat, numChannels, bitsPerSample;
	int sampleRate;

	while (file.read(chunkId, 4))
	{
		file.read(reinterpret_cast<char *>(&chunkSize), 4);

		if (std::strncmp(chunkId, "fmt ", 4) == 0)
		{
			file.read(reinterpret_cast<char *>(&audioFormat), 2);
			file.read(reinterpret_cast<char *>(&numChannels), 2);
			file.read(reinterpret_cast<char *>(&sampleRate), 4);
			file.ignore(6); // Byte rate + block align
			file.read(reinterpret_cast<char *>(&bitsPerSample), 2);
			file.ignore(chunkSize - 16); // Skip the rest
		}
		else if (std::strncmp(chunkId, "data", 4) == 0)
		{
			data.resize(chunkSize);
			file.read(data.data(), chunkSize);
			break;
		}
		else
		{
			file.ignore(chunkSize);
		}
	}

	freq = sampleRate;
	if (numChannels == 1)
	{
		format = (bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	}
	else if (numChannels == 2)
	{
		format = (bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
	}
	else
	{
		std::cerr << "Unsupported channel count: " << numChannels << "\n";
		return false;
	}

	return true;
}

int SoundEngine::sound()
{
	ALCdevice *device = alcOpenDevice(nullptr);
	if (!device)
	{
		std::cerr << "Failed to open device.\n";
		return -1;
	}

	ALCcontext *context = alcCreateContext(device, nullptr);
	if (!context || !alcMakeContextCurrent(context))
	{
		std::cerr << "Failed to set OpenAL context.\n";
		alcCloseDevice(device);
		return -1;
	}

	// Load sound
	std::vector<char> bufferData;
	ALenum format;
	ALsizei freq;
	if (!loadWavFile("sounds/test.wav", bufferData, format, freq))
	{
		std::cerr << "Failed to load WAV.\n";
		alcDestroyContext(context);
		alcCloseDevice(device);
		return -1;
	}

	// Generate buffer
	ALuint buffer;
	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, bufferData.data(), bufferData.size(), freq);

	// Generate source
	ALuint source;
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);

	if (this->sound_state)
	{
		alSourcePlay(source);
	}
	else
	{
		alSourceStop(source);
		return 0;
	}

	std::cout << "Playing sound...\n";

	// Wait until done
	ALint state;
	do
	{
		alGetSourcei(source, AL_SOURCE_STATE, &state);
	} while (state == AL_PLAYING);
	

	std::cout << "Done.\n";

	this->sound_state = false;

	// Cleanup
	alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);
	alcMakeContextCurrent(nullptr);
	alcDestroyContext(context);
	alcCloseDevice(device);

	return 0;
}

bool SoundEngine::init()
{
	// Init ( to do )
	this->isInitialized = true;

	if (this->isInitialized)
		return true;

	return false;
}

void SoundEngine::run()
{
	if (this->init())
	{
		this->MainWindow();
	}
}