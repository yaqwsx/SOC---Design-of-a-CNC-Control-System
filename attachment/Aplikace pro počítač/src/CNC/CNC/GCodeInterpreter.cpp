#define _USE_MATH_DEFINES
#include "GCodeInterpreter.h"
#include <limits>
#include <cmath>
#include <algorithm>
#include <iterator>
#include "global.h"
#include "iniFile.h"
#include "CommunicationEnumerations.h"
#include "PathPartMark.h"


GCodeInterpreter::GCodeInterpreter(void): output(0), workingPlane(XY), lastMovementFunction(-1)
{
	FillTables();
	for(char u = 'A'; u != 'Z'+1; u++)
		lastValue[u] = 0;
}


GCodeInterpreter::~GCodeInterpreter(void)
{
}

//The main method of the class - processes string containing G-code to the vector of commands for the interpreter
bool GCodeInterpreter::ProcessString(string code, vector<ProcessedData>& output, vector<PathPartSimple>* simpleOutput, vector<PathPartSimple>* offsetOutput)
{
	//Prepare
	output.clear();
	this->output = &output;
	try
	{
		LoadConfiguration();
		ParseCodeToPath(code);
		PostProcessPath();
		//assert(IsPathValid());
		if(simpleOutput)
			CreateSimpleOutput(simpleOutput);
		if(offsetOutput)
			CreateOffsetOutput(offsetOutput);
		TunePath();
		assert(IsPathValid());
		TransformPathToCommands();
	}
	catch(exception ex)
	{
		wxString text(ex.what(), *wxConvCurrent);
		wxMessageBox(text, wxT("Chyba!"), wxOK | wxICON_EXCLAMATION);
		return false;
	}
	//Clean
	commands.clear();
	path.clear();
	return true;
}

bool GCodeInterpreter::IsMovementFunction(GCodeLine& command)
{
	const float functions[] = {0,1,2,3,12,13,28};
	size_t G;
	if(command.ParamExists('G'))
		G = command.GetParam('G');
	if(find(functions, functions + sizeof(functions)/sizeof(float), G) != functions + sizeof(functions)/sizeof(float))
		return true;
	return false;
}

bool GCodeInterpreter::IsMovementFunction(GFunction& f)
{
	const float movementTable[] = {0, 1, 2, 3};//Temp definition for current version, needs to be endhanced
	if(f.first != 'G')
		return false;
	return any_of(movementTable, movementTable + sizeof(movementTable), [&] (float p)->bool{return p==f.second;});
}

bool GCodeInterpreter::IsPathModifiableFunction(GCodeLine& command)
{
	if(command.ParamExists('F'))
		return true;
	const float functions[] = {17, 18, 19, 20, 21, 41, 42, 43, 44, 49};
	if(find(functions, functions + sizeof(functions)/sizeof(float), command.GetParam('G')) != functions + sizeof(functions)/sizeof(float))
		return true;
	return false;
}

void GCodeInterpreter::ParseCodeToPath(string& code)
{
	//Add starting mark
	unique_ptr<PathPart> part(new PathPartMark(PROGRAM_BEGIN, "Zaèátek programu", 0));
	AddPathSegment(part);
	istringstream s(code);
	string line;
	size_t lineCounter = 0;
	while(getline(s, line))
	{
		lineCounter++;
		auto data = ParseGCodeLine(line);
		auto comm = SeparateCommandsFromLine(data, lineCounter);
		for_each(comm.begin(), comm.end(), [&](GCodeLine& c)
		{
			//Process command to PathPart
			ConvertCommandToPath(c);
		});
	}
	//For sure, add ending mark
	AddPathSegment(unique_ptr<PathPart>(new PathPartMark(PROGRAM_END, "Konec programu", 0)));
}

void GCodeInterpreter::LoadConfiguration()
{
	CIniFile ini;
	rapidFeed=StringToFloat(ini.GetValue("defaultRapidFeed", "gcode", iniFile));
	standardFeed=StringToFloat(ini.GetValue("defaultStandardFeed", "gcode", iniFile));
	maxAcceleration=StringToFloat(ini.GetValue("maxLinearAcceleration", "axis", iniFile));
	maxJerk=StringToFloat(ini.GetValue("maxLinearJerk", "axis", iniFile));
	maxRapidTollerance=StringToFloat(ini.GetValue("maxRapidTollerance", "gcode", iniFile));
	maxStandardTollerance=StringToFloat(ini.GetValue("maxStandardTollerance", "gcode", iniFile));
	unitMultiply=StringToFloat(ini.GetValue("defaultUnits", "gcode", iniFile));
	string temp = ini.GetValue("defaultWorkingPlane", "gcode", iniFile);
	if(temp == "XY")
		workingPlane = XY;
	else if(temp == "YZ")
		workingPlane = YZ;
	else if(temp == "XY")
		workingPlane = XY;
	else
	{
		wxString text = wxT("Špatnì nastavená pracovní rovina v ini souboru. Použít XY a pokraèovat?");
		if(wxMessageBox(text, wxT("Chyba!"), wxYES_NO | wxICON_EXCLAMATION) == wxNO)
				throw(exception("Operace ukonèena"));
		workingPlane = XY;
	}

	temp = ini.GetValue("defaultCoordType", "gcode", iniFile);
	if(temp == "absolute" || temp == "abs" || temp == "ABS")
		coordType = ABS;
	else if(temp == "relative" || temp == "rel" || temp == "REL" 
		|| temp == "incremental" || temp == "inc" || temp == "INC")
		coordType = INC;
	else
	{
		wxString text = wxT("Špatnì nastavený typ souøadnic v ini souboru. Použít absolutní a pokraèovat?");
		if(wxMessageBox(text, wxT("Chyba!"), wxYES_NO | wxICON_EXCLAMATION) == wxNO)
				throw(exception("Operace ukonèena"));
		coordType = ABS;
	}

	temp = ini.GetValue("defaultArcCoordType", "gcode", iniFile);
	if(temp == "absolute" || temp == "abs" || temp == "ABS")
		arcCoordType = ABS;
	else if(temp == "relative" || temp == "rel" || temp == "REL" 
		|| temp == "incremental" || temp == "inc" || temp == "INC")
		arcCoordType = INC;
	else
	{
		wxString text = wxT("Špatnì nastavený typ souøadnic pro oblouky v ini souboru. Použít absolutní a pokraèovat?");
		if(wxMessageBox(text, wxT("Chyba!"), wxYES_NO | wxICON_EXCLAMATION) == wxNO)
				throw(exception("Operace ukonèena"));
		arcCoordType = ABS;
	}

}

//Computes speeds
void GCodeInterpreter::TunePath()
{
	auto end = path.rend();
	for(auto i = path.rbegin(); i != end; i++)
	{
		(*i)->ComputeSpeeds(*this);
	}
}

void GCodeInterpreter::TransformPathToCommands()
{
	for_each(path.begin(), path.end(), [&](unique_ptr<PathPart>& part)
	{
		if(part->IsCommand())
			output->push_back(part->TransformToCommand());
	});
}

//Test for float for a NaN or 1.#IND
bool IsValid(float f)
{
	return f == f;
}


//Output for drawing purposes
void GCodeInterpreter::CreateSimpleOutput(vector<PathPartSimple>* output)
{
	for_each(path.begin(), path.end(), [&](unique_ptr<PathPart>& p)
	{
		if(p->GetType() != UNPROC)
		{
			output->push_back(p->ToSimple());
		}
	});
}

map<char, vector<float>> GCodeInterpreter::ParseGCodeLine(string line)
{
	map<char, vector<float>> ret;

	istringstream s(line);
	char p;
	bool firstChar = true;
	while(s.get(p))
	{
		p = toupper(p);
		//Skip comments
		if(p == '(')
		{
			do s.get(p);
			while(p != ')' && !s.eof());
			if(s.eof())
			{
				//Error - illegal command
				stringstream s;
				s << "Neukonèený komentáø na øádku: " << line;
				throw exception(s.str().c_str());
			}
			continue;
		}
		if(p == ';')//Rest of the line is comment
			break;
		if(p == '%')//This line marks end or beginning of the program
			break;
		if(isspace(p))
			continue;
		float temp;
		s >> temp;
		if(ret.find(p) == ret.end())
			ret[p] = vector<float>();
		ret[p].push_back(temp);
	}
	return ret;
}

void GCodeInterpreter::PostProcessPath()
{
	ProcessPathRoundness();
	ProcessPathOffsets();
}

void GCodeInterpreter::ProcessPathRoundness()//Replaces sharp corners with sine
{
	//Not implemented
}

void GCodeInterpreter::ProcessPathOffsets()//Processes tool compensation
{
	PathPart* part = &(**path.begin());
	if(!part->IsMovable())
		part = &part->GetNextMovable();
	while(part->GetType() != PROGRAM_END)
	{
		part->ProcessToolCompensation(*this);
		part = &part->GetNextMovable();
	}
}

void GCodeInterpreter::FillTables()
{
	//Format Function, args; args; ...
	ifstream in("gcode.dat");
	string line;
	while(getline(in, line))
	{
		istringstream i(line);
		GFunction f;
		FunctionArguments args;
		i >> f.first >> f.second;
		while(i.good())
		{
			char c;
			vector<char> temp;
			while(i >> c && c != ';')
				temp.push_back(c);
			if(!temp.empty())
				args.push_back(temp);
		}
		orderTable.push_back(f);
		functionPrototypes[f] = args;
	}
	//Converting functions
	FillGTable();
}

vector<GCodeLine> GCodeInterpreter::SeparateCommandsFromLine(map<char, vector<float>>& data, size_t line)
{
	//Check if there isn't unknown function
	AssertExistingFunction(data, 'G', line);
	AssertExistingFunction(data, 'M', line);
	vector<GCodeLine> ret;
	for_each(orderTable.begin(), orderTable.end(), [&](GFunction& fun){
		auto it = data.find(fun.first);
		if(it == data.end())
			return;//This function prototype isn't in current line
		if(fun.second != -1)
		{
			auto it2 = find(it->second.begin(), it->second.end(), fun.second);
			if(it2 == it->second.end())
				return;//Function number doesn't fit, interrupt
		}
		//Function exists, use params
		GCodeLine func = AssignGFuncParams(data, fun, line);
		if(IsMovementFunction(fun))
			lastMovementFunction = fun.second;
		ret.push_back(func);
	});
	//Test if there's a G- or M-function - if not, try to use the old G;
	if(data.find('G') == data.end() && data.find('M') == data.end() &&
		data.find('S') == data.end() && data.find('F') == data.end() &&
		!data.empty() && lastMovementFunction != -1)
	{
		//Using old GCode
		GFunction fun('G', lastMovementFunction);
		GCodeLine func = AssignGFuncParams(data, fun, line);
		ret.push_back(func);
	}
	//Set old parameters
	for_each(data.begin(), data.end(), [&] (pair<char, vector<float>> p)
	{
		lastValue[p.first] = p.second.back();
	});
	return ret;
}

bool GCodeInterpreter::MatchesArgumentList(vector<char>& prototype, map<char, vector<float>>& data)
{
	return prototype.size() == 0 || all_of(prototype.begin(), prototype.end(), [&](char c)->bool {
		if(data.find(c) != data.end())
			return true;
		return false;
	});
}
bool GCodeInterpreter::MatchesArgumentListWithOldVariables(vector<char>& prototype, map<char, vector<float>>& data)
{
	return all_of(prototype.begin(), prototype.end(), [&](char c)->bool {
		if(data.find(c) != data.end() || lastValue.find(c) != lastValue.end())
			return true;
		return false;
	});
}

void GCodeInterpreter::AssertExistingFunction(map<char, vector<float>>& data, char type, size_t line)
{
	assert(type == 'G' || type == 'M');
	map<char, vector<float>>::iterator dataIter;
	if((dataIter = data.find(type)) != data.end())
	{
		for_each(dataIter->second.begin(), dataIter->second.end(), [&](float f)
		{
			GFunction func(type, f);
			if(none_of(orderTable.begin(), orderTable.end(), [&func](GFunction& g){return g==func;}))
			{
				string error = "Nalezena neznámá funkce ";
				error += func.first;
				error += func.second;
				error += string(" na øádku ") + line + ".";
				throw exception(error.c_str());
			}
		});
	}
}

GCodeLine GCodeInterpreter::AssignGFuncParams(map<char, vector<float>>& data, GFunction& fun, size_t line)
{
	GCodeLine func;
	func.function = fun;
	func.line = line;
	FunctionArguments args = functionPrototypes[fun];
	if(args.size() == 0)
	{
		//Function with no parameters
		if(fun.second == -1)
			func.data[fun.first] = data[fun.first][0];
		else
			func.data[fun.first] = fun.second;
		return func;
	}
	auto iterator = find_if(args.begin(), args.end(), [&] (vector<char>& a)->bool
	{
		return MatchesArgumentList(a, data);//there are no arguments
	});
	if(iterator == args.end())//Match didn't found, use  previous arguments
		iterator = find_if(args.begin(), args.end(), [&] (vector<char>& a)->bool
		{
			return MatchesArgumentListWithOldVariables(a, data);
		});
	if(iterator == args.end())
		throw exception("Funkci nebyly pøedány správné argumenty");
	//Copy arguments
	if(fun.second == -1)
		func.data[fun.first] = data[fun.first][0];
	else
		func.data[fun.first] = fun.second;
	for_each(iterator->begin(), iterator->end(), [&](char c)
	{
		auto argIter = data.find(c);
		if(argIter != data.end())
		{
			if(argIter->second.size() > 1)
				throw exception("Vícenásobný argument");
			func.data[c] = data[c][0];
			return;
		}
		auto argIter2 = lastValue.find(c);
		if(argIter2 != lastValue.end())
		{
			if(coordType == INC && IsAxis(c))
			{
				func.data[c] = 0;
			}
			else
				func.data[c] = lastValue[c];
			return;
		}
		throw exception("Nebyly pøedány všechny argumenty");
	});
	return func;
}

void GCodeInterpreter::ConvertCommandToPath(GCodeLine& line)
{
	auto iterator = processingFunctions.find(line.function);
	if(iterator == processingFunctions.end())
	{
		throw exception("Nalezena neznámá funkce");
	}
	(this->*(iterator->second))(line);//Process
}

void GCodeInterpreter::AddPathSegment(unique_ptr<PathPart>& part)
{
	path.push_back(move(part));
	path.back()->SetIterator(--path.end());//Set the iterator
}

float GCodeInterpreter::ConvertUnits(float number)
{
	return number*unitMultiply;
}

bool GCodeInterpreter::IsAxis(char c)
{
	static const string axis("XYZIJK");
	c = toupper(c);
	return any_of(axis.begin(), axis.end(), [&](char a)->bool{return a == c;});
}

PathOffsets GCodeInterpreter::GetPathOffsets()
{
	return offset;
}

bool GCodeInterpreter::IsPathValid()
{
	//ToDo: Complete this function
	ofstream o("debug.txt");
	PathPart* cur = &(*path.begin())->GetNextMovable();
	PathPart* next = &cur->GetNextMovable();
	while(next->GetType() != PROGRAM_END)
	{
		o << cur->GetType() << "\t" << cur->GetStartingSpeed() << "\t" << cur->GetSpeed() << "\t" << cur->GetEndingSpeed() << endl;
		assert(cur->GetOffsetEndingPoint() == next->GetOffsetStartingPoint());
		assert(cur->GetEndingSpeed() == next->GetStartingSpeed());
		assert(cur->GetSpeed() >= cur->GetStartingSpeed() && cur->GetSpeed() >= cur->GetEndingSpeed());
		cur = next;
		next = &cur->GetNextMovable();
	}
	return true;
}


void GCodeInterpreter::AddPathPart(unique_ptr<PathPart>& p, list<unique_ptr<PathPart>>::iterator i)
{
	auto in = path.insert(i, move(p));
	(*in)->SetIterator(--i);//Set the iterator
}

void GCodeInterpreter::CreateOffsetOutput(vector<PathPartSimple>* offsetOutput)
{
	for_each(path.begin(), path.end(), [&](unique_ptr<PathPart>& p)
	{
		if(p->IsOffset())
		{
			offsetOutput->push_back(p->ToOffsetSimple());
		}
	});
}