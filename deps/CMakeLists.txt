include(FetchContent)
FetchContent_Declare(
	wx-widgets
	URL https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.5/wxWidgets-3.2.5.zip
	URL_HASH SHA1=dda19cebdcc1906902d88edcad1c052f40c9460b
)
add_subdirectory("wx-widgets")