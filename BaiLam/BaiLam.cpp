#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <commdlg.h>

// Định nghĩa ID cho các thành phần giao diện 
#define IDC_INPUT_TEXT 101
#define IDC_ALGO_LIST 102
#define IDC_SORT_BUTTON 103
#define IDC_OUTPUT_TEXT 104
#define IDC_LOAD_FILE_BUTTON 105
#define IDC_SAVE_FILE_BUTTON 106
#define IDC_TIME_TEXT 107
#define IDC_RESET_BUTTON 108

// Biến toàn cục 
HWND hInput, hAlgoList, hSortButton, hOutput, hLoadFile, hSaveFile, hTimeText, hResetButton;
std::vector<int> data; // Dữ liệu sẽ được sắp xếp 
HWND hWndMain; // Cửa sổ chính 
HDC hdcMem; // HDC bộ nhớ để vẽ đôi 
HBITMAP hBitmap; // Bitmap để lưu hình ảnh vẽ tạm thời

// Nguyên mẫu hàm 
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
std::vector<int> ParseInput(const std::wstring& input); // Phân tích dữ liệu nhập từ người dùng 
bool LoadDataFromFile(const std::wstring& filename, std::vector<int>& data); // Tải dữ liệu 
bool SaveDataToFile(const std::wstring& filename, const std::vector<int>& data); // Lưu dữ liệu 
void DrawVisualization(HDC hdc, const std::vector<int>& arr);  //vẽ trực quan hóa sắp xếp 
void RedrawUI(HDC hdc); // Vẽ lại giao diện người dùng 

// Các thuật toán sắp xếp 
void BubbleSort(std::vector<int>& arr, HWND hWnd = NULL);
void QuickSort(std::vector<int>& arr, int low, int high, HWND hWnd = NULL);
void MergeSort(std::vector<int>& arr, int left, int right, HWND hWnd = NULL);
void SelectionSort(std::vector<int>& arr, HWND hWnd = NULL);
void InsertionSort(std::vector<int>& arr, HWND hWnd = NULL);
void HeapSort(std::vector<int>& arr, HWND hWnd = NULL);
void RadixSort(std::vector<int>& arr, HWND hWnd = NULL);
void ShellSort(std::vector<int>& arr, HWND hWnd = NULL);

// Hàm chính 
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"SortingVisualizerClass";


    //Đăng ký lớp cửa sổ
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc; // Gắn hàm xử lý sự kiện 
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassEx(&wc)) return 0;

    // Tạo cửa sổ chính 
    hWndMain = CreateWindowEx(
        0, CLASS_NAME, L"Sorting Visualizer",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL);

    if (!hWndMain) return 0;

    // Hiển thị cửa sổ 
    ShowWindow(hWndMain, nCmdShow);
    UpdateWindow(hWndMain);

    // Vòng lặp xử lý sự kiện chính 
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

// Hàm xử lý sự kiện cửa sổ 
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        // Tạo các thành phần giao diện người dùng 
        hInput = CreateWindow(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE,
            20, 20, 300, 50, hWnd, (HMENU)IDC_INPUT_TEXT, NULL, NULL);

        hAlgoList = CreateWindow(L"LISTBOX", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_MULTIPLESEL | WS_VSCROLL,
            20, 90, 200, 150, hWnd, (HMENU)IDC_ALGO_LIST, NULL, NULL);

        // Thêm các thuật toán sắp xếp vào danh sách 
        SendMessage(hAlgoList, LB_ADDSTRING, 0, (LPARAM)L"Bubble Sort");
        SendMessage(hAlgoList, LB_ADDSTRING, 0, (LPARAM)L"Quick Sort");
        SendMessage(hAlgoList, LB_ADDSTRING, 0, (LPARAM)L"Merge Sort");
        SendMessage(hAlgoList, LB_ADDSTRING, 0, (LPARAM)L"Selection Sort");
        SendMessage(hAlgoList, LB_ADDSTRING, 0, (LPARAM)L"Insertion Sort");
        SendMessage(hAlgoList, LB_ADDSTRING, 0, (LPARAM)L"Heap Sort");
        SendMessage(hAlgoList, LB_ADDSTRING, 0, (LPARAM)L"Radix Sort");
        SendMessage(hAlgoList, LB_ADDSTRING, 0, (LPARAM)L"Shell Sort");

        // Tạo nút bấm "Sort"
        hSortButton = CreateWindow(L"BUTTON", L"Sort",
            WS_CHILD | WS_VISIBLE,
            240, 90, 80, 30, hWnd, (HMENU)IDC_SORT_BUTTON, NULL, NULL);

        // Tạo nút bấm "Reset"
        hResetButton = CreateWindow(L"BUTTON", L"Reset",
            WS_CHILD | WS_VISIBLE,
            240, 130, 80, 30, hWnd, (HMENU)IDC_RESET_BUTTON, NULL, NULL);

        // Tạo nút bấm "Load File" 
        hLoadFile = CreateWindow(L"BUTTON", L"Load File",
            WS_CHILD | WS_VISIBLE,
            340, 20, 100, 30, hWnd, (HMENU)IDC_LOAD_FILE_BUTTON, NULL, NULL);

        // Tạo nút bấm "Save File"
        hSaveFile = CreateWindow(L"BUTTON", L"Save File",
            WS_CHILD | WS_VISIBLE,
            340, 60, 100, 30, hWnd, (HMENU)IDC_SAVE_FILE_BUTTON, NULL, NULL);

        // Tạo hộp văn bản hiển thị kết quả 
        hOutput = CreateWindow(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            20, 250, 740, 100, hWnd, (HMENU)IDC_OUTPUT_TEXT, NULL, NULL);

        // Tạo hộp văn bản hiển thị thời gian thực thi 
        hTimeText = CreateWindow(L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            20, 360, 740, 100, hWnd, (HMENU)IDC_TIME_TEXT, NULL, NULL);

        // Tạo nhãn cho trực quan hóa 
        CreateWindow(L"STATIC", L"Sorting Visualization",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 470, 740, 20, hWnd, NULL, NULL, NULL);

        // Tạo bitmap ngoài màn hình để vẽ đôi
        HDC hdc = GetDC(hWnd);
        hdcMem = CreateCompatibleDC(hdc);
        RECT rect;
        GetClientRect(hWnd, &rect);
        hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
        SelectObject(hdcMem, hBitmap);
        ReleaseDC(hWnd, hdc);

        break;
    }
    case WM_COMMAND: {
        // Xử lý sự kiện cho các nút bấm 
        if (LOWORD(wParam) == IDC_SORT_BUTTON) {
            wchar_t inputText[1024];
            GetWindowText(hInput, inputText, 1024);
            data = ParseInput(inputText);

            // Kiểm tra dữ liệu đầu vào 
            if (data.empty()) {
                MessageBox(hWnd, L"Please enter valid data (numbers separated by spaces)!", L"Error", MB_OK);
                return 0;
            }

            // Xử lý các thuật toán đã chọn
            int selectedCount = SendMessage(hAlgoList, LB_GETSELCOUNT, 0, 0);
            if (selectedCount == 0) {
                MessageBox(hWnd, L"Please select at least one sorting algorithm!", L"Error", MB_OK);
                return 0;
            }

            std::vector<int> selectedIndices(selectedCount);
            SendMessage(hAlgoList, LB_GETSELITEMS, selectedCount, (LPARAM)selectedIndices.data());

            std::wstring resultText;
            std::wstring timeText;

            // Lặp qua các chỉ số của thuật toán đã chọn và thực thi
            for (int index : selectedIndices) {
                std::vector<int> dataCopy = data;
                auto start = std::chrono::high_resolution_clock::now(); // Bắt đầu tính thời gian thực 

                wchar_t algoName[50];
                SendMessage(hAlgoList, LB_GETTEXT, index, (LPARAM)algoName);

                // Chỉ vẽ đồ họa nếu chỉ chọn một thuật toán 
                bool visualize = (selectedCount == 1);

                // Thực thi thuật toán sắp xếp theo chỉ số 
                switch (index) {
                case 0: BubbleSort(dataCopy, visualize ? hWnd : NULL); break;
                case 1: QuickSort(dataCopy, 0, dataCopy.size() - 1, visualize ? hWnd : NULL); break;
                case 2: MergeSort(dataCopy, 0, dataCopy.size() - 1, visualize ? hWnd : NULL); break;
                case 3: SelectionSort(dataCopy, visualize ? hWnd : NULL); break;
                case 4: InsertionSort(dataCopy, visualize ? hWnd : NULL); break;
                case 5: HeapSort(dataCopy, visualize ? hWnd : NULL); break;
                case 6: RadixSort(dataCopy, visualize ? hWnd : NULL); break;
                case 7: ShellSort(dataCopy, visualize ? hWnd : NULL); break;
                }

                auto end = std::chrono::high_resolution_clock::now(); // Kết thúc tính thời gian thực 
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start); //Tính toán thời gian thực thi 

                // Cập nhật kết quả và thời gian thực thi 
                resultText += std::wstring(algoName) + L": " + std::to_wstring(dataCopy[0]);
                for (size_t i = 1; i < dataCopy.size(); ++i) {
                    resultText += L" " + std::to_wstring(dataCopy[i]);
                }
                resultText += L"\r\n";

                timeText += std::wstring(algoName) + L" Execution Time: " + std::to_wstring(duration.count()) + L" µs\r\n";

                // Nếu có vẽ đồ họa, cập nhật lại dữ liệu và vẽ lại 
                if (visualize) {
                    data = dataCopy;
                    InvalidateRect(hWnd, NULL, TRUE);
                }
            }

            // Hiển thị kết quả và thời gian thực thi trên giao diện 
            SetWindowText(hOutput, resultText.c_str());
            SetWindowText(hTimeText, timeText.c_str());
        }
        else if (LOWORD(wParam) == IDC_RESET_BUTTON) {
            // Xóa dữ liệu và làm mới giao diện 
            SetWindowText(hInput, L"");
            SetWindowText(hOutput, L"");
            SetWindowText(hTimeText, L"");
            data.clear();
            SendMessage(hAlgoList, LB_SETSEL, FALSE, -1);
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);  // Cập nhật ngay lập tức giao diện 
        }
        else if (LOWORD(wParam) == IDC_LOAD_FILE_BUTTON) {
            // Mở hộp thoại để chọn file dữ liệu 
            OPENFILENAME ofn;
            wchar_t szFile[260] = { 0 };

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn) == TRUE) {
                data.clear();
                if (LoadDataFromFile(ofn.lpstrFile, data)) {
                    // Đọc nội dung từ file và hiển thi vào input 
                    std::wstring fileContent;
                    for (int num : data) {
                        fileContent += std::to_wstring(num) + L" ";
                    }
                    SetWindowText(hInput, fileContent.c_str());
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                else {
                    // Hiển thị thông báo lỗi nếu không mở được file
                    MessageBox(hWnd, L"Cannot open file!", L"Error", MB_OK);
                }
            }
        }
        else if (LOWORD(wParam) == IDC_SAVE_FILE_BUTTON) {
            // Mở hộp thoại để lưu dữ liệu ra file 
            OPENFILENAME ofn;
            wchar_t szFile[260] = { 0 };

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"Text Files\0*.TXT\0All Files\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetSaveFileName(&ofn) == TRUE) {
                if (!SaveDataToFile(ofn.lpstrFile, data)) {
                    // Hiển thị thông báo lỗi nếu không lưu được file 
                    MessageBox(hWnd, L"Cannot save file!", L"Error", MB_OK);
                }
            }
        }
        break;
    }
    case WM_PAINT: {
        // Vẽ lại giao diện khi có thay đổi 
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Xóa vùng vẽ cũ 
        RECT rect;
        GetClientRect(hWnd, &rect);
        FillRect(hdcMem, &rect, (HBRUSH)(COLOR_WINDOW + 1));

        // Vẽ lại các phần tử giao diện 
        RedrawUI(hdcMem);

        // Vẽ dữ liệu dưới dạng đồ họa 
        DrawVisualization(hdcMem, data);

        // Copy ảnh vẽ từ bộ nhớ sang cửa sổ 
        BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        DeleteDC(hdcMem);
        DeleteObject(hBitmap);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

std::vector<int> ParseInput(const std::wstring& input) {
    std::vector<int> arr;
    std::wistringstream ss(input);
    int num;
    while (ss >> num) {
        arr.push_back(num);
    }
    return arr;
}

bool LoadDataFromFile(const std::wstring& filename, std::vector<int>& data) {
    std::wifstream file(filename);
    if (!file.is_open()) return false;

    int num;
    while (file >> num) {
        data.push_back(num);
    }
    return true;
}

bool SaveDataToFile(const std::wstring& filename, const std::vector<int>& data) {
    std::wofstream file(filename);
    if (!file.is_open()) return false;

    // Sắp xếp dữ liệu 
    std::vector<int> sortedData = data;
    std::sort(sortedData.begin(), sortedData.end());

    // Lưu dữ liệu đã sắp xếp 
    for (int num : sortedData) {
        file << num << L" ";
    }

    return true;
}

void DrawVisualization(HDC hdc, const std::vector<int>& arr) {
    RECT rect;
    GetClientRect(hWndMain, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top - 470; // Dành không gian cho các phần tử giao diện 

    // Xóa vùng vẽ 
    RECT clearRect = { 0, 470, width, rect.bottom };
    FillRect(hdc, &clearRect, (HBRUSH)(COLOR_WINDOW + 1));

    if (arr.empty()) return;

    int barWidth = width / arr.size();
    int maxElement = *std::max_element(arr.begin(), arr.end());

    // Vẽ các thanh theo chiều cao dữ liệu 
    for (size_t i = 0; i < arr.size(); ++i) {
        int barHeight = static_cast<int>((static_cast<double>(arr[i]) / maxElement) * height);
        RECT barRect = { static_cast<LONG>(i * barWidth), height - barHeight + 470,
                         static_cast<LONG>((i + 1) * barWidth), height + 470 };

        // Tạo màu sắc gradient cho các thanh
        int hue = static_cast<int>((static_cast<double>(arr[i]) / maxElement) * 240);
        COLORREF color = RGB(255 - hue, hue, 128);

        HBRUSH hBrush = CreateSolidBrush(color);
        FillRect(hdc, &barRect, hBrush);
        DeleteObject(hBrush);

        // Vẽ giá trị số lên trên mỗi cột
        int textX = static_cast<LONG>(i * barWidth) + (barWidth / 2) - 10;  // X: centered on bar
        int textY = height - barHeight + 470 - 20;  // Y: 20px above the top of the bar
        std::wstring valueText = std::to_wstring(arr[i]);

        SetBkMode(hdc, TRANSPARENT);  // Transparent background for text
        SetTextColor(hdc, RGB(0, 0, 0));  // Black text color
        TextOut(hdc, textX, textY, valueText.c_str(), valueText.length());
    }
}


void RedrawUI(HDC hdc) {
    // Vẽ lại các thành phần giao diện 
    RECT inputRect = { 20, 20, 320, 70 };
    DrawEdge(hdc, &inputRect, EDGE_SUNKEN, BF_RECT);

    RECT listRect = { 20, 90, 220, 240 };
    DrawEdge(hdc, &listRect, EDGE_SUNKEN, BF_RECT);

    RECT sortButtonRect = { 240, 90, 320, 120 };
    DrawEdge(hdc, &sortButtonRect, EDGE_RAISED, BF_RECT);
    DrawText(hdc, L"Sort", -1, &sortButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    RECT resetButtonRect = { 240, 130, 320, 160 };
    DrawEdge(hdc, &resetButtonRect, EDGE_RAISED, BF_RECT);
    DrawText(hdc, L"Reset", -1, &resetButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    RECT loadButtonRect = { 340, 20, 440, 50 };
    DrawEdge(hdc, &loadButtonRect, EDGE_RAISED, BF_RECT);
    DrawText(hdc, L"Load File", -1, &loadButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    RECT saveButtonRect = { 340, 60, 440, 90 };
    DrawEdge(hdc, &saveButtonRect, EDGE_RAISED, BF_RECT);
    DrawText(hdc, L"Save File", -1, &saveButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Vẽ lại khung hiển thị đầu ra và thời gian 
    RECT outputRect = { 20, 250, 760, 350 };
    DrawEdge(hdc, &outputRect, EDGE_SUNKEN, BF_RECT);

    RECT timeRect = { 20, 360, 760, 460 };
    DrawEdge(hdc, &timeRect, EDGE_SUNKEN, BF_RECT);

    // Vẽ lại tiêu đề 
    RECT titleRect = { 20, 470, 760, 490 };
    DrawText(hdc, L"Sorting Visualization", -1, &titleRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// Thuật toán Bubble Sort
void BubbleSort(std::vector<int>& arr, HWND hWnd) {
    int n = arr.size();
    HDC hdc = GetDC(hWnd);  // Get the device context to draw

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            // Swap if the current element is greater than the next element
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);

                // Update the interface after each swap
                if (hWnd) {
                    // Redraw the visualization with the updated array
                    InvalidateRect(hWnd, NULL, TRUE);  // Invalidate the window
                    UpdateWindow(hWnd);  // Force the window to repaint

                    // Draw the updated array
                    DrawVisualization(hdc, arr);

                    Sleep(1000);  // Wait for 1 second to allow user to see the update
                }
            }
        }
    }

    ReleaseDC(hWnd, hdc);  // Release the device context
}


// Thuật toán sắp xếp nhanh (Quick Sort)
void QuickSort(std::vector<int>& arr, int low, int high, HWND hWnd) {
    if (low < high) {
        // Chọn pivot và sắp xếp các phần tử xung quanh pivot
        int pi = [&]() {
            int pivot = arr[high];
            int i = (low - 1);
            for (int j = low; j <= high - 1; j++) {
                if (arr[j] < pivot) {
                    i++;
                    std::swap(arr[i], arr[j]);

                    // Cập nhật giao diện sau mỗi lần đổi chỗ
                    if (hWnd) {
                        // Vẽ lại giao diện với mảng đã cập nhật
                        HDC hdc = GetDC(hWnd);  // Lấy context thiết bị để vẽ
                        InvalidateRect(hWnd, NULL, TRUE);  // Đánh dấu lại cửa sổ
                        UpdateWindow(hWnd);  // Buộc cửa sổ phải vẽ lại

                        // Vẽ mảng hiện tại
                        DrawVisualization(hdc, arr);
                        ReleaseDC(hWnd, hdc);  // Giải phóng context thiết bị

                        Sleep(500);  // Chờ 500ms để người dùng thấy sự thay đổi
                    }
                }
            }

            // Đổi vị trí phần tử pivot vào vị trí đúng của nó
            std::swap(arr[i + 1], arr[high]);

            // Cập nhật giao diện sau khi đặt pivot đúng vị trí
            if (hWnd) {
                HDC hdc = GetDC(hWnd);
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
                DrawVisualization(hdc, arr);
                ReleaseDC(hWnd, hdc);

                Sleep(500);
            }

            return (i + 1);
            }();

        // Đệ quy sắp xếp các phần tử bên trái và phải của pivot
        QuickSort(arr, low, pi - 1, hWnd);
        QuickSort(arr, pi + 1, high, hWnd);
    }
}

void Merge(std::vector<int>& arr, int left, int mid, int right, HWND hWnd) {
    // Tạo mảng con trái và phải
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<int> L(n1), R(n2);

    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;

    // Trộn các mảng con
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;

        // Cập nhật giao diện sau mỗi lần trộn
        if (hWnd) {
            HDC hdc = GetDC(hWnd);  // Lấy context thiết bị để vẽ
            InvalidateRect(hWnd, NULL, TRUE);  // Đánh dấu lại cửa sổ
            UpdateWindow(hWnd);  // Buộc cửa sổ phải vẽ lại

            // Vẽ mảng hiện tại
            DrawVisualization(hdc, arr);
            ReleaseDC(hWnd, hdc);  // Giải phóng context thiết bị

            Sleep(1000);  // Chờ 500ms để người dùng thấy sự thay đổi
        }
    }

    // Nếu còn phần tử trong mảng con trái
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;

        // Cập nhật giao diện
        if (hWnd) {
            HDC hdc = GetDC(hWnd);
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);
            DrawVisualization(hdc, arr);
            ReleaseDC(hWnd, hdc);

            Sleep(1000);
        }
    }

    // Nếu còn phần tử trong mảng con phải
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;

        // Cập nhật giao diện
        if (hWnd) {
            HDC hdc = GetDC(hWnd);
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);
            DrawVisualization(hdc, arr);
            ReleaseDC(hWnd, hdc);

            Sleep(500);
        }
    }
}
void MergeSort(std::vector<int>& arr, int left, int right, HWND hWnd) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        MergeSort(arr, left, mid, hWnd);
        MergeSort(arr, mid + 1, right, hWnd);

        Merge(arr, left, mid, right, hWnd);
    }
}

void SelectionSort(std::vector<int>& arr, HWND hWnd) {
    int n = arr.size();
    HDC hdc = GetDC(hWnd);  // Get the device context to draw

    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;

        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }

        if (min_idx != i) {
            // Swap if the minimum element is not at the current position
            std::swap(arr[min_idx], arr[i]);

            // Update the interface after each swap
            if (hWnd) {
                // Invalidate and update window
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);

                // Draw the updated array
                DrawVisualization(hdc, arr);

                // Sleep to allow user to see the update
                Sleep(1000);  // Adjust the delay as necessary
            }
        }
    }

    ReleaseDC(hWnd, hdc);  // Release the device context
}


void InsertionSort(std::vector<int>& arr, HWND hWnd) {
    int n = arr.size();
    HDC hdc = GetDC(hWnd);  // Get the device context to draw

    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;

        // Move elements of arr[0..i-1] that are greater than key to one position ahead
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;

            // Update the interface after each move
            if (hWnd) {
                InvalidateRect(hWnd, NULL, TRUE);  // Invalidate the window
                UpdateWindow(hWnd);  // Force the window to repaint

                // Draw the updated array
                DrawVisualization(hdc, arr);

                Sleep(1000);  // Wait for 1 second to allow user to see the update
            }
        }
        arr[j + 1] = key;

        // Redraw the array after the insertion step
        if (hWnd) {
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);
            DrawVisualization(hdc, arr);
            Sleep(1000);  // Wait for 1 second to allow user to see the update
        }
    }

    ReleaseDC(hWnd, hdc);  // Release the device context
}


void Heapify(std::vector<int>& arr, int n, int i, HWND hWnd) {
    int largest = i;
    int l = 2 * i + 1;
    int r = 2 * i + 2;

    if (l < n && arr[l] > arr[largest])
        largest = l;

    if (r < n && arr[r] > arr[largest])
        largest = r;

    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        if (hWnd) {
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);
            Sleep(10);
        }

        Heapify(arr, n, largest, hWnd);
    }
}

void HeapSort(std::vector<int>& arr, HWND hWnd) {
    int n = arr.size();

    // Build the heap (rearrange the array)
    for (int i = n / 2 - 1; i >= 0; i--) {
        Heapify(arr, n, i, hWnd);
    }

    // One by one extract elements from the heap
    for (int i = n - 1; i > 0; i--) {
        // Move current root to the end
        std::swap(arr[0], arr[i]);

        // Update the interface after the swap
        if (hWnd) {
            InvalidateRect(hWnd, NULL, TRUE);  // Invalidate the window
            UpdateWindow(hWnd);  // Force the window to repaint

            // Draw the updated array after the swap
            HDC hdc = GetDC(hWnd);
            DrawVisualization(hdc, arr);
            ReleaseDC(hWnd, hdc);  // Release the device context

            Sleep(1000);  // Wait for 1 second to allow user to see the update
        }

        // Call Heapify on the reduced heap
        Heapify(arr, i, 0, hWnd);
    }
}

void CountingSort(std::vector<int>& arr, int exp, HWND hWnd) {
    std::vector<int> output(arr.size());
    std::vector<int> count(10, 0);

    // Counting occurrences of digits
    for (int i = 0; i < arr.size(); i++) {
        count[(arr[i] / exp) % 10]++;
    }

    // Update the visualization after counting
    if (hWnd) {
        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
        HDC hdc = GetDC(hWnd);
        DrawVisualization(hdc, arr);  // Draw the array after counting phase
        ReleaseDC(hWnd, hdc);
        Sleep(500);  // Small delay to show counting update
    }

    // Modify the count array to store actual positions of digits
    for (int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
    }

    // Update the visualization after modifying count array
    if (hWnd) {
        InvalidateRect(hWnd, NULL, TRUE);
        UpdateWindow(hWnd);
        HDC hdc = GetDC(hWnd);
        DrawVisualization(hdc, arr);  // Draw the array after modifying count array
        ReleaseDC(hWnd, hdc);
        Sleep(500);  // Small delay to show update
    }

    // Build the output array
    for (int i = arr.size() - 1; i >= 0; i--) {
        output[count[(arr[i] / exp) % 10] - 1] = arr[i];
        count[(arr[i] / exp) % 10]--;
    }

    // Copy the output array to arr
    for (int i = 0; i < arr.size(); i++) {
        arr[i] = output[i];

        // Update the interface after each element is copied
        if (hWnd) {
            InvalidateRect(hWnd, NULL, TRUE);
            UpdateWindow(hWnd);
            HDC hdc = GetDC(hWnd);
            DrawVisualization(hdc, arr);  // Draw the updated array
            ReleaseDC(hWnd, hdc);
            Sleep(500);  // Small delay to allow user to see the update
        }
    }
}


void RadixSort(std::vector<int>& arr, HWND hWnd) {
    int max = *std::max_element(arr.begin(), arr.end());

    for (int exp = 1; max / exp > 0; exp *= 10)
        CountingSort(arr, exp, hWnd);
}

void ShellSort(std::vector<int>& arr, HWND hWnd) {
    int n = arr.size();

    // Iterate over gap values (starting from n/2 and halving each time)
    for (int gap = n / 2; gap > 0; gap /= 2) {
        // Perform insertion sort for the current gap value
        for (int i = gap; i < n; i++) {
            int temp = arr[i];
            int j;
            // Shift elements of arr[0..i-gap] that are greater than temp
            for (j = i; j >= gap && arr[j - gap] > temp; j -= gap) {
                arr[j] = arr[j - gap];

                // Update the interface after each shift
                if (hWnd) {
                    InvalidateRect(hWnd, NULL, TRUE);
                    UpdateWindow(hWnd);
                    HDC hdc = GetDC(hWnd);
                    DrawVisualization(hdc, arr);  // Redraw the array
                    ReleaseDC(hWnd, hdc);
                    Sleep(500);  // Small delay to show the shift
                }
            }
            arr[j] = temp;

            // Update the interface after inserting the element into its correct position
            if (hWnd) {
                InvalidateRect(hWnd, NULL, TRUE);
                UpdateWindow(hWnd);
                HDC hdc = GetDC(hWnd);
                DrawVisualization(hdc, arr);  // Redraw the array after insertion
                ReleaseDC(hWnd, hdc);
                Sleep(500);  // Small delay to show the insertion
            }
        }
    }
}
