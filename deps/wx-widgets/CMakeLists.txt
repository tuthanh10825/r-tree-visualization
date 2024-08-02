message(STATUS "Fetching wxWidgets...")
set(wxBUILD_SHARED OFF)

FetchContent_MakeAvailable(wx-widgets)
