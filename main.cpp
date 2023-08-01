// Dear ImGui: standalone example application for DirectX 12
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// Important: to compile on 32-bit systems, the DirectX12 backend requires code to be compiled with '#define ImTextureID ImU64'.
// This is because we need ImTextureID to carry a 64-bit value and by default ImTextureID is defined as void*.
// This define is set in the example .vcxproj file and need to be replicated in your app or by adding it to your imconfig.h file.

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#include "imfilebrowser.h"
#include <d3d9.h>
#include <tchar.h>
#include <stdio.h>
#include <Windows.h>
#include "ethercatConfigurator_2.0.h"

// Data
static LPDIRECT3D9              g_pD3D = nullptr;
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

CEthercatConfigurator ecatConfigurator;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int numKeypad(int input, bool* p_open);

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Ethercat Configurator", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Create a file browser instance
    ImGui::FileBrowser fileDialogLoad;
    //ImGui::FileBrowser fileDialogWrite(ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir);
    ImGui::FileBrowser fileDialogWrite(ImGuiFileBrowserFlags_SelectDirectory);

    // Set browser properties
    fileDialogLoad.SetTitle("File Explorer");
    fileDialogLoad.SetTypeFilters({ ".json" });
    fileDialogWrite.SetTitle("File Explorer");

    // Main loop
    bool done = false;
    bool error = false;
    bool showAllAdapters = false;
    bool write = false;
    bool written = false;
    bool load = false;
    bool loaded = false;
    bool keypad = false;
    int selected = -1;
    bool first = false;
    bool last = false;
    std::string key;

    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //My Code
        ImGuiStyle& style = ImGui::GetStyle();
        style.FramePadding.y = 10;

        // Or statment is needed to keep code in this path until code has finished all necessary processes
        if (!ecatConfigurator.adapterDevices.size())
        {
            ImGui::SetNextWindowPos(ImVec2(285, 280), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(700, 135), ImGuiCond_FirstUseEver);

            ImGui::Begin("Error", &done);
            ImGui::Text("File ethercatBus.json was not found.\n"
                        "This means either there are no adapters with devices on the Bus or ReadBus.exe failed to run.\n"
                        "You can now either load a ethercatBus.json saved elsewhere on the system or close the program\n");

            if (ImGui::Button("Load File", ImVec2(100, 40)))
            {
                fileDialogLoad.Open();
                load = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("Close Program", ImVec2(100, 40)))
                done = true;

            if (load)
            {
                fileDialogLoad.Display();
                if (fileDialogLoad.HasSelected())
                {
                    std::string file = fileDialogLoad.GetSelected().string();
                    ecatConfigurator.init(file);
                    load = false;
                    fileDialogLoad.Close();
                }
            }

            ImGui::End();
        }
        else
        {
            // We specify a default position/size in case there's no data in the .ini file.
            // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
            ImGui::SetNextWindowPos(ImVec2(2, 3), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(808, 650), ImGuiCond_FirstUseEver);

            ImGui::Begin("Configurator");

            // Using the generic BeginCombo() API, you have full control over how to display the combo contents.
            // (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
            // stored in the object itself, etc.)
            //const char* items[] = { "AAAA" }; //, "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
            if (showAllAdapters)
            {
                static int item_current_idx = 0; // Here we store our selection data as an index.
                const char* combo_preview_value = ecatConfigurator.allAdapters[item_current_idx].data();  // Pass in the preview value visible before opening the combo (it could be anything)
                if (ImGui::BeginCombo("Avaliable Adapters", combo_preview_value))
                {

                    for (int n = 0; n < ecatConfigurator.allAdapters.size(); n++)
                    {
                        const bool is_selected = (item_current_idx == n);
                        if (ImGui::Selectable(ecatConfigurator.allAdapters[n].data(), is_selected))
                        {
                            item_current_idx = n;
                            key = ecatConfigurator.allAdapters[n];
                        }

                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }
            else
            {
                std::vector<std::string> adapters;
                for (auto i = ecatConfigurator.adapterDevices.begin(); i != ecatConfigurator.adapterDevices.end(); i++)
                {
                    adapters.push_back(i->first);
                }

                static int item_current_idx = 0; // Here we store our selection data as an index.
                const char* combo_preview_value = adapters[item_current_idx].data(); //items[item_current_idx];  // Pass in the preview value visible before opening the combo (it could be anything)
                key = adapters[item_current_idx];
                if (ImGui::BeginCombo("Avaliable Adapters", combo_preview_value))
                {

                    for (int n = 0; n < adapters.size(); n++)
                    {
                        const bool is_selected = (item_current_idx == n);
                        if (ImGui::Selectable(adapters[n].data(), is_selected))
                        {
                            item_current_idx = n;
                            key = adapters[n];
                        }

                        // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }            

            ImGui::Checkbox("List All Adapters", &showAllAdapters);

            if (key.size())
                ImGui::Checkbox("Assign Chutes Forward", &ecatConfigurator.adapterChutesDirection);

            if (ImGui::Button("Transfer All Settings to be Written", ImVec2(270, 40)))           // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                ecatConfigurator.transferSettings(key);
                loaded = true;
            }
            if (loaded)
            {
                ImGui::SameLine();
                if (ImGui::Button("Transfer Adapter Name to be Written", ImVec2(270, 40)))
                    ecatConfigurator.currentAdapterName = key;
            }

            if (ImGui::Button("Load Settings", ImVec2(110, 40)))                                 // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                fileDialogLoad.Open();            
                load = true;
            }
            if (loaded)                       // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                ImGui::SameLine();
                if (ImGui::Button("Save Settings", ImVec2(110, 40)))
                {
                    fileDialogWrite.Open();
                    write = true;
                }
                
            }

            if (ImGui::Button("Read the Bus Again", ImVec2(150, 40)))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                //for (int n = 0; n < IM_ARRAYSIZE(items); n++)
                //    items[n] = NULL;
                ecatConfigurator.init();
                ecatConfigurator.adapterDevices.clear();
            }
                        

            // Written this way so that the file browser window remains open until the user is done with it
            if (write)
            {
                fileDialogWrite.Display();
                if (fileDialogWrite.HasSelected())
                {
                    // Add ability to select destinantion and name of file to write
                    std::string pwd = fileDialogWrite.GetPwd().string();
                    //std::string file = fileDialogLoad.GetSelected().string();
                    ecatConfigurator.writeSettings(pwd);
                    written = true;
                    fileDialogWrite.Close();
                    write = false;
                }            
            }
            else if (load)
            {
                fileDialogLoad.Display();
                if (fileDialogLoad.HasSelected())
                {
                    std::string file = fileDialogLoad.GetSelected().string();
                    if (ecatConfigurator.loadSettings(file))
                        loaded = true;
                    else
                        error = true;
                    fileDialogLoad.Close();
                    load = false;
                }                
            }

            // Window to let user know the file has been written
            if (written)
            {
                ImGui::SetNextWindowPos(ImVec2(96, 155), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(191, 106), ImGuiCond_FirstUseEver);
                ImGui::Begin("Update", &written);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                ImGui::Text("Your file has been saved.");
                if (ImGui::Button("OK", ImVec2(60, 40)))
                    written = false;
                ImGui::End();
            }

            // Window to let user know the file has been loaded did not have the correct information
            if (error)
            {
                ImGui::SetNextWindowPos(ImVec2(10, 169), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(433, 106), ImGuiCond_FirstUseEver);
                ImGui::Begin("Error", &error);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                ImGui::Text("Your selected file does not contain the correct information.\n Please Try again with a different file.");
                if (ImGui::Button("OK", ImVec2(60, 40)))
                    error = false;
                ImGui::End();
            }

            ImGui::Text("");
            ImGui::Text("Selected Adapter:");
                    
            static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;

            if (ImGui::BeginTable("Selected Adapter", 8, flags))
            {
                // Display headers so we can inspect their interaction with borders.
                // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for details)
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("EEP Man");
                ImGui::TableSetupColumn("EEP ID");
                ImGui::TableSetupColumn("Address");
                ImGui::TableSetupColumn("Index");
                ImGui::TableSetupColumn("Alias");
                ImGui::TableSetupColumn("Obits");
                ImGui::TableSetupColumn("Ibits");
                ImGui::TableHeadersRow();            

                for (int row = 0; row < ecatConfigurator.adapterDevices[key].size(); row++)
                {
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 25.0f);
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%s", ecatConfigurator.adapterDevices[key][row].name.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.adapterDevices[key][row].eepMan);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.adapterDevices[key][row].eepId);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.adapterDevices[key][row].addressConfig);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.adapterDevices[key][row].index);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.adapterDevices[key][row].addressAlias);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.adapterDevices[key][row].Obits);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.adapterDevices[key][row].Ibits);
                    
                }
                ImGui::EndTable();

                if (!ecatConfigurator.adapterDevices[key].size())
                    ecatConfigurator.adapterDevices.erase(key);
            }

            if (loaded)
            {
                ImGui::Text("");
                ImGui::Text("Data to be Written:");
                ImGui::Text("Adapter Name: %s", ecatConfigurator.currentAdapterName.c_str());
                //ImGui::SameLine;
                //ImGui::Text("Assign Chutes Forward Value(1 = true, 0 = false): %d", ecatConfigurator.currentAdapterChutesDirection);
            }            

            if (loaded && ImGui::BeginTable("Data to be Written", 11, flags))
            {
                // Display headers so we can inspect their interaction with borders.
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("EEP Man");
                ImGui::TableSetupColumn("EEP ID");
                ImGui::TableSetupColumn("Address");
                ImGui::TableSetupColumn("Index");
                ImGui::TableSetupColumn("Alias");
                ImGui::TableSetupColumn("Obits");
                ImGui::TableSetupColumn("Ibits");
                ImGui::TableSetupColumn("Device Type", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                ImGui::TableSetupColumn("First Chute");
                ImGui::TableSetupColumn("Last Chute");
                ImGui::TableHeadersRow();

                for (int row = 0; row < ecatConfigurator.currentDevices.size(); row++)
                {
                    ImGui::PushID(row);
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 25.0f);
                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("%s", ecatConfigurator.currentDevices[row].name.c_str());               
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.currentDevices[row].eepMan);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.currentDevices[row].eepId);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.currentDevices[row].addressConfig);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.currentDevices[row].index);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.currentDevices[row].addressAlias);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.currentDevices[row].Obits);
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", ecatConfigurator.currentDevices[row].Ibits);
                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(-FLT_MIN);
                    const char* types[] = { "NotDefined", "Chutes", "FlapsLamps", "Scales", "Inverter" };
                    static int item_current_idx2 = 0; // Here we store our selection data as an index.
                    const char* combo_preview_value2 = types[static_cast<int>(ecatConfigurator.currentDevices[row].deviceType)];  // Pass in the preview value visible before opening the combo (it could be anything)
                    //ImGuiStyle::FramePadding = ImVec2(270, 40);
                    if (ImGui::BeginCombo("", combo_preview_value2))
                    {

                        for (int n = 0; n < IM_ARRAYSIZE(types); n++)
                        {
                            const bool is_selected = (item_current_idx2 == n);
                            if (ImGui::Selectable(types[n], is_selected))
                            {
                                item_current_idx2 = n;
                                ecatConfigurator.currentDevices[row].deviceType = static_cast<CEthercatConfigurator::EthercatDeviceType>(n);
                            }

                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }                
                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(-FLT_MIN);
                    ImGui::PushID(8);
                    (ImGui::InputInt("", &ecatConfigurator.currentDevices[row].firstChute, 0));
                    if (ImGui::IsItemActive())
                    {
                        keypad = true;
                        selected = row;
                        first = true;
                        last = false;
                    }
                    ImGui::PopID();               
                    ImGui::TableNextColumn();
                    ImGui::PushItemWidth(-FLT_MIN);
                    ImGui::PushID(9);
                    (ImGui::InputInt("", &ecatConfigurator.currentDevices[row].lastChute, 0));
                    if (ImGui::IsItemActive())
                    {
                        keypad = true;
                        selected = row;
                        first = false;
                        last = true;
                    }
                    ImGui::PopID();
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }

            if (keypad && first)
            {
                ecatConfigurator.currentDevices[selected].firstChute = numKeypad(ecatConfigurator.currentDevices[selected].firstChute, &keypad);
            }                
            else if (keypad && last)
            {
                ecatConfigurator.currentDevices[selected].lastChute = numKeypad(ecatConfigurator.currentDevices[selected].lastChute, &keypad);
            }                

            ImGui::End();
        }
        //

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        //Background Color
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    // Cleanup
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

int numKeypad(int input, bool* p_open)
{
    ImGui::SetNextWindowPos(ImVec2(817, 434), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(152, 221), ImGuiCond_FirstUseEver);

    ImGui::Begin("Keypad", p_open);

    if (ImGui::Button("<x", ImVec2(40, 40)))        // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        input /= 10;
    }
    ImGui::SameLine();
    if (ImGui::Button("0", ImVec2(88, 40)))         // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        input *= 10;
    }

    if (ImGui::Button("1", ImVec2(40, 40)))         // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        input = input * 10 + 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("2", ImVec2(40, 40)))         // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        input = input * 10 + 2;
    }
    ImGui::SameLine();
    if (ImGui::Button("3", ImVec2(40, 40)))         // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        input = input * 10 + 3;
    }

    if (ImGui::Button("4", ImVec2(40, 40)))         // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        input = input * 10 + 4;
    }
    ImGui::SameLine();
    if (ImGui::Button("5", ImVec2(40, 40)))         // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        input = input * 10 + 5;
    }
    ImGui::SameLine();
    if (ImGui::Button("6", ImVec2(40, 40)))         // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        input = input * 10 + 6;
    }
    
    if (ImGui::Button("7", ImVec2(40, 40)))         // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        input = input * 10 + 7;
    }
    ImGui::SameLine();
    if (ImGui::Button("8", ImVec2(40, 40)))         // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        input = input * 10 + 8;
    }
    ImGui::SameLine();
    if (ImGui::Button("9", ImVec2(40, 40)))         // Buttons return true when clicked (most widgets return true when edited/activated)
    {
        input = input * 10 + 9;
    }

    // Only used for debugging purposes; Can be removed
    //ImGui::Text("%d", input);

    ImGui::End();

    return input;
}

/*
* Coloring for UI
* 
* Color buttons, demonstrate using PushID() to add unique identifier in the ID stack, and changing style.
        IMGUI_DEMO_MARKER("Widgets/Basic/Buttons (Colored)");
        for (int i = 0; i < 7; i++)
        {
            if (i > 0)
                ImGui::SameLine();
            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
            ImGui::Button("Click");
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }
*
* Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0f or 1.0f)
    if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
        style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
    { bool border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &border)) { style.WindowBorderSize = border ? 1.0f : 0.0f; } }
    ImGui::SameLine();
    { bool border = (style.FrameBorderSize > 0.0f);  if (ImGui::Checkbox("FrameBorder",  &border)) { style.FrameBorderSize  = border ? 1.0f : 0.0f; } }
    ImGui::SameLine();
    { bool border = (style.PopupBorderSize > 0.0f);  if (ImGui::Checkbox("PopupBorder",  &border)) { style.PopupBorderSize  = border ? 1.0f : 0.0f; } }
*
*  ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
* 
* You can pass in a reference ImGuiStyle structure to compare to, revert to and save to
    // (without a reference style pointer, we will use one compared locally as a reference)
    ImGuiStyle& style = ImGui::GetStyle();
    static ImGuiStyle ref_saved_style;

    // Default to using internal storage as reference
    static bool init = true;
    if (init && ref == NULL)
        ref_saved_style = style;
    init = false;
    if (ref == NULL)
        ref = &ref_saved_style;
* 
* ImGui::BeginChild("##colors", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NavFlattened);
            ImGui::PushItemWidth(-160);
            for (int i = 0; i < ImGuiCol_COUNT; i++)
            {
                const char* name = ImGui::GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                ImGui::PushID(i);
                ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
                if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
                {
                    // Tips: in a real user application, you may want to merge and use an icon font into the main font,
                    // so instead of "Save"/"Revert" you'd use icons!
                    // Read the FAQ and docs/FONTS.md about using icon fonts. It's really easy and super convenient!
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) { ref->Colors[i] = style.Colors[i]; }
                    ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) { style.Colors[i] = ref->Colors[i]; }
                }
                ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
                ImGui::TextUnformatted(name);
                ImGui::PopID();
            }
            ImGui::PopItemWidth();
            ImGui::EndChild();
*
* 
*/