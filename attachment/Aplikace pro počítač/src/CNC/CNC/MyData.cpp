#include "MyData.h"
#include <algorithm>
#include <sstream>
#include "CommunicationEnumerations.h"
#include "GCodeInterpreter.h"

MyData::MyData(void)
{
}


MyData::~MyData(void)
{
}

void MyData::SetDependences(MainWindow* w, MyDevice*d)
{
	window = w;
	settings.SetDependencies(d);
}

void MyData::OpenFile()
{
	wxFileDialog* openFile = new wxFileDialog(window, wxT("Otevøít soubor"), wxT(""), wxT(""), wxT("*"), wxFD_FILE_MUST_EXIST | wxFD_OPEN);
	if(openFile->ShowModal() != wxID_OK)
		return;
	file = openFile->GetPath();
	ProcessFile();
}

void MyData::ProcessFile()
{
	ifstream stream(file.c_str());//Open the file
	if(!stream.is_open())
	{
		PostErrorMessageC(wxT("Neleze otevøít zadaný soubor. Zkontrolujte pøístupová práva a akci opakujte."));
		return;
	}
	string firstLine;
	getline(stream, firstLine);
	if(firstLine != fileID)
	{
		PostErrorMessageC(wxT("Zadaný soubor neodpovídá formátu. Vyberte jiný soubor."));
		return;
	}
	//ready to load
	//Clear data
	unprocessedData.clear();
	do
	{
		string temp;
		getline(stream, temp);//Just load the lines and process it
		unprocessedData.push_back(temp);
	}
	while(stream.good());
	//Process data
	if(ProcessData())
		return;
	//Show data
	window->codeView->Clear();
	wxString temp;
	for(int i = 0; i != unprocessedData.size(); i++)
	{
		temp += wxString(unprocessedData[i].c_str(), wxConvUTF8);
		temp += '\n';
	}
	*window->codeView << temp;
	window->codeView->SetInsertionPoint(0);
}

bool MyData::ProcessData()
{
	processedData.clear();
	int32_t IDcounter = 1;
	float lastSpeed = 0;
	for_each(unprocessedData.begin(), unprocessedData.end(), [&](string s)
	{
		istringstream stream(s);
		char c;
		stream >> c;
		switch(c)
		{
			case 'l':
			case 'L':
				{
					//Process line
					CommandStruct temp;
					temp.ID = IDcounter;
					IDcounter++;
					temp.type = COM_AXIS_LINE;
					temp.receiver = COM_RECEIVER_AXIS;
					for(size_t i = 0; i != 3; i++)
					{
						stream >> temp.line.axes[i];
					}
					//Speeds
					stream >> temp.line.v;
					stream >> temp.line.vb;
					temp.line.v0 = lastSpeed;
					lastSpeed = temp.line.vb;
					ProcessedData p;
					p.data = temp;
					processedData.push_back(p);
				}
				break;
			case 'c':
			case 'C':
				{
					//Process circle
					CommandStruct temp;
					temp.ID = IDcounter;
					IDcounter++;
					temp.type = COM_AXIS_CIRCLE;
					temp.receiver = COM_RECEIVER_AXIS;
					for(size_t i = 0; i != 3; i++)
					{
						stream >> temp.circle.B[i];
					}
					for(size_t i = 0; i != 3; i++)
					{
						stream >> temp.circle.C[i];
					}
					//Direction
					//Speeds
					stream >> temp.circle.v;
					stream >> temp.circle.vb;
					temp.circle.v0 = lastSpeed;
					lastSpeed = temp.circle.vb;
					ProcessedData p;
					p.data = temp;
					processedData.push_back(p);
				}
				break;
			case 's':
			case 'S':
				{
					//Process sine
					CommandStruct temp;
					temp.ID = IDcounter;
					IDcounter++;
					temp.type = COM_AXIS_SINE;
					temp.receiver = COM_RECEIVER_AXIS;
					for(size_t i = 0; i != 3; i++)
					{
						stream >> temp.sine.C[i];
					}
					for(size_t i = 0; i != 3; i++)
					{
						stream >> temp.sine.B[i];
					}
					//Speeds
					stream >> temp.sine.v;
					lastSpeed = temp.sine.v;
					ProcessedData p;
					p.data = temp;
					processedData.push_back(p);
				}
				break;
		}
	});
	return false;
}


void MyData::OpenGcode()
{
	wxFileDialog* openFile = new wxFileDialog(window, wxT("Otevøít soubor"), wxT(""), wxT(""), wxT("*"), wxFD_FILE_MUST_EXIST | wxFD_OPEN);
	if(openFile->ShowModal() != wxID_OK)
	{
		delete openFile;
		return;
	}
	file = openFile->GetPath();
	std::ifstream t(file.c_str());
	//Load program into string
	std::string code((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());

	//Process G-code to commands
	GCodeInterpreter interpreter;
	interpreter.ProcessString(code, processedData,
		window->middlePanel->GetToDrawPointer(),
		window->middlePanel->GetToDrawOffsetPointer());

	//GUI
	window->codeView->Clear();
	*window->codeView << wxString(code.c_str(), wxConvUTF8);
	window->middlePanel->DrawDataFinished();

	size_t index = file.find_last_of('\\') + 1;
	window->SetProgramName(wxString(file, index, file.size() - index));

	delete openFile;
}