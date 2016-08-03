#pragma once
#include <string>
#include <vector>
#include "ProcessedData.h"
#include "global.h"
#include <map>
#include <list>
#include "Geometry.h"
#include "CodeModificators.h"
#include "PathPart.h"
#include "GCodeLine.h"
#include "PathOffsets.h"


using namespace std;

//Test if float value is valid
bool IsValid(float f);


Point operator*(Point, float& f);

class GCodeInterpreter;
struct PathPartSimple;

//Typedefs for GCodeInterpreter
typedef vector<vector<char>> FunctionArguments;
typedef void (GCodeInterpreter::*GCodeProcessingFunction)(GCodeLine&);

//Class process GCode to list of commands for interpolator
class GCodeInterpreter
{
	friend class PathPart;
	public:
		GCodeInterpreter(void);
		~GCodeInterpreter(void);
		bool ProcessString(string code, vector<ProcessedData>& output, vector<PathPartSimple>* simpleOutput, vector<PathPartSimple>* offsetOutput);
	
		//Technical functions
		void AddPathPart(unique_ptr<PathPart>& p, list<unique_ptr<PathPart>>::iterator i);
		float GetJerk(){return maxJerk;}
		float GetAcceleration(){return maxAcceleration;};
	private:
		//Speeds
		float rapidFeed;
		float standardFeed;
		//Acceleration
		float maxAcceleration;
		float maxJerk;
		//Maximal tollerance
		float maxRapidTollerance;//Tollerance for rapid speed movement
		float maxStandardTollerance;//Tollerance for standard speed moventnt

		//Non-physical configuration
		float unitMultiply;//Units multiplyer

		//State
		WorkingPlane workingPlane;//Working plane
		TypeOfCoordinates coordType;
		TypeOfCoordinates arcCoordType;
		Point currentPosition;//Actual position of axes
		map<char, float> lastValue;//Stores last value
		float lastMovementFunction;
		PathOffsets offset;//Currently aplied offsets

		//Settings
		vector<GFunction> orderTable;//Table with order of functions
		map<GFunction, FunctionArguments> functionPrototypes;//Prototypes of GCode function functions
		map<GFunction, GCodeProcessingFunction> processingFunctions;//Member functions for converting
			//lines into path

		vector<GCodeLine> commands;
		vector<ProcessedData>* output;
		list<unique_ptr<PathPart>> path;

		

	//Functions
		void LoadConfiguration();
		void FillTables();
		//Gcode functions
		void ParseCodeToPath(string& code);
		float ConvertUnits(float number);
		bool IsMovementFunction(GCodeLine& command);
		bool IsMovementFunction(GFunction& f);
		bool IsPathModifiableFunction(GCodeLine& command);
		bool MatchesArgumentList(vector<char>& prototype, map<char, vector<float>>& data);
		bool MatchesArgumentListWithOldVariables(vector<char>& prototype, map<char, vector<float>>& data);
		map<char, vector<float>> ParseGCodeLine(string line);
		vector<GCodeLine> SeparateCommandsFromLine(map<char, vector<float>>& data, size_t line);
		void AssertExistingFunction(map<char, vector<float>>& data, char type, size_t line);
		GCodeLine AssignGFuncParams(map<char, vector<float>>& data, GFunction& fun, size_t line);
		void ConvertCommandToPath(GCodeLine& line);
		bool IsAxis(char c);
		PathOffsets GetPathOffsets();
		//Path functions
		bool IsPathValid();//Check wheter the Path is valid - eg. it's continuous
		void PostProcessPath();//Creates equidistances etc.
			void ProcessPathRoundness();//Replaces sharp corners with sine
			void ProcessPathOffsets();//Processes tool compensation
		void TunePath();
		void TransformPathToCommands();
		void CreateSimpleOutput(vector<PathPartSimple>* output);
		void CreateOffsetOutput(vector<PathPartSimple>* offsetOutput);
		void AddPathSegment(unique_ptr<PathPart>& part);

		//GCode functions
		void FillGTable();
		void G00(GCodeLine& line);
		void G01(GCodeLine& line);
		void G02(GCodeLine& line);
		void G03(GCodeLine& line);
		void CircleFunction(GCodeLine& line, PathPartType t);
		void G20(GCodeLine& line);
		void G21(GCodeLine& line);
		void G40(GCodeLine& line);
		void G41(GCodeLine& line);
		void G42(GCodeLine& line);
		void G43(GCodeLine& line);
		void G44(GCodeLine& line);
		void G49(GCodeLine& line);
		void F(GCodeLine& line);
		void M02(GCodeLine& line);
		void M03(GCodeLine& line);
		void M04(GCodeLine& line);
		void M05(GCodeLine& line);
};

