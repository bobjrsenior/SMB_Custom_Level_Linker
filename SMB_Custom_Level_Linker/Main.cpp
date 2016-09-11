#include <iostream>
#include <string>
#include <windows.h>
#using <mscorlib.dll>
#using <System.dll>
#using <System.Windows.Forms.dll>

void stringConvert(System::String ^ s, std::string& os) {
	using namespace System::Runtime::InteropServices;
	const char* chars =
		(const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
	os = chars;
	Marshal::FreeHGlobal(System::IntPtr((void*)chars));
}

int sleep() {
	Sleep(500);
	return 1;
}

[System::STAThreadAttribute]
int main(int argc, char* argv[]) {
	std::string yoshiFile = "";
	std::string isoFilePath = "";
	std::string objFilePath = "";
	std::string objFileLoc = "";
	std::string configFilePath = "";
	std::string stageNumber = "001";
	if (argc < 5) {
		System::Windows::Forms::OpenFileDialog^ openDialog = gcnew System::Windows::Forms::OpenFileDialog;
		openDialog->Title = "Find the SMB iso";
		openDialog->Filter = "iso file (*.iso)|*.iso";
		openDialog->FilterIndex = 2;
		openDialog->RestoreDirectory = true;
		openDialog->AddExtension = true;

		std::cout << "Choose the SMB iso" << std::endl;
		if (openDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			std::string filename;
			stringConvert(openDialog->SafeFileName, filename);
			stringConvert(openDialog->FileName, isoFilePath);
			std::cout << "Opened iso: " << filename << std::endl;
		}
		else {
			std::cout << "Failed to open iso" << std::endl;
			return 0;
		}

		char smbNumber;
		std::cout << "What SMB Verion is this (1 or 2): ";
		std::cin >> smbNumber;

		if (smbNumber == '1') {
			yoshiFile = ".\\yoshiConverter\\smb1.exe ";
		}
		else if (smbNumber == '2') {
			yoshiFile = ".\\yoshiConverter\\smb2.exe ";
		}
		else {
			std::cout << "Bad SMB Number" << std::endl;
			return 0;
		}

		openDialog->Title = "Find the Object file";
		openDialog->Filter = "Object file (*.obj)|*.obj";
		openDialog->FileName = "st001.obj";
		std::cout << "Choose the Object file" << std::endl;
		if (openDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			std::string filename;
			stringConvert(openDialog->SafeFileName, filename);
			stringConvert(openDialog->FileName, objFilePath);
			std::cout << "Opened Object File: " << filename << std::endl;

			stageNumber = filename.substr(filename.length() - 7, 3);

			objFileLoc = objFilePath.substr(0, objFilePath.length() - filename.length());

		}
		else {
			std::cout << "Failed to open Object FIle" << std::endl;
			return 0;
		}

		openDialog->Title = "Find the Config file";
		openDialog->Filter = "Config file (*.*)|*.*";
		openDialog->FileName = "Config.txt";
		std::cout << "Choose the Config file" << std::endl;
		if (openDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
		{
			std::string filename;
			stringConvert(openDialog->SafeFileName, filename);
			stringConvert(openDialog->FileName, configFilePath);
			std::cout << "Opened Config File: " << filename << std::endl;
		}
		else {
			std::cout << "Failed to open Config File" << std::endl;
			return 0;
		}

	}
	else {
		isoFilePath.assign(argv[1]);
		char smbNumber = argv[2][0];
		if (smbNumber == '1') {
			yoshiFile = ".\\yoshiConverter\\smb1.exe ";
		}
		else if (smbNumber == '2') {
			yoshiFile = ".\\yoshiConverter\\smb2.exe ";
		}
		else {
			std::cout << "Bad SMB Number" << std::endl;
			return 0;
		}
		objFilePath.assign(argv[3]);
		configFilePath.assign(argv[4]);

		char fname[256];

		_splitpath(objFilePath.c_str(), NULL, NULL, fname, NULL);
		std::string filename(fname);
		stageNumber = filename.substr(filename.length() - 3, 3);

		objFileLoc = objFilePath.substr(0, objFilePath.length() - filename.length() - 4);

	}

	system(("mkdir \"" + objFileLoc + "temp\"").c_str());
	Sleep(100);
	if (!system(("START /D \"" + objFileLoc + "\" " + yoshiFile + "\"" + objFilePath + "\" \"" + configFilePath + "\"").c_str())) {
		std::cout << "SUCCESS: Writing raw stage lz file" << std::endl;
		Sleep(500);
		if (!system(("START /D \"" + objFileLoc + "temp\" .\\lzdec\\LZSCDec.exe output.lz.raw C output.lz.raw.lzs").c_str()) && sleep() && !system(("START /D \"" + objFileLoc + "temp\" .\\lzdec\\lzfix.exe").c_str())) {
			std::cout << "SUCCESS: Compressing stage lz file" << std::endl;
			Sleep(100);
			if (!system(("move /y \"" + objFileLoc + "temp\\output.lz\" \"" + objFileLoc + "temp\\STAGE" + stageNumber + ".lz\"").c_str())) {
				std::cout << "SUCCESS: Renamed stage lz file to STAGE" << stageNumber << ".lz" << std::endl;
				Sleep(100);
				if (!system((".\\GxModelViewerNoGUI\\GxModelViewer.exe \"" + objFilePath + "\"").c_str())) {
					std::cout << "SUCCESS: Converting Object file to gma and tpl files" << std::endl;
					Sleep(100);
					int returnValue = system((".\\gcr_1.0_No_UI\\gcr_v1.0.exe \"" + isoFilePath + "\" \"" + objFileLoc + "temp\\STAGE" + stageNumber + ".lz\" \"" + objFileLoc + "st" + stageNumber + ".gma\" \"" + objFileLoc + "st" + stageNumber + ".tpl\"").c_str());
					System::String^ titleText;
					System::String^ text;
					if(returnValue == 0){
						std::cout << "Success: Importing level into the SMB iso" << std::endl;
						std::cout << "Done" << std::endl;
						text = gcnew System::String("Completed Successfully");
						titleText = gcnew System::String("Done");
					}
					else if (returnValue == -1) {
						std::cout << "FAILURE: Couldn't find iso file" << std::endl;
						text = gcnew System::String("FAILURE: Couldn't find iso file");
						titleText = gcnew System::String("Failure");
					}
					else if (returnValue == 2) {
						std::cout << "Partial Failuer: lz file was too big to import" << std::endl;
						text = gcnew System::String("Partial Failuer: lz file was too big to import");
						titleText = gcnew System::String("Failure");
					}
					else if (returnValue == 3) {
						std::cout << "Partial Failuer: gma file was too big to import" << std::endl;
						text = gcnew System::String("Partial Failuer: gma file was too big to import");
						titleText = gcnew System::String("Failure");
					}
					else if (returnValue == 4) {
						std::cout << "Partial Failuer: tpl file was too big to import" << std::endl;
						text = gcnew System::String("Partial Failuer: tpl file was too big to import");
						titleText = gcnew System::String("Failure");
					}
					else if (returnValue == 5) {
						std::cout << "Partial Failuer: lz and gma file was too big to import" << std::endl;
						text = gcnew System::String("Partial Failuer: lz and gma file was too big to import");
						titleText = gcnew System::String("Failure");
					}
					else if (returnValue == 6) {
						std::cout << "Partial Failuer: lz and tpl file was too big to import" << std::endl;
						text = gcnew System::String("Partial Failuer: lz and tpl file was too big to import");
						titleText = gcnew System::String("Failure");
					}
					else if (returnValue == 7) {
						std::cout << "Partial Failuer: gma and tpl file was too big to import" << std::endl;
						text = gcnew System::String("Partial Failuer: gma and tpl file was too big to import");
						titleText = gcnew System::String("Failure");
					}
					else if (returnValue == 9) {
						std::cout << "Partial Failuer: lz, gma and tpl file was too big to import" << std::endl;
						text = gcnew System::String("Partial Failuer: lz, gma and tpl file was too big to import");
						titleText = gcnew System::String("Failure");
					}
					Sleep(50);
					if (argc < 5) {
						System::Windows::Forms::MessageBox::Show(titleText, text,
							System::Windows::Forms::MessageBoxButtons::OK, System::Windows::Forms::MessageBoxIcon::None,
							System::Windows::Forms::MessageBoxDefaultButton::Button1, System::Windows::Forms::MessageBoxOptions::DefaultDesktopOnly);
					}
					Sleep(50);

				}
				else {
					std::cout << "FAILED: Converting Object file to gma and tpl files" << std::endl;
				}
			}
			else {
				std::cout << "FAILED: Renaming stage lz file" << std::endl;
			}
		}
		else {
			std::cout << "FAILED: Compressing stage lz file" << std::endl;
		}
	}
	else {
		std::cout << "FAILED: Converting Object and Config file to stage.lz file" << std::endl;
	}



	return 0;
}