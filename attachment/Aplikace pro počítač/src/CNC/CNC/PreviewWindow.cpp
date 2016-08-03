#include "PreviewWindow.h"
#include "global.h"
#include <wx/dcbuffer.h>


PreviewWindow::PreviewWindow(wxWindow *parent, int x, int y, int w, int h, long style, const wxString &name):
	wxPanel(parent, x, y, w, h, style, name), wheelPosition(0), zoomFactor(0.2f), drawTrajectory(true)
{
	Connect(this->GetId(), wxEVT_PAINT, wxPaintEventHandler(PreviewWindow::OnPaint));
	Connect(this->GetId(), wxEVT_MOTION, wxMouseEventHandler(PreviewWindow::OnMouseMove));
	Connect(this->GetId(), wxEVT_MOUSEWHEEL, wxMouseEventHandler(PreviewWindow::OnWheelMoved));

	SetBackgroundStyle(wxBG_STYLE_CUSTOM);

	CIniFile ini;
	originSize = StringToFloat(ini.GetValue("originSize", "UI", iniFile));
	zoomFactor = StringToFloat(ini.GetValue("zoomCoefficient", "UI", iniFile));
	lineWidth = StringToFloat(ini.GetValue("lineWidth", "UI", iniFile));
	rapidColour = ParseColour(ini.GetValue("rapidColour", "UI", iniFile));
	normalColour = ParseColour(ini.GetValue("normalColour", "UI", iniFile));
	cCWColour = ParseColour(ini.GetValue("CWColour", "UI", iniFile));
	cCCWColour = ParseColour(ini.GetValue("CCWColour", "UI", iniFile));
	trajectoryColour = ParseColour(ini.GetValue("trajectoryColour", "UI", iniFile));
	backgroundColour = ParseColour(ini.GetValue("backgroundColour", "UI", iniFile));
}


PreviewWindow::~PreviewWindow(void)
{
}

void PreviewWindow::OnPaint(wxPaintEvent& evt)
{
	wxAutoBufferedPaintDC dc(this);
    Render(dc);
}

void PreviewWindow::RefreshView()
{
	wxClientDC dc(this);
	wxBufferedDC* dc2 = new wxBufferedDC(&dc);
    Render(*dc2);
	delete dc2;
}

void PreviewWindow::Render(wxDC& dc)
{
	dc.SetBackground(wxBrush(backgroundColour));
	dc.Clear();		
	DrawPath(dc);
	DrawTrajectory(dc);
	DrawOrigin(dc);
	wxString text;
	text << wxT("X: ") << offset.x << wxT("  Y: ") << offset.y
		<< wxT("  Z: ") << wheelPosition;
	dc.SetTextForeground(wxColour(255, 255, 255));
	dc.DrawText(text, 40, 10);
}

void PreviewWindow::OnMouseMove(wxMouseEvent& evt)
{
	if(evt.LeftIsDown())//Moving with screen
	{
		wxPoint t = evt.GetPosition() - lastCurPos;
		Point increment;
		increment = t;
		increment /= 1+wheelPosition/120*zoomFactor;
		offset +=  increment;
		Refresh();
	}
	lastCurPos = evt.GetPosition();
}

void PreviewWindow::OnWheelMoved(wxMouseEvent& evt)
{
	wxPoint position = evt.GetPosition();
	wxSize size = GetSize();
	if(position.x >= 0 && position.x <= size.GetWidth() &&
		position.y >= 0 && position.y <= size.GetHeight())
		{	
			//Point correction = Point(evt.GetPosition());
			if(1+(wheelPosition + evt.GetWheelRotation())/120*zoomFactor > 0)
				wheelPosition += evt.GetWheelRotation();
		}
	Refresh();
}

void PreviewWindow::Reset()
{
	wheelPosition = 0;
	wxSize size = GetSize();
	offset.x = size.GetWidth()/2;
	offset.y = size.GetHeight()/2;
	offset.x = offset.y = 0;
	Refresh();
}

wxColour PreviewWindow::ParseColour(string s)
{
	int r, g, b;
	replace(s.begin(), s.end(), ',', ' ');
	stringstream stream(s);
	stream >> r >> g >> b;
	return wxColour(r, g, b);
}

void PreviewWindow::DrawOrigin(wxDC& dc)
{
	Point offset2 = offset;
	wxSize size = GetSize();
	offset2.x *= 1+wheelPosition/120*zoomFactor;
	offset2.y *= 1+wheelPosition/120*zoomFactor;
	offset2.x += size.GetWidth()/2;
	offset2.y += size.GetHeight()/2;
	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.DrawArc(offset2.x, offset2.y+originSize, offset2.x+originSize, offset2.y, offset2.x, offset2.y);
	dc.DrawArc(offset2.x, offset2.y-originSize, offset2.x-originSize, offset2.y, offset2.x, offset2.y);
	dc.SetPen(*wxWHITE_PEN);
	dc.SetBrush(*wxBLACK_BRUSH);
	dc.DrawArc(offset2.x+originSize, offset2.y, offset2.x, offset2.y-originSize, offset2.x, offset2.y);
	dc.DrawArc(offset2.x-originSize, offset2.y, offset2.x, offset2.y+originSize, offset2.x, offset2.y);
}

void PreviewWindow::SetToDraw(vector<PathPartSimple>& p)
{
	toDraw = p;
	ClearTrajectory();
	SetMaxZoom();
	Refresh();
}

void PreviewWindow::AddToTrajectory(Point p)
{
	trajectory.push_back(p);
	Refresh();
}

void PreviewWindow::ClearTrajectory()
{
	trajectory.clear();
	Refresh();
}

vector<PathPartSimple>* PreviewWindow::GetToDrawPointer()
{
	toDraw.clear();
	ClearTrajectory();
	return &toDraw;
}

void PreviewWindow::DrawPath(wxDC& dc)
{
	wxColour cw = cCWColour;
	wxColour ccw = cCCWColour;
	for_each(toDraw.begin(), toDraw.end(), [&](PathPartSimple p)
	{
		switch(p.type)
		{
			case LINE_NORMAL:
				{
					dc.SetPen(wxPen(normalColour, lineWidth, wxSOLID));
					p.start = TransformPoint(p.start);
					p.end = TransformPoint(p.end);
					dc.DrawLine(p.start.x, p.start.y, p.end.x, p.end.y);
				}
				break;
			case LINE_RAPID:
				{
					dc.SetPen(wxPen(rapidColour, lineWidth, wxLONG_DASH));
					p.start = TransformPoint(p.start);
					p.end = TransformPoint(p.end);
					dc.DrawLine(p.start.x, p.start.y, p.end.x, p.end.y);
				}
				break;
			case CIRCLE_CW:
				{
					
					cw.Set(255-cw.Red(), cw.Green(), cw.Blue());//Differes two circles
					dc.SetPen(wxPen(cw, lineWidth, wxSOLID));
					p.start = TransformPoint(p.start);
					p.end = TransformPoint(p.end);
					p.center = TransformPoint(p.center);
					dc.SetBrush(wxBrush(wxColour(), wxTRANSPARENT));
					Vector2D distance(p.start, p.end);
					if(distance.Length() < 2)
						break;
					distance = Vector2D(p.center, p.start);
					if(distance.Length() < 2)
						break;
					dc.DrawArc(p.start.x, p.start.y, p.end.x, p.end.y, p.center.x, p.center.y);
				}
				break;
			case CIRCLE_CCW:
				{
					ccw.Set(255-ccw.Red(), ccw.Green(), ccw.Blue());//Differes two circles
					dc.SetPen(wxPen(ccw, lineWidth, wxSOLID));
					p.start = TransformPoint(p.start);
					p.end = TransformPoint(p.end);
					p.center = TransformPoint(p.center);
					dc.SetBrush(wxBrush(wxColour(), wxTRANSPARENT));
					Vector2D distance(p.start, p.end);
					if(distance.Length() < 2)
						break;
					distance = Vector2D(p.center, p.start);
					if(distance.Length() < 2)
						break;
					dc.DrawArc(p.end.x, p.end.y, p.start.x, p.start.y, p.center.x, p.center.y);
				}
				break;
		}
	});

	for_each(offsetToDraw.begin(), offsetToDraw.end(), [&](PathPartSimple p)
	{
		dc.SetPen(wxPen(wxColor(255, 255, 255), lineWidth, wxSOLID));
		switch(p.type)
		{
			case LINE_NORMAL:
				{
					p.start = TransformPoint(p.start);
					p.end = TransformPoint(p.end);
					dc.DrawLine(p.start.x, p.start.y, p.end.x, p.end.y);
				}
				break;
			case LINE_RAPID:
				{
					dc.SetPen(wxPen(wxColor(255, 255, 255), lineWidth, wxDOT_DASH));
					p.start = TransformPoint(p.start);
					p.end = TransformPoint(p.end);
					dc.DrawLine(p.start.x, p.start.y, p.end.x, p.end.y);
				}
				break;
			case CIRCLE_CW:
			case OFFSET_CIRCLE_CW:
				{
					static wxColour c = cCWColour;
					c.Set(255-c.Red(), c.Green(), c.Blue());
					p.start = TransformPoint(p.start);
					p.end = TransformPoint(p.end);
					p.center = TransformPoint(p.center);
					dc.SetBrush(wxBrush(wxColour(), wxTRANSPARENT));
					Vector2D distance(p.start, p.end);
					if(distance.Length() < 2)
						break;
					distance = Vector2D(p.center, p.start);
					if(distance.Length() < 2)
						break;
					dc.DrawArc(p.start.x, p.start.y, p.end.x, p.end.y, p.center.x, p.center.y);
				}
				break;
			case CIRCLE_CCW:
			case OFFSET_CIRCLE_CCW:
				{
					static wxColour c = cCCWColour;
					c.Set(255-c.Red(), c.Green(), c.Blue());
					p.start = TransformPoint(p.start);
					p.end = TransformPoint(p.end);
					p.center = TransformPoint(p.center);
					dc.SetBrush(wxBrush(wxColour(), wxTRANSPARENT));
					Vector2D distance(p.start, p.end);
					if(distance.Length() < 2)
						break;
					distance = Vector2D(p.center, p.start);
					if(distance.Length() < 2)
						break;
					dc.DrawArc(p.end.x, p.end.y, p.start.x, p.start.y, p.center.x, p.center.y);
				}
				break;
		}
	});
}

Point PreviewWindow::TransformPoint(Point p)
{
	p *= 1+wheelPosition/120*zoomFactor;
	Point offset2 = offset;
	offset2 *= 1+wheelPosition/120*zoomFactor;
	wxSize size = GetSize();
	offset2.x += size.GetWidth()/2;
	offset2.y += size.GetHeight()/2;
	return p+offset2;
}

void PreviewWindow::DrawDataFinished()
{
	Reset();
	SetMaxZoom();
	Refresh();
}

void PreviewWindow::SetMaxZoom()
{
	Point minimal, maximal;
	for_each(toDraw.begin(), toDraw.end(), [&](PathPartSimple& p)
	{
		minimal.x = min(minimal.x, min(p.start.x, p.end.x));
		maximal.x = max(maximal.x, max(p.start.x, p.end.x));
		minimal.y = min(minimal.y, min(p.start.y, p.end.y));
		maximal.y = max(maximal.y, max(p.start.y, p.end.y));
	});
	float height = fabs(maximal.y - minimal.y);
	float width = fabs(maximal.x - minimal.x);
	wxSize size = GetSize();
	float zoom = min(size.GetHeight()/height, size.GetWidth()/width)*0.9;
	wheelPosition=(zoom-1)*120/zoomFactor;
	Point center;
	center.x = -(minimal.x+maximal.x)/2;
	center.y = -(minimal.y+maximal.y)/2;
	offset = center;
}



void PreviewWindow::DrawTrajectory(wxDC& dc)
{
	Point previous;
	dc.SetPen(wxPen(trajectoryColour, lineWidth*2, wxSOLID));
	for_each(trajectory.begin(), trajectory.end(), [&](Point& p)
	{
		Point start = TransformPoint(previous);
		Point end = TransformPoint(p);
		if(drawTrajectory)
			dc.DrawLine(start.x, start.y, end.x, end.y);
		previous = p;
	});
	dc.SetPen(wxPen(trajectoryColour, lineWidth, wxSOLID));
	dc.SetBrush(*wxRED_BRUSH);
	previous = TransformPoint(previous);
	dc.DrawCircle(previous.x, previous.y, 4);
}

void PreviewWindow::ToggleTrajectory()
{
	drawTrajectory = !drawTrajectory;
}

vector<PathPartSimple>* PreviewWindow::GetToDrawOffsetPointer()
{
	offsetToDraw.clear();
	return &offsetToDraw;
}
