#pragma once
#include "global.h"
#include "Geometry.h"
#include <wx\wx.h>
#include "iniFile.h"
#include <string>
#include "GCodeInterpreter.h"

class MyData;

using namespace std;

//Window (middle panel) - shows trajectory preview
class PreviewWindow :
	public wxPanel
{
	public:
		PreviewWindow(wxWindow *parent, int x, int y, int w, int h, long style=wxTAB_TRAVERSAL, const wxString &name=wxPanelNameStr);
		~PreviewWindow(void);
		void Reset();
		void OnPaint(wxPaintEvent& evt);
		void RefreshView();
		void OnMouseMove(wxMouseEvent& evt);
		void OnWheelMoved(wxMouseEvent& evt);
		vector<PathPartSimple>* GetToDrawPointer();
		void DrawDataFinished();
		vector<PathPartSimple>* GetToDrawOffsetPointer();
		void SetToDraw(vector<PathPartSimple>& p);
		void AddToTrajectory(Point p);
		void ClearTrajectory();
		void ToggleTrajectory();
	private:
		void Render(wxDC& dc);

		float zoomFactor;
		wxPoint lastCurPos;
		Point offset;
		int wheelPosition;
		MyData* data;

		float originSize;
		float lineWidth;
		wxColour rapidColour, normalColour,
			cCWColour, cCCWColour, backgroundColour,
			trajectoryColour;

		vector<Point> trajectory;
		vector<PathPartSimple> toDraw;
		vector<PathPartSimple> offsetToDraw;
		bool drawTrajectory;

		static wxColour ParseColour(string s);
		void DrawOrigin(wxDC& dc);
		void DrawPath(wxDC& dc);
		void DrawTrajectory(wxDC& dc);
		Point TransformPoint(Point p);
		void SetMaxZoom();
};

